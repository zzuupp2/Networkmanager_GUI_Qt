# ConnectionEditorModel QML 暴露清单

本文档用于 Wrapper 后续向 QML 暴露 `ConnectionEditorModel` 时对齐接口。

## 只读属性

- `uuid: QString`
- `type: QString`
- `isModified: bool`
- `isNew: bool`

## 可编辑属性

- `id: QString`
- `autoconnect: bool`
- `autoconnectPriority: int`
- `interfaceName: QString`
- `ssid: QString`
- `wirelessSecurity: QString`
- `wirelessPassword: QString`
- `mtu: int`
- `ipv4Method: QString`
- `ipv4Address: QString`
- `ipv4Gateway: QString`
- `ipv4Dns: QStringList`

## QML 可调用方法（Q_INVOKABLE）

- `reset()`
- `setField(field: QString, value: QVariant) -> bool`
- `applyPatch(patch: QVariantMap)`
- `toSettingInfo() -> ConnectionSettingInfo`
- `toSettingsMap() -> QVariantMap`

## 建议 Wrapper 透传对象

- `editor`（`ConnectionEditorModel*`）

> 说明：本清单只整理 Editor 模型本身接口，不涉及 `ConnectionManager`/UI 绑定命名调整。
>
> 变更说明：
> - `uuid`/`type` 不是 `CONSTANT`，为可通知只读属性（加载新数据时会触发刷新）
> - 字段更新以 Q_PROPERTY setter 为主路径，`setField/applyPatch` 仅作为动态补丁入口并复用 setter
