# 最小领域背景

## 逻辑请求（Logical Request）

由 `(tenant_id, idempotency_key)` 和一个 `payload` 共同标识的一次调用方操作。

## 任务记录（Job Record）

系统为一次逻辑请求保存的记录。公开兼容字段见 `workspace/jobrunner/models.py`。

## 包装器（Wrapper）

`JobService` 或 `JobStore` 的一个进程内实例。包装器可以被销毁、重建，多个包装器也可能同时存在。

## 派发尝试（Dispatch Attempt）

跨过派发器接口的一次调用。它可能产生队列侧的外部副作用。

## 确定拒绝（Definitive Rejection）

能够证明某次派发尝试没有产生外部副作用的结果。阶段 1 中的 `DispatchError` 具有这一语义。
