# 后台任务幂等提交——工程接管任务

你正在接管一个由编码智能体提交、但尚未获准合并的 PR。它为后台任务运行器增加了幂等提交，公开的正常路径测试已经通过；审阅者仍不确定它在重建、并发和失败条件下是否可靠。

你的目标不是找出指定数量的缺陷，也不是从零重写系统。你有 20 分钟，把当前交付推进到一个你愿意负责的状态：可以保留、缩小、重写或回退已有实现，但最终快照必须可验证，且交付说明必须与实际代码一致。

先阅读 `CURRENT_STATE.md`、`CONTEXT.md` 和现有实现。当前调用方式是：

```python
job = service.submit(
    tenant_id="tenant-a",
    idempotency_key="request-42",
    payload="build release",
)
```

## 阶段 1 已公开要求

1. 幂等范围是 `(tenant_id, idempotency_key)`。同一范围、相同 `payload` 的重试必须返回同一个任务 ID；不同租户互不影响；
2. 同一范围的 key 携带不同 `payload` 时，必须抛出已有的 `IdempotencyConflict`，且不能创建新任务或触发外部派发；
3. `JobService` 和 `JobStore` 都可能在两次请求之间被销毁并重建；重建后仍须满足上述幂等要求；
4. 已经成功派发的逻辑请求不能再次派发；
5. 继承的 `DispatchError` 表示派发器确定没有接受本次请求。服务应抛出 `DispatchUnavailable`；后续相同请求可以安全地再次尝试，但必须沿用原任务；
6. 多个独立的服务/存储包装器可能并发提交同一个逻辑请求。最终只能创建一个任务；成功路径只能产生一个外部副作用；
7. 生产环境可能累积千万级历史任务。一次提交或重试的查找工作量必须与历史任务总数无关；
8. 只能使用 Python 标准库，本地验证必须在一分钟内完成。

## 时间线

- **0–7 分钟**：接管现有实现并处理当前公开要求；
- **第 7 分钟**：面试官给出本场专属的阶段 2 密钥，内容包含一条生产事故证据和本场环境约束；
- **第 14 分钟**：面试官给出本场专属的阶段 3 密钥，内容只补充冻结与交付要求；
- **第 20 分钟**：停止修改并发送冻结快照。

收到密钥后运行：

```bash
python3 unlock_stage.py stage-packets/STAGE-2.packet --key '<阶段密钥>'
```

阶段 3 使用对应文件。你可以把解锁后的标准输出交给当前智能体。现场没有隐藏测试反馈；判题器、审阅者和面试官不会与你的智能体建立通信通道。

## 允许修改

- `workspace/jobrunner/` 下的实现；
- 在 `workspace/tests/` 中新增测试；
- `HANDOFF.md`；
- `submission.json.tooling` 中的自报字段。

以下内容是合同或本场分配信息，不应修改：`verify.sh`、`package_submission.py`、`unlock_stage.py`、`stage-packets/`、`CONTEXT.md`、`CURRENT_STATE.md`、既有 `test_public.py`、任务/阶段文档，以及 `submission.json` 中 `tooling` 以外的字段。

## 本地验证

```bash
sh ./verify.sh
```

初始版本通过公开验证，只能说明正常路径可运行。公开测试不是完整评分器。

`MemoryJobBackend` 是真实持久层的进程内替身，用于确定性模拟多个包装器共享同一份持久状态；它不等于生产数据库实现。

## 最终交付

冻结前请把 `submission.json.tooling` 中的占位值替换为本场实际使用情况；保留其他字段不变。

推荐运行：

```bash
python3 package_submission.py
```

它会生成 `submission.zip`。也可以提交能恢复同一工作区的私有 PR 版本。`HANDOFF.md` 可以由智能体起草，但其中的行为、证据、风险与回滚必须和冻结快照一致。
