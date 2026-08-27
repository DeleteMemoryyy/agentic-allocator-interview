# Durable Dispatch Protocol — Agentic Takeover V4

一个 **20 分钟、允许使用 Coding Agent / sub-agent** 的工程交付接管模块。你会接手一个 happy-path tests 已通过、但还不能合并的 Job Runner PR；面试过程中会在固定时间收到新的公开 evidence，并在第 20 分钟冻结可验证 snapshot。

这不是手写算法题，也不是从零搭 App。环境只有 Python 标准库，解压或 clone 后即可验证。考核对象是候选人与其自选 Agent 工作流组成的 Candidate Work System：能否识别真正的状态所有权、吸收阶段性信息、做出范围一致的架构决策，并留下 reviewer 可以复现的证据和风险说明。

## 面试前准备：Private PR

不要向本 public repository 提交答案。使用模板创建候选人自己拥有的 **Private Repository**：

<https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private>

随后邀请面试官 GitHub 账号 `DeleteMemoryyy`，从 `main` 创建 `solution` 分支，并在自己的 private repo 内建立 `solution -> main` Draft PR。完整步骤见 [SUBMISSION.md](SUBMISSION.md)。GitHub 不可用时可以直接提交 `submission.zip`。

## 0:00 收到的内容

先读：

- [TASK.md](TASK.md)：公开合同、时间线、允许修改范围；
- [CURRENT_STATE.md](CURRENT_STATE.md)：继承 PR 与未解决 review；
- [CONTEXT.md](CONTEXT.md)：领域词汇；
- [HANDOFF_PROMPT.md](HANDOFF_PROMPT.md)：最终 Handoff 约束。

本仓库只包含初始 packet。第 7 和第 14 分钟的更新由面试官单独发送；public repo 不包含 hidden judge、参考解或现场反馈。

## 本地验证

```bash
sh ./verify.sh
```

Starter 通过公开 happy-path tests；这不表示任务已经完成。无需 Docker、依赖安装或网络服务。

## 最终 snapshot

第 20 分钟停止一切代码、测试、metadata 和 Handoff 修改。推荐随后运行：

```bash
python3 package_submission.py
```

将生成的 `submission.zip` 附到 private Draft PR，或提交当时的 PR head。无需手抄 Commit SHA；只需保证 freeze 后没有继续修改。Agent transcript 不提交。

面试官可直接复制 [DELIVER_TO_CANDIDATE.md](DELIVER_TO_CANDIDATE.md) 中的消息发送给候选人。

> Status: V4 pilot。题目和事后 judge 已完成结构自检；招聘阈值仍应结合当期 Agent 基线校准。
