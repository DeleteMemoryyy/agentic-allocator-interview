# 可直接发给候选人的 Prompt

下面整段可以直接复制。面试前 private repo 初始化不计入 20 分钟；正式计时从最后一句“现在开始”算起。

---

这是一个 20 分钟的 AI-assisted 工程交付接管任务。你可以使用任意 Coding Agent、background Agent 或 sub-agent，也可以手工推进；我们评估的是你与自选工作流形成的完整交付系统，不要求提交 Agent transcript。

题目仓库：<https://github.com/DeleteMemoryyy/agentic-allocator-interview>

请不要向 public upstream 提交答案。开始前请用仓库模板创建一个你拥有的 private repo，邀请 GitHub 账号 `DeleteMemoryyy`，并创建 `solution -> main` Draft PR。若 GitHub 权限或网络有问题，直接使用我发送的 candidate zip，最终上传 `submission.zip`，不要消耗计时时间排查环境。

规则：

1. 先阅读 `TASK.md`、`CURRENT_STATE.md`、`CONTEXT.md` 和现有实现；
2. 你接管的是一个尚未获准合并的 PR。目标不是找指定数量的 bug，而是在时间内推进成范围一致、可验证、风险表达准确的 snapshot；
3. 第 7 和第 14 分钟我会发送新的公开 packet。由你决定如何让当前 Agent/workflow 吸收它；不要假设你的 Agent 能与 judge 或 reviewer 通信；
4. 现场没有 hidden-test feedback，也不会实时 review 你的中间提交；
5. 第 20 分钟我会明确说 freeze。届时立即停止修改；之后只允许打包或上传已经冻结的 bytes；
6. 推荐最终运行 `sh ./verify.sh` 和 `python3 package_submission.py`，把 `submission.zip` 附到 private Draft PR。也可以直接提交冻结的 PR head，不要求手抄 Commit SHA；
7. `HANDOFF.md` 可以由 Agent 起草，但其中的行为、测试结果、未完成项、风险和 rollback 必须与实际 snapshot 一致。

请准备好工作区和 Agent。现在开始。

---
