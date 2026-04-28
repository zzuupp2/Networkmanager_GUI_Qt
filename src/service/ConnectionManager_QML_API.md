# ConnectionManager QML 暴露清单

用于 Wrapper 向 QML 暴露连接管理能力时的接口基线。

## 原始接口（兼容保留）

- `addConnection(settings: NMVariantMapMap) -> void`
- `updateConnection(uuid: QString, newSettings: NMVariantMapMap) -> void`
- `deleteConnection(uuid: QString) -> void`
- `activateConnection(uuid: QString) -> void`
- `deactivateConnection(uuid: QString) -> void`

## 简化接口（推荐 QML 使用）

- `apply(settings: QVariantMap, isNew: bool, uuid: QString = "") -> void`
- `remove(uuid: QString) -> void`
- `activate(uuid: QString) -> void`
- `deactivate(uuid: QString) -> void`

## 结果通知（异步）

- `operationCompleted(uuid: QString, success: bool)`
- `errorOccurred(uuid: QString, error: QString)`
- `connectionAdded/connectionUpdated/connectionRemoved`
