# Durable Dispatch Protocol — Takeover Task

你正在接管一个由 Coding Agent 提交、但尚未获准合并的 PR。它为后台 Job Runner 增加了幂等提交，公开 happy-path tests 已经通过；reviewer 仍不确定状态所有权、并发 admission、失败恢复和交付证据是否足够。

你的目标不是从零重写系统，也不是找出指定数量的 bug。你有 20 分钟，把当前交付推进到一个你愿意负责的状态：可以保留、缩小、重写或回退已有实现，但最终 snapshot 必须可验证，并准确表达它选择了什么、拒绝了什么和还承担什么风险。

先阅读 `CONTEXT.md` 中的领域词汇。调用接口目前是：

```python
job = service.submit(
    tenant_id="tenant-a",
    idempotency_key="request-42",
    payload="build release",
)
```

## Stage 1 已公开合同

1. idempotency scope 是 `(tenant_id, idempotency_key)`。同 scope、同 payload 的重试必须返回同一个 job id；不同 tenant 互不影响；
2. 同 scope 的 key 携带不同 payload 时必须抛出已有的 `IdempotencyConflict`，且不能产生新的持久状态或 dispatch；
3. `JobService` 和 `JobStore` wrapper 都可能重建。只有共享的 backend repository state 可被视为 authoritative；
4. 已成功 dispatch 的逻辑请求不能再次 dispatch；
5. 继承的 `DispatchError` 表示 dispatcher **确定没有接受**本次请求。service 应抛出 `DispatchUnavailable`，保留同一个 pending job，后续重试可以再次 dispatch；
6. 不同 wrapper 可能并发提交同一个逻辑请求。最终只能有一个 durable job；成功路径不能产生两个外部 dispatch；
7. admission 热路径不得调用 `all_jobs()` 或通过全量扫描寻找 key；
8. 只能使用 Python 标准库，本地验证必须在一分钟内完成。

## 三个阶段

- **Stage 1 · Takeover（0–7 分钟）**：阅读当前实现和证据，决定保留、重写或回退哪些部分，并开始推进；
- **Stage 2 · Incident Update（第 7 分钟）**：面试官会发送一份新的生产证据与合同澄清；
- **Stage 3 · Release Gate（第 14 分钟）**：面试官会发送最终交付门槛；
- **第 20 分钟**：冻结并发送交付快照。

后续更新不会推翻已经公开的合同，但会补充当前尚未定义的 dispatch outcome。不要假设 judge、reviewer 或面试官可以与你的 Agent 通信，也不会获得隐藏测试反馈。

## 允许修改

- `workspace/jobrunner/` 下的实现（后续 release profile 会约束允许改变的 public seam）；
- 在 `workspace/tests/` 中新增测试；
- `HANDOFF.md`；
- `submission.json` 中明确允许填写的字段。

`verify.sh`、`package_submission.py`、`CONTEXT.md`、既有 `test_public.py` 和任务/阶段文档视为合同，不应修改。

## 本地验证

```bash
sh ./verify.sh
```

Starter 通过公开验证，这只能说明当前 PR 的 happy path 可运行。公开测试不是完整评分器，也不会在现场返回 hidden feedback。

`MemoryJobBackend` 是真实持久层的进程内替身。不要把它误解成生产存储实现；它存在是为了让 wrapper 生命周期、原子 admission 和状态转换可以被确定性测试。

## 最终交付

推荐运行：

```bash
python3 package_submission.py
```

它会生成 `submission.zip`。也可以提交一个能恢复相同工作区的稳定 Git ref 或 PR revision。`HANDOFF.md` 可以完全由 Agent 起草，但其中的事实必须与实际 diff 和可重放结果一致。
