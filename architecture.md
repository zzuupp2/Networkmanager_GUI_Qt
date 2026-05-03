# 项目架构

基于 NetworkManagerQt (Qt binding for NetworkManager DBus API) 的 Linux 网络管理 GUI，
使用 Qt 6 + QML + CMake。

---

## 目录

- [入口与初始化](#入口与初始化)
- [整体分层架构](#整体分层架构)
- [详细调用关系](#详细调用关系)
- [数据流](#数据流)
- [各模块详解](#各模块详解)
  - [Utils 层 — NetUtils](#utils-层--netutils)
  - [Wrapper 层 — NetworkManagerWrapper](#wrapper-层--networkmanagerwrapper)
  - [Service 层](#service-层)
  - [Model 层](#model-层)
- [QML 视图结构](#qml-视图结构)
- [QML 组件概览](#qml-组件概览)

---

## 入口与初始化

`main.cpp`:

1. 创建 `QQmlApplicationEngine`
2. 注册 `Net::ConnectionEditorModel` 为 QML 类型 `"Networkmanager_GUI_Qt" 1.0`
3. 创建单例 `Net::NetworkManagerWrapper nm`，通过 `setContextProperty("nm", &nm)` 暴露给 QML
4. 加载 QML 模块 `"Networkmanager_GUI_Qt"` 的 `"Main"` 组件

`NetworkManagerWrapper` 构造函数初始化顺序：

```
成员初始化列表:
  m_runtimeService(this)     ← 无依赖
  m_connectionList(&m_runtimeService, this)
  m_runtimeModel(&m_runtimeService, this)

构造函数体:
  m_deviceProxy.setSourceModel(&m_deviceModel)
  m_apModel.setService(&m_apService)
  m_editor.setConnectionManager(&m_connectionManager)
  initDeviceService()                     ← 连接 DeviceService ↔ DeviceModel 信号
  QTimer::singleShot(0, ...)             ← 推迟到事件循环启动后:
      m_runtimeService.updateAll()
      m_runtimeModel.setConnections(uuids)
      m_runtimeModel.refreshPrimaryRow()
      selectConnection(first())
```

> `QTimer::singleShot(0)` 是必要的：NM 的 `listConnections()` 等 API 在构造函数执行时尚未就绪，
> 推迟到事件循环中可确保 NM DBus 异步属性已可用。

---

## 整体分层架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                        QML UI 层                                    │
│  TopBar / SideBar / DevicePage / WifiPage / ConnectionsPage         │
│  DeviceCard / WifiItem / ConnectionEditorPanel / NewConnectionDialog│
└──────────────────────────┬──────────────────────────────────────────┘
                           │ 全局上下文属性 "nm"
                           ▼
┌─────────────────────────────────────────────────────────────────────┐
│              NetworkManagerWrapper (桥梁层)                          │
│  暴露所有 Model/Service 为 Q_PROPERTY，信号转发，初始化编排             │
└────┬──────────┬───────────────┬──────────────────┬──────────────────┘
     │          │               │                  │
     ▼          ▼               ▼                  ▼
┌─────────┐ ┌─────────┐ ┌──────────────┐ ┌──────────────┐
│ Device  │ │ WiFi/AP │ │ Connection   │ │ Connection   │
│ Service │ │ Service │ │ Runtime      │ │ Manager      │
│ + Model │ │ + Model │ │ Service      │ │ (CRUD + Act) │
└─────────┘ └─────────┘ │ + Model      │ └──────┬───────┘
                        └──────┬───────┘        │
                               │                ▼
                               │         ┌──────────────┐
                               │         │ Editor Model │
                               │         │ (QML type)   │
                               │         └──────────────┘
                               ▼
                        ┌──────────────┐
                        │ Connection   │
                        │ List Model   │
                        └──────────────┘
```

---

## 详细调用关系

### Device 调用链

```
NM 信号 (deviceAdded/Removed, stateChanged, ...)
  → DeviceService (监听 + 200ms 防抖 + 原因冻结)
    → DeviceService::deviceAdded/deviceUpdated/deviceRemoved 信号
      → DeviceModel::addDevices/updateDevice/removeDevice 槽
        → QML ListView (model: nm.deviceProxy)
          → DeviceFilterProxyModel (三态过滤)
            → DeviceCard 委托
```

### AP/WiFi 调用链

```
NM 信号 (accessPointAppeared/Disappeared, lastScanChanged)
  → ApService (扫描间隔控制 10s 定时器)
    → ApService::accessPointsChanged/activeApChanged 信号
      → ApModel::reload (SSID→UUID 缓存, 活跃 AP 置顶, 按信号排序)
        → QML ListView (model: nm.apModel)
          → WifiItem 委托

用户操作:
  WifiPage "扫描"按钮 → nm.apService.requestScan()
  WifiItem "连接"按钮 → connectDialog → nm.manager.createAndActivateWifiConnection()
  WifiItem "断开"按钮 → nm.manager.deactivate(uuid)
```

### 连接运行时调用链

```
NM 信号 (activeConnectionAdded/Removed, stateChanged, ipV4ConfigChanged)
  → ConnectionRuntimeService (动态绑定 ActiveConnection watcher)
    → updateAll() 遍历所有活跃连接，组装 RuntimeState
      → stateChanged(uuid) 信号
        → ConnectionRuntimeModel::onStateChanged (插入/更新行)
          → QML ConnectionsPage 状态页 Repeater
          → TopBar primaryInfo (通过 primaryRefresh 强制重求值)

primary 连接追踪:
  NM Notifier::primaryConnectionChanged
    → ConnectionRuntimeModel::updatePrimaryUuid()
      → primaryRowChanged / primaryChanged 信号
        → TopBar primaryInfo 属性绑定
        → primaryRefresh++ 强制重求值
```

### 连接管理调用链

```
保存连接:
  ConnectionEditorPanel "保存"按钮
    → nm.manager.apply(info, isNew, uuid)
      → isNew ? addConnection() : updateConnection()
        → NM DBus addConnection/connection.update()
          → operationCompleted 信号

激活/断开:
  ConnectionsPage 列表 "连接"/"断开"按钮
    → nm.manager.activate(uuid)
      → NetworkManager::activateConnection()
    → nm.manager.deactivate(uuid)
      → 临时禁用设备 autoconnect → NetworkManager::deactivateConnection()
        → 成功后恢复 autoconnect

新建连接:
  NewConnectionDialog "下一步"
    → dialogEditor.loadDefaults(type)
      → ConnectionEditorModel 填入默认值（auto-select interface）
    → 编辑后保存 → nm.manager.apply(...)
      → nm 监听 connectionAdded → selectConnection(uuid) 自动选中
```

### 主连接 TopBar 调用链

```
NetworkManager::primaryConnection()
  → ConnectionRuntimeModel::updatePrimaryUuid()
    → 查找 primary Uuid 在 m_index 中的行号
      → primaryRow 属性 → QML primaryInfo 计算

NetworkManager::connectivity()
  → ConnectionRuntimeModel 构造函数监听 connectivityChanged
    → m_connectivity 属性 → TopBar 联网状态圆点 + 文字

TopBar 刷新机制:
  Connections {
    target: nm.runtimeModel
    function onPrimaryRowChanged() { primaryRefresh++ }
    function onPrimaryChanged()  { primaryRefresh++ }
  }
  readonly property var primaryInfo: {
      primaryRefresh  // 依赖追踪
      const row = nm.runtimeModel.primaryRow
      if (row < 0) return null
      return nm.runtimeModel.get(row)
  }
```

---

## 数据流

### 读取（NM → QML）

```
NM DBus → ConnectionSettings::Ptr
  → ConnectionSettingInfo::fromNMSettings()
    → ConnectionEditorModel::loadFromSettingInfo()
      → QML property bindings → UI 显示
```

### 写入/更新（QML → NM）

```
QML 编辑字段 → editor.xxxChanged() → m_working 更新
  → 用户点击"保存"
    → editor.toSettingInfo() → ConnectionSettingInfo
      → nm.manager.apply()
        → isNew ? addConnection(info.toNMSettings())
                : updateConnection(uuid, info)
```

### 运行时状态（NM → QML）

```
NM DBus → ActiveConnection state change
  → ConnectionRuntimeService::updateAll()
    → stateChanged(uuid)
      → ConnectionRuntimeModel 更新行数据
        → dataChanged 信号 → QML 绑定自动刷新
```

---

## 各模块详解

### Utils 层 — NetUtils

`src/utils/network_utils.h/.cpp`

纯函数工具集，无状态，不依赖实例。涵盖了项目中所有的 NM 枚举→字符串转换和设备/类型判断。

| 分类 | 函数 | 说明 |
|------|------|------|
| 设备枚举转换 | `deviceTypeToString()` | 30+ 设备类型 → 英文名 |
| | `deviceStateToString()` | 13 种 NM 设备状态 → 英文名 |
| | `stateReasonToString()` | 30+ 状态变化原因 → 中文 |
| 状态摘要 | `stateReasonSummary(state, reason)` | 80+ case 的中文摘要 |
| | `stateReasonSummary(old, new, reason)` | 带状态过渡的版本，消除歧义 |
| 原因/状态过滤 | `isMeaningfulReason()` | 过滤 NoReason/UnknownReason |
| | `isTransientState()` | 识别瞬态状态（Unavailable 等） |
| 设备类型判断 | `isWifiDev()`, `isEthernetDev()`, `isNetworkDevice()` | |
| | `isWiredConType()`, `isWirelessConType()` | Connection 配置类型判断 |
| | `conTypeToDeviceType()` | "802-11-wireless" → Device::Wifi |
| | `interfacesForConType()`, `firstInterfaceForConType()` | 按连接类型列举可用接口 |
| AP/WiFi | `frequencyToBand()` | 频率→频段名（2.4/5/6 GHz） |
| | `apSecurityString()` | AP WPA/RSN flags → 安全类型（WPA2/WPA3 等） |
| 安全协议 | `keyMgmtToString()` / `stringToKeyMgmt()` | 双向转换，9 种协议类型 |
| 运行状态 | `activeConnectionStateToString()` | ActiveConnection 状态→中文 |
| | `activeConnectionReasonToString()` | 断开原因→中文 |
| | `activeConnectionStatusText()` | 状态+原因 → 显示文本 |
| IPv4 | `ipv4MethodToString()` / `stringToIpv4Method()` | ConfigMethod ↔ 字符串 |
| 杂项 | `getHwAddr()` | 从 Device::Ptr 提取 MAC 地址 |

---

### Wrapper 层 — NetworkManagerWrapper

`src/wrapper/nm_wrapper.h/.cpp`

**中心枢纽**，单例注入 QML 上下文为 `nm`。职责：

1. **暴露 Model/Service** — 所有 `Q_PROPERTY` 指向内部成员
2. **初始化编排** — 构造函数中设定 Model↔Service 信号连接
3. **信号转发** — NM notifier → QML 可用的信号
4. **便捷方法** — `selectConnection()` / `firstConnectionUuid()` / `hasConnection()`

#### Q_PROPERTY 一览

| 属性 | 类型 | 说明 |
|------|------|------|
| `deviceModel` | `DeviceModel*` | 设备列表（原始） |
| `deviceProxy` | `DeviceFilterProxyModel*` | 设备列表（已过滤） |
| `apModel` | `ApModel*` | WiFi AP 列表 |
| `apService` | `ApService*` | 扫描控制 |
| `connectionListModel` | `ConnectionListModel*` | 已保存连接列表 |
| `runtimeModel` | `ConnectionRuntimeModel*` | 运行时状态模型 |
| `manager` | `ConnectionManager*` | 连接 CRUD + 激活/断开 |
| `editor` | `ConnectionEditorModel*` | 连接编辑表单 |
| `currentUuid` | `QString` | 当前选中连接 UUID |
| `wirelessEnabled` | `bool` | WiFi 开关状态 |
| `networkingEnabled` | `bool` | 联网开关状态（飞行模式） |

---

### Service 层

#### DeviceService

`src/service/device_service.h/.cpp`

监听 NM 设备生命周期和状态变化。

- `init()`: 遍历现有设备，逐个调用 `watchDevice()` 发出 `deviceAdded`
- `watchDevice(dev)`: 为每个设备注册多种信号监听 + 200ms 防抖 QTimer
- **原因冻结**: 设备断开时记录用户操作原因，防止瞬态 Unavailable 覆盖

#### ApService

`src/service/ap_service.h/.cpp`

管理无线设备的扫描和活跃 AP 跟踪。

- 持有第一个找到的 `WirelessDevice::Ptr`
- `requestScan(force)`: 扫描间隔控制（强制忽略 5s 冷却）
- `setScanEnable(bool)`: 切换扫描定时器（10s 间隔），页面切换时调用

#### ConnectionRuntimeService

`src/service/con_runtime_service.h/.cpp`

运行时状态的核心服务，追踪 `NetworkManager::ActiveConnection` 状态。

- 动态绑定 ActiveConnection watcher (`refreshActiveConnectionWatchers()`)
- `updateAll()`: 遍历所有活跃连接，汇总 `RuntimeState`（IP/网关/DNS/接口/MAC/MTU/速率/频段 等）
- 处理 `activatingConnection` 特殊状态（激活中的连接单独处理）

#### ConnectionManager

`src/service/connection_manager.h/.cpp`

NM DBus 操作的高级封装，所有方法的最终调用方。

| 方法 | DBus 操作 | 说明 |
|------|-----------|------|
| `activate()` | `NetworkManager::activateConnection()` | 自动选择合适设备 |
| `deactivate()` | `NetworkManager::deactivateConnection()` | 临时禁用 autoconnect 防重连 |
| `addConnection()` | `NetworkManager::addConnection()` | 创建新配置文件 |
| `updateConnection()` | `conn->update()` | 原地更新 NM settings |
| `deleteConnection()` | `conn->remove()` | 删除配置文件 |
| `createAndActivateWifiConnection()` | 组合上述操作 | 一步创建并激活 WiFi |
| `apply()` | 自动判断 add/update | 统一入口 |

`activateConnection()` 设备选择策略：
1. 优先匹配 `connection.interface-name` 指定的接口
2. 其次选择同类型第一个非 Unavailable 设备
3. 兜底：允许 Unavailable 设备

`deactivateConnection()` 防重连机制：断开前保存设备 `autoconnect` 原值并设为 false，
等待活跃连接真正移除后恢复（通过 `activeConnectionAdded` 信号检测，场景为原连接被移除并恢复 autoconnect）。

---

### Model 层

#### DeviceModel

`src/model/device/device_model.h/.cpp`

`QAbstractListModel`，存储 `DeviceInfo` 列表，按 `uni` 索引。

角色: `devName`, `devType`, `devHwAddr`, `devMtu`, `devState`, `devStateSummary`,
`curConnection`, `devDriver`, `devBitRate`, `devCarrier`, `devActiveSsid`, `devSignalStrength` 等。

#### DeviceInfo

`src/model/device/device_info.h/.cpp`

纯数据结构，`fromDevice()` 静态工厂从 NM `Device::Ptr` 提取必要字段。

#### DeviceFilterProxyModel

`src/filter/device_filter_proxy_model.h/.cpp`

`QSortFilterProxyModel`，三种过滤模式：
- `PrimaryOnly` — 仅 WiFi / 以太网 / PPP / Modem
- `PrimaryAndSecondary` — 排除 Loopback / Veth / Dummy 等虚拟设备
- `AllDevices` — 全部通过

#### ApModel

`src/model/ap/ap_model.h/.cpp`

`QAbstractListModel`，WiFi AP 列表。

角色: `ssid`, `strength`, `security`, `connected`, `band`, `bssid`,
`securityType`, `bandwidth`, `activeConnectionUuid`, `savedConnectionUuid`。

`reload()` 执行：构建 SSID→UUID 缓存 → 获取 AP 列表 → 活跃 AP 置顶 → 信号强度降序。

先前 `bandFromFrequency()` 和 `securityTypeString()` 两个私有静态方法已提取到 `NetUtils`。

#### ConnectionListModel

`src/model/connection/con_list_model.h/.cpp`

`QAbstractListModel`，已保存连接列表。

角色: `conName`, `conUuid`, `conActiveState`, `conLastConnection`。

- 监听 `SettingsNotifier::connectionAdded/Removed` → 全量 `reload()`
- 监听 `ConnectionRuntimeService::stateChanged` → 单行更新 activeState
- `lastConnection` 不依赖 NM timestamp，仅在运行时观察到 `activeState == 2` 时设置

#### ConnectionRuntimeModel

`src/model/connection/con_runtime_model.h/.cpp`

`QAbstractListModel`，运行时状态展示模型。

角色: `uuid`, `activeState`, `ipv4`, `gateway`, `dns`, `interface`, `mac`, `mtu`,
`wiredSpeed`, `carrier`, `ssid`, `wirelessRate`, `mode`, `bandwidth`,
`frequencyBand`, `deviceType`, `stateReason`, `statusText`。

额外 `Q_PROPERTY`:
- `primaryRow` — 当前主连接在模型中的行号（-1 表示无主连接）
- `connectivity` — 当前联网状态（0=Unknown, 1=None, 2=Portal, 3=Limited, 4=Full）

初始化时序：`setConnections()` 必须在 QTimer::singleShot(0) 中调用（NM 异步属性）。
`refreshPrimaryRow()` 在 model 重建后重发 `primaryRowChanged`。

#### ConnectionEditorModel

`src/model/connection/con_editor_model.h/.cpp`

**非 Model 类型**（继承 QObject），注册为 QML 类型 `"Networkmanager_GUI_Qt" 1.0`。

双副本设计：
- `m_original` — 从 NM 加载的原始值
- `m_working` — 用户在 UI 中编辑的当前值
- `isModified` — 比较两者得出（驱动保存按钮 enabled）

每个字段有独立 `Q_PROPERTY` + `NOTIFY` 信号，支持 QML 双向绑定。

**loadDefaults(type):** 为新连接自动填入：
- 自动生成 UUID
- 根据类型填写默认名称（"New Wi-Fi" / "New Wired"）
- 自动选择同类型第一个接口

#### ConnectionSettingInfo

`src/model/connection/con_setting_info.h/.cpp`

纯数据结构 + 双向 NM 序列化。

- `fromNMSettings()` — NM settings → Info（解析 5 个 section，请求 WiFi secrets）
- `toNMSettings()` — Info → `NMVariantMapMap`（供 addConnection 使用）

所有字段扁平化，不嵌套 NM 的 section 结构。

---

## QML 视图结构

```
Main.qml (ApplicationWindow, 1000×650)
 │
 ├── TopBar (height: 50)
 │   ├── 设备图标（📶/🔌）
 │   ├── 连接名称（SSID 或接口名）
 │   ├── IP 地址（含掩码，如 "192.168.1.100/24"）
 │   ├── 连接状态文字（已连接/正在激活/...）
 │   ├── 联网状态圆点（绿/橙/红 = 已联网/受限/无网络）
 │   ├── WiFi 开关 Switch
 │   └── 飞行模式 Switch
 │
 ├── SideBar (width: 180, 最小 100)
 │   └── ListView: ["设备", "WiFi", "已保存网络"]
 │
 └── StackLayout
       │
       ├── [0] DevicePage
       │   ├── 过滤栏 ComboBox（仅主设备 / 主+副 / 全部）
       │   └── ListView (model: nm.deviceProxy)
       │       └── DeviceCard 委托
       │           ├── 类型图标 + 接口名
       │           ├── 状态徽标（颜色编码）
       │           ├── MAC 地址 / 当前连接
       │           └── "详情"按钮 → detailDialog
       │               ├── 状态总览（颜色编码文字）
       │               ├── 驱动信息 / 链路 / 无线
       │               └── "关闭"按钮
       │
       ├── [1] WifiPage
       │   ├── "扫描"按钮 → nm.apService.requestScan()
       │   └── ListView (model: nm.apModel)
       │       └── WifiItem 委托
       │           ├── SSID / 已连接标记
       │           ├── 安全类型 / 频段 / 带宽
       │           ├── 信号强度（数字 + 条形图）
       │           ├── BSSID
       │           └── "连接"/"断开"按钮
       │               └── connectDialog（密码输入)
       │
       └── [2] ConnectionsPage
           ├── 左侧面板 (min 220px)
           │   ├── "新增连接"按钮 → NewConnectionDialog
           │   │   ├── Step 1: 类型选择（WiFi / 以太网）
           │   │   └── Step 2: 编辑面板 (ConnectionEditorPanel)
           │   ├── ListView (model: nm.connectionListModel)
           │   │   └── 连接列表项
           │   │       ├── 连接名 / 状态文字 + 上次连接时间
           │   │       └── "连接"/"断开"按钮
           │   └── "删除连接"按钮
           │
           └── 右侧面板 (min 400px)
               ├── TabBar: "状态" | "配置"
               └── StackLayout
                   ├── 状态页
                   │   ├── 未选择连接 → "请选择一个连接"
                   │   ├── 运行时信息 Repeater
                   │   │   ├── 状态文字（带颜色）
                   │   │   ├── 网络接口 / MAC / MTU
                   │   │   ├── IPv4 / 网关 / DNS
                   │   │   ├── 有线：端口速率 / 载波状态
                   │   │   └── 无线：SSID / 速率 / 频段 / 带宽
                   │   └── 未激活提示 → "该连接当前未激活"
                   │
                   └── 配置页 (ConnectionEditorPanel)
                       ├── 基本设置：名称 / 类型 / 接口（ComboBox）
                       ├── 自动连接：CheckBox + 优先级 SpinBox
                       ├── Wi-Fi 设置（条件显示）
                       │   ├── SSID（可编辑 ComboBox + 🔒 锁图标）
                       │   ├── 安全类型（ComboBox）
                       │   └── 密码（TextField + 显示/隐藏）
                       ├── 有线设置（条件显示）
                       │   ├── MTU
                       │   ├── 连接协商（自动/手动/忽略）
                       │   ├── 速率 / 双工模式
                       └── IPv4 设置
                           ├── 方法（DHCP / 手动 / 禁用 / ...）
                           ├── 地址 / 网关 / DNS
```

---

## QML 组件概览

| 组件 | 位置 | 职责 |
|------|------|------|
| `TopBar` | `qml/components/TopBar.qml` | 主连接信息 + 联网状态 + WiFi/飞行模式开关 |
| `SideBar` | `qml/components/SideBar.qml` | 三页导航，高亮当前选中项 |
| `DeviceCard` | `qml/components/DeviceCard.qml` | 设备摘要卡片，颜色按状态编码 |
| `WifiItem` | `qml/components/WifiItem.qml` | WiFi AP 卡片，安全/频段/信号 + 连接按钮 |
| `ConnectionEditorPanel` | `qml/components/ConnectionEditorPanel.qml` | 完整连接编辑表单，分组布局 |
| `NewConnectionDialog` | `qml/components/NewConnectionDialog.qml` | 两步新建：类型选择 → 编辑 |

### 组件规范

- **灰度色阶**: `clPrimary: "#1a1a1a"`, `clSecondary: "#666666"`, `clDisabled: "#999999"`
- **未配置提示**: `hint(base, val)` 函数 → val 为空时追加 "（未配置）"
- **密码显示/隐藏**: `checkable Button` 切换 `echoMode`
- **Dialog 居中**: `x: (parent.width - width) / 2`
- **SSID 锁图标**: 扫描 AP 模型，仅当 editText 匹配已知加密网络时显示 `🔒`

---

## CMakeLists 结构

```
CMakeLists.txt:
  Qt 6.2+, KF6NetworkManagerQt
  qt_add_qml_module(URI: Networkmanager_GUI_Qt, VERSION: 1.0)
  SOURCES: 所有 .h/.cpp（除 con_setting_service/model 被注释）
  target_link_libraries: Qt6::Quick, Qt6::Qml, KF6::NetworkManagerQt
```

当前未启用的组件（注释状态）：
- `ConnectionSettingService` / `ConnectionSettingModel` — 计划但未实现
