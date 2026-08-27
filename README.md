# Heap Guardian：Agent 时代的 Allocator 工程面试

这是一道 60 分钟、允许使用异步 Coding Agent 的系统工程题。候选人不需要从零实现 `malloc`；你会接手一个公开测试全绿、但线上偶发 heap corruption 的 allocator，并负责定位错误假设、审查 Agent 产出、构造独立反例和完成私测纠错。

仓库只包含候选人可见材料。隐藏测试、参考解和面试官答案不在这里。

## 开始面试

**不要 Fork，也不要直接向本公开仓库提交 PR。**公开仓库上的分支和 PR 都是公开的。

点击下面的链接，从模板创建一个属于你的 **Private Repository**：

[Create private interview repository](https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private)

随后阅读：

1. [完整题目 TASK.md](TASK.md)
2. [私有 PR 提交流程 SUBMISSION.md](SUBMISSION.md)
3. 修改 `allocator.c`、`candidate.trace` 和 `decision.md`

## 本地公开验证

环境要求：macOS/Linux、C11 compiler；脚本默认使用 `clang`，也可以通过 `CC` 指定编译器。

```bash
./verify-public.sh
```

公开验证会启用 AddressSanitizer 和 UndefinedBehaviorSanitizer。Starter 应当通过全部公开测试；这只证明环境正常，不代表题目已经完成。

## 时间盒

- 0–5 分钟：阅读合同，建立 allocator invariant。
- 5–25 分钟：最多使用两个异步 Coding Agent，审查结果并冻结 Phase A。
- 25–28 分钟：提交 Draft PR；面试官运行私测，只返回失败类别。
- 28–43 分钟：基于反馈纠错并推送 Final commit。
- 43–47 分钟：最终私测。
- 47–60 分钟：候选人主导代码 Review 和设计复盘。

## 交付物

Private PR 中只评审：

- `allocator.c`
- `candidate.trace`
- `decision.md`

GitHub Actions 会在候选人的 Private PR 上自动运行公开测试。私有 verifier 由面试官独立运行，不会进入候选人的仓库。

## 许可与用途

代码以 MIT License 发布。这是面试原型，不是生产级通用 allocator。
