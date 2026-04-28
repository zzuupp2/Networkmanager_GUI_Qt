# NetworkManagerWrapper QML 暴露整理

## 已有连接相关属性

- `connectionListModel`
- `connectionRuntimeModel`
- `connectionManager`（兼容保留）

## 新增简化属性（供 QML 使用）

- `manager` -> `ConnectionManager*`
- `editor` -> `ConnectionEditorModel*`
- `runtimeModel` -> `ConnectionRuntimeModel*`（`connectionRuntimeModel` 的简化别名）
  - 支持 `getRuntimeByUuid(uuid)`，便于跟随左侧连接选项切换读取单项运行时信息
- `currentUuid: QString`（统一选中项状态）

## 统一选择入口（推荐）

- `selectConnection(uuid)`：统一处理“选中 + editor 加载”
- `firstConnectionUuid()`：取列表第一项 uuid（用于默认选中）
- `hasConnection(uuid)`：校验当前 uuid 是否仍有效

## 命名建议

- 业务逻辑调用优先 `nm.manager.*`
- 编辑状态绑定优先 `nm.editor.*`
- 运行时状态读取优先 `nm.runtimeModel`（或 `nm.connectionRuntimeModel`）
- 旧命名 `nm.connectionManager` 继续保留，便于平滑迁移
