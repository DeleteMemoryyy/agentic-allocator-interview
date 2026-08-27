# Current PR State

原始 change request：调用方会使用 `idempotency_key` 重试 Job 提交，重复请求不应创建或 dispatch 第二个 job。

上一位 Agent 的实现：

- 在 `JobService` 中增加 `_idempotency` 字典；
- 首次提交时创建 `JobRecord`，写入 store，并调用 dispatcher；
- 相同 key 再次提交时从 service-local 字典找到旧 job；
- 增加了三个公开 happy-path tests；
- 自报 `./verify.sh` 通过，认为可以合并。

未解决的 review comments：

> `JobService` 的生命周期是否真的拥有 idempotency？如果 service 被重新创建，当前实现依赖的事实还存在吗？

> 两个 wrapper 同时看不到 key 时，会不会各自 insert 并 dispatch？`insert` 和 key reservation 的原子边界在哪里？

当前没有 production incident 的完整复现，也没有覆盖 wrapper recreation、冲突 payload、并发 admission 或失败恢复的测试。dispatcher 对“调用失败”的返回语义目前只覆盖确定拒绝；是否存在结果不可判定的失败，会在后续阶段补充。

仓库中已经存在 `MemoryJobBackend`。它模拟可被多个 `JobStore` wrapper 复用的 backend，当前 PR 却没有利用这个生命周期边界表达 idempotency ownership 或原子状态转换。
