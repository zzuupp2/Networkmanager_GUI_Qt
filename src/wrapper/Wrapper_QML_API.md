# NetworkManagerWrapper QML 暴露整理

## 已有连接相关属性

- `connectionListModel`
- `connectionManager`（兼容保留）

## 新增简化属性（供 QML 使用）

- `manager` -> `ConnectionManager*`
- `editor` -> `ConnectionEditorModel*`

## 命名建议

- 业务逻辑调用优先 `nm.manager.*`
- 编辑状态绑定优先 `nm.editor.*`
- 旧命名 `nm.connectionManager` 继续保留，便于平滑迁移
