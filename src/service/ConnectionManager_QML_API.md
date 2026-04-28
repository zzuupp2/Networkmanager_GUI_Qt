# ConnectionManager QML 暴露清单

用于 Wrapper 向 QML 暴露连接管理能力时的接口基线。

## 原始接口（兼容保留）

- `addConnection(settings: NMVariantMapMap) -> QString`
- `updateConnection(uuid: QString, newSettings: NMVariantMapMap) -> bool`
- `deleteConnection(uuid: QString) -> bool`
- `activateConnection(uuid: QString) -> bool`
- `deactivateConnection(uuid: QString) -> bool`

## 简化接口（推荐 QML 使用）

- `apply(settings: QVariantMap, isNew: bool, uuid: QString = "") -> QString`
  - `isNew=true` 时执行新增并返回新 UUID
  - `isNew=false` 时执行更新并返回目标 UUID
- `remove(uuid: QString) -> bool`
- `activate(uuid: QString) -> bool`
- `deactivate(uuid: QString) -> bool`
