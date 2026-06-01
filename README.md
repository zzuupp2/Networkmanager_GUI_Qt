# Networkmanager GUI Qt

> 本项目为毕业设计作品，由 AI Agent（Claude Code）辅助完成。This project is a graduation design, developed with the assistance of AI Agent (Claude Code).

基于 Qt 6、QML 和 KF6NetworkManagerQt 构建的 Linux 网络管理图形界面。A Linux network management GUI built with Qt 6, QML, and KF6NetworkManagerQt (Qt bindings for NetworkManager's D-Bus API).

## Features / 功能特性

- **Device management** — view all network devices with real-time state, MAC address, driver info
- **Wi-Fi scanning & connection** — scan available access points, connect/disconnect with security type and signal strength display
- **Connection profiles** — create, edit, delete, activate, and deactivate saved network connections
- **Runtime monitoring** — live IP address, gateway, DNS, interface speed, SSID, and connectivity status
- **Auto-reconnect prevention** — temporarily disables device autoconnect during user-initiated disconnection
- **Device filtering** — three-tier filtering (primary / primary+secondary / all devices)
- **System tray status** — top bar shows primary connection info, IP, and network connectivity indicator

## Requirements

- Qt 6.2+
- KF6NetworkManagerQt
- CMake 3.16+
- NetworkManager (running on the system)

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

```bash
./build/appNetworkmanager_GUI_Qt
```

## Directory Structure

```
.
├── CMakeLists.txt                  # Build configuration
├── main.cpp                        # Application entry, QML type registration
├── architecture.md                 # Detailed architecture documentation
├── qml/
│   ├── Main.qml                    # Root window with TopBar, SideBar, StackLayout
│   ├── components/
│   │   ├── TopBar.qml              # Primary connection info, network status, toggles
│   │   ├── SideBar.qml             # Three-page navigation (Devices / Wi-Fi / Saved Networks)
│   │   ├── DeviceCard.qml          # Device summary card with state color coding
│   │   ├── WifiItem.qml            # Wi-Fi AP card (SSID, security, band, signal strength)
│   │   ├── ConnectionEditorPanel.qml  # Full connection editing form
│   │   └── NewConnectionDialog.qml    # Two-step new connection wizard
│   └── pages/
│       ├── DevicePage.qml          # Device list with filter
│       ├── WifiPage.qml            # Wi-Fi AP list with scan
│       └── ConnectionsPage.qml     # Saved connections list + runtime status / editor
└── src/
    ├── wrapper/
    │   └── nm_wrapper.h/.cpp       # Central hub: exposes models/services to QML as "nm"
    ├── model/
    │   ├── ap/
    │   │   └── ap_model.h/.cpp     # Wi-Fi AP list model
    │   ├── connection/
    │   │   ├── con_editor_model.h/.cpp   # Connection editing form model (QML type)
    │   │   ├── con_list_model.h/.cpp     # Saved connections list model
    │   │   ├── con_runtime_model.h/.cpp  # Runtime state model (IP, gateway, DNS, etc.)
    │   │   └── con_setting_info.h/.cpp   # NM settings ↔ flat data structure
    │   └── device/
    │       ├── device_info.h/.cpp   # Device data struct from NM Device::Ptr
    │       └── device_model.h/.cpp  # Device list model
    ├── service/
    │   ├── ap_service.h/.cpp       # Wi-Fi scanning and AP tracking
    │   ├── con_runtime_service.h/.cpp  # Active connection state tracking
    │   ├── connection_manager.h/.cpp   # Connection CRUD + activate/deactivate
    │   └── device_service.h/.cpp   # Device lifecycle monitoring
    ├── filter/
    │   └── device_filter_proxy_model.h/.cpp  # Three-tier device filter proxy
    └── utils/
        └── network_utils.h/.cpp    # Enum conversion, type checks, formatting utilities
```

## Architecture

```
QML UI (Main.qml, pages, components)
  │
  ▼
NetworkManagerWrapper (exposed as "nm" context property)
  │
  ├── DeviceService ←→ DeviceModel ←→ DeviceFilterProxyModel → QML
  ├── ApService ←→ ApModel → QML
  ├── ConnectionRuntimeService ←→ ConnectionRuntimeModel → QML
  ├── ConnectionListModel → QML
  ├── ConnectionManager (CRUD + activate/deactivate)
  └── ConnectionEditorModel (editing form, QML type)
```

All NetworkManager interactions go through KF6NetworkManagerQt's D-Bus API. Services listen to NM signals and forward changes to models, which QML views bind to via property bindings.

---

## 中文说明

### 项目背景

本项目为计算机相关专业毕业设计作品，由 AI Agent（Claude Code）辅助开发完成。项目目标是构建一个基于 Qt 6 和 QML 的 Linux 桌面网络管理工具，替代或补充现有的 `nm-applet` 等工具，提供更现代化和功能更完整的图形界面。

### 项目简介

基于 Qt 6、QML 和 KF6NetworkManagerQt（NetworkManager 的 Qt 绑定库），通过 D-Bus 与系统 NetworkManager 服务通信，实现完整的网络管理功能。

### 核心功能

- **设备管理** — 实时查看所有网络设备状态（有线/无线/蓝牙/虚拟设备等），支持三态过滤（仅主设备 / 主+副设备 / 全部），提供设备详情弹窗（驱动信息、链路状态、无线参数）
- **Wi-Fi 管理** — 扫描周边热点，按信号强度排序，显示安全类型（WPA2/WPA3 等）、频段（2.4/5/6 GHz）、带宽和 BSSID，支持一键连接/断开，密码输入支持明文/密文切换
- **连接配置** — 新建/编辑/删除已保存的网络连接，支持 DHCP 和手动 IP 配置，内置 Wi-Fi SSID 智能匹配（扫描列表匹配显示加密状态），连接编辑器采用双副本（original/working）模式确保修改可识别
- **运行时监控** — 实时展示当前活跃连接的完整信息：IP 地址、网关、DNS、网络接口、MAC 地址、MTU、有线端口速率、无线 SSID/速率/频段/带宽
- **防自动重连** — 用户手动断开连接时，临时禁用设备 autoconnect 属性，待连接完全断开后再恢复，避免 NetworkManager 自动重连
- **系统状态栏** — 顶部栏显示当前主连接的名称、IP 地址、状态文字和联网状态彩色圆点（绿色=已联网、橙色=受限、红色=无网络），同时提供 Wi-Fi 开关和飞行模式开关

### 技术架构

```
QML UI 层（Main.qml + 页面 + 组件）
  │ 全局上下文属性 "nm"
  ▼
NetworkManagerWrapper（中心桥接层，单例模式）
  │
  ├── DeviceService ←→ DeviceModel ←→ DeviceFilterProxyModel → QML
  ├── ApService ←→ ApModel → QML
  ├── ConnectionRuntimeService ←→ ConnectionRuntimeModel → QML
  ├── ConnectionListModel → QML
  ├── ConnectionManager（连接 CRUD + 激活/断开）→ QML
  └── ConnectionEditorModel（编辑表单，注册为 QML 类型）→ QML
```

| 层次 | 职责 | 关键技术 |
|------|------|----------|
| UI 层 | 界面展示与交互 | Qt Quick, StackLayout, Repeater, ListView |
| 桥接层 | 初始化编排、信号转发 | QQmlContext, Q_PROPERTY, Q_INVOKABLE |
| 服务层 | NM 信号监听、业务逻辑 | 信号/槽、QTimer 防抖、状态机 |
| 模型层 | 数据存储与展示 | QAbstractListModel, QSortFilterProxyModel |
| 数据层 | NM D-Bus 通信 | KF6NetworkManagerQt |

### 构建与运行

```bash
# 依赖
# Qt 6.2+, KF6NetworkManagerQt, CMake 3.16+, NetworkManager

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/appNetworkmanager_GUI_Qt
```

### 目录结构

```
├── CMakeLists.txt                   # CMake 构建配置
├── main.cpp                         # 程序入口，QML 类型注册
├── architecture.md                  # 详细架构文档
├── qml/
│   ├── Main.qml                     # 主窗口（TopBar + SideBar + StackLayout）
│   ├── components/
│   │   ├── TopBar.qml               # 主连接信息 + 联网状态 + 开关
│   │   ├── SideBar.qml              # 三页导航（设备 / Wi-Fi / 已保存网络）
│   │   ├── DeviceCard.qml           # 设备摘要卡片（颜色编码）
│   │   ├── WifiItem.qml             # Wi-Fi AP 卡片
│   │   ├── ConnectionEditorPanel.qml # 连接编辑表单
│   │   └── NewConnectionDialog.qml  # 两步新建连接向导
│   └── pages/
│       ├── DevicePage.qml           # 设备列表页
│       ├── WifiPage.qml             # Wi-Fi 扫描页
│       └── ConnectionsPage.qml      # 连接管理页（状态 + 配置）
└── src/
    ├── wrapper/  nm_wrapper.*       # 中心枢纽
    ├── model/                       # 数据模型
    │   ├── ap/                      #   Wi-Fi AP 模型
    │   ├── connection/              #   连接编辑器 / 列表 / 运行时 / 设置
    │   └── device/                  #   设备信息 / 设备列表
    ├── service/                     # 业务服务
    │   ├── ap_service.*            #   Wi-Fi 扫描
    │   ├── con_runtime_service.*   #   运行时状态追踪
    │   ├── connection_manager.*    #   连接 CRUD
    │   └── device_service.*        #   设备生命周期
    ├── filter/  device_filter.*     # 三态设备过滤
    └── utils/  network_utils.*      # 工具函数集
```

