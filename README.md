# 后台任务幂等提交——20 分钟智能体工程接管

这是一个允许使用编码智能体、后台智能体或子智能体的工程面试模块。你会接手一个正常路径测试已经通过、但尚未获准合并的 PR；第 7 和第 14 分钟会收到新的本场信息，第 20 分钟冻结可验证快照。

环境只有 Python 标准库，不需要 Docker、依赖安装或网络服务。考察重点是候选人与其自选工作流能否理解已有代码、处理不完整信息、做出工程决策、纠正错误，并留下可复现的证据和准确的风险说明。

## 公开模板不是完整本场题目

本仓库只包含 0:00 的公开模板，不保存可复用的阶段密文或密钥。正式计时前，面试官会提供本场专属材料：

- 已从本模板创建私有仓库：收到 `agentic-takeover-session-overlay.zip` 后，在仓库根目录解压；
- 不使用 GitHub：直接使用面试官发送的完整 `agentic-takeover-candidate-session.zip`。

开始前应确认：

```text
submission.json 中的 assignmentId 不再是 public-template
stage-packets/STAGE-2.packet 存在
stage-packets/STAGE-3.packet 存在
```

这些准备不计入 20 分钟。正式阶段包每场重新生成，上一场密钥不能解锁本场内容。

## 私有 PR 准备

不要向本公开仓库提交答案。使用模板创建你自己拥有的私有仓库：

<https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private>

邀请面试官 GitHub 账号 `DeleteMemoryyy`，创建 `solution` 分支和 `solution -> main` Draft PR。然后把本场 overlay 解压到仓库根目录并提交。完整步骤见 [SUBMISSION.md](SUBMISSION.md)。

## 0:00 阅读顺序

- [TASK.md](TASK.md)：公开要求、时间线和允许修改范围；
- [CURRENT_STATE.md](CURRENT_STATE.md)：继承 PR 与尚未解决的审阅意见；
- [CONTEXT.md](CONTEXT.md)：理解代码所需的最小领域词汇；
- [HANDOFF_PROMPT.md](HANDOFF_PROMPT.md)：最终交付说明格式。

第 7 分钟收到密钥后运行：

```bash
python3 unlock_stage.py stage-packets/STAGE-2.packet --key '<阶段密钥>'
```

第 14 分钟对 `STAGE-3.packet` 使用对应密钥。错误密钥或被修改的文件会认证失败且不会输出明文。你可以把解锁后的文本交给当前智能体；现场没有隐藏测试反馈，判题器和审阅者也不会与你的智能体通信。

## 本地验证

```bash
sh ./verify.sh
```

初始版本会通过公开的正常路径测试，这不表示任务已经完成。

## 第 20 分钟冻结

听到冻结后，停止修改代码、测试、`submission.json` 和 `HANDOFF.md`。随后可以运行：

```bash
python3 package_submission.py
```

把生成的 `submission.zip` 附到私有 Draft PR，或直接提交冻结的 PR head。无需手抄 Commit SHA，但冻结后不能再补测试、改说明、amend、rebase 或 force-push。不需要提交智能体对话记录。

面试官可直接复制 [DELIVER_TO_CANDIDATE.md](DELIVER_TO_CANDIDATE.md) 中的说明。
