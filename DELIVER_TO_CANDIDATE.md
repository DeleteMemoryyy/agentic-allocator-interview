# 可直接发给候选人的说明

下面整段可以直接复制。私有仓库和本场材料准备不计入 20 分钟；正式计时从最后一句“现在开始”算起。

---

这是一个 20 分钟的 AI 辅助工程交付接管任务。你可以使用任意编码智能体、后台智能体或子智能体，也可以手工推进；我们评估的是你与自选工作流形成的完整交付系统，不要求提交智能体对话记录。

公开模板：<https://github.com/DeleteMemoryyy/agentic-allocator-interview>

请不要向公开上游提交答案。开始前请用模板创建一个你拥有的私有仓库，邀请 GitHub 账号 `DeleteMemoryyy`，并创建 `solution -> main` Draft PR。我会发送本场专属的 `agentic-takeover-session-overlay.zip`；请在私有仓库根目录解压并提交。若 GitHub 权限或网络有问题，直接使用我发送的完整候选人包，不要消耗计时时间排查环境。

正式开始前请确认：`submission.json.assignmentId` 不再是 `public-template`，且 `stage-packets/STAGE-2.packet`、`STAGE-3.packet` 均存在。

规则：

1. 先阅读 `TASK.md`、`CURRENT_STATE.md`、`CONTEXT.md` 和现有实现；
2. 你接管的是一个尚未获准合并的 PR。目标不是找指定数量的缺陷，而是在时间内推进成范围一致、可验证、风险表达准确的快照；
3. 第 7 和第 14 分钟我会分别发送一把本场密钥。收到后运行 `python3 unlock_stage.py stage-packets/STAGE-N.packet --key '<阶段密钥>'`；由你决定怎样让当前工作流吸收新信息；
4. 现场没有隐藏测试反馈，也不会实时审阅你的中间提交；不要假设你的智能体能与判题器或审阅者通信；
5. 第 20 分钟我会明确说“冻结”。届时立即停止修改；之后只允许打包或上传已经冻结的内容；
6. 推荐最终运行 `sh ./verify.sh` 和 `python3 package_submission.py`，把 `submission.zip` 附到私有 Draft PR。也可以提交冻结的 PR head，不要求手抄 Commit SHA；
7. `HANDOFF.md` 可以由智能体起草，但其中的行为、测试结果、未完成项、风险和回滚必须与实际快照一致。

请准备好工作区和智能体。现在开始。

---
