# Private PR 提交说明

## 为什么不能向公开题库直接提 PR

GitHub 上公开仓库的 PR、分支、commit 和 diff 都可被其他人看到。即使把 PR 标记为 Draft，它也不是 private。为避免泄露答案，本题采用：

> 公开 Template Repository → 候选人创建个人 Private Repository → 在该私有仓库内部提交 Draft PR。

不要 Fork 本仓库，也不要向 `DeleteMemoryyy/agentic-allocator-interview` 提交解答 PR。

## 面试前：创建私有仓库

1. 打开 [Create private interview repository](https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private)。
2. 确认来源模板是 `DeleteMemoryyy/agentic-allocator-interview`。
3. Owner 选择你自己的 GitHub account。
4. 仓库名建议：`allocator-interview-<candidate-id>`。
5. Visibility 必须选择 **Private**。
6. 不要选择包含其他分支；只从默认 `main` 创建。
7. 创建后进入 `Settings → Collaborators`，邀请面试官提供的 GitHub 用户名。本场默认可邀请 `DeleteMemoryyy`；如面试官另行指定，以现场信息为准。
8. 等待面试官确认能够打开该私有仓库后再开始计时。

GitHub 官方说明：任何对模板仓库有读取权限的人都可以从模板生成新仓库，并在创建时选择可见性；私人仓库的协作者接受邀请后才能访问内容。

## 本地准备

```bash
git clone https://github.com/<your-account>/allocator-interview-<candidate-id>.git
cd allocator-interview-<candidate-id>
git switch -c submission/<candidate-id>
./verify-public.sh
```

初始公开测试应该全部通过。如果环境失败，立即告诉面试官；环境处理不计入解题时间。

## Phase A：第 25 分钟冻结

1. 保存三个交付物：`allocator.c`、`candidate.trace`、`decision.md`。
2. 运行公开测试并记录结果。
3. 提交并推送：

   ```bash
   git add allocator.c candidate.trace decision.md
   git commit -m "phase-a: allocator investigation"
   git push -u origin submission/<candidate-id>
   ```

4. 在**你的 Private Repository 内部**创建 PR：
   - base：`main`
   - compare：`submission/<candidate-id>`
   - 选择 **Create draft pull request**
   - 标题：`[Phase A] <candidate-id> allocator submission`
5. 在 PR 模板中填写 Phase A SHA、Agent 分工、当前 invariant 和公开测试状态。
6. 把这个 Private PR 链接发给面试官。

第 25 分钟之后，不要修改或 force-push Phase A commit。后续纠错使用新 commit，使面试官能直接 Review 两阶段差异。

## Final：第 43 分钟冻结

在同一分支和同一个 Draft PR 上继续：

```bash
git add allocator.c candidate.trace decision.md
git commit -m "final: correct allocator invariants"
git push
```

随后：

1. 在 PR 描述中填写 Final SHA。
2. 记录私测失败类别以及它改变了什么假设，不要猜测或抄写隐藏用例。
3. 把标题改为 `[Final] <candidate-id> allocator submission`。
4. 保持 Draft 状态，不要 Merge，不要关闭 PR。

## PR 中必须能看到

- Phase A commit SHA 和 Final commit SHA。
- 两个阶段之间清晰、可 Review 的增量 diff。
- 公开 GitHub Actions check 状态。
- `candidate.trace` 的目的和最小性说明。
- Agent 委派内容、采纳/拒绝理由和候选人的独立证据。
- 剩余风险。

## 隐私检查

提交前确认：

- 浏览器仓库标题旁显示 **Private**。
- PR URL 属于你自己的私有仓库，而不是公开模板仓库。
- 已邀请且只邀请本场需要的面试官账号。
- 没有把解答贴到公开 Issue、Discussion、Gist 或聊天链接。
- 不要把 token、邮箱、系统路径、Agent 密钥或私有日志写入 commit。

## 常见问题

### 看不到 “Use this template”

直接使用本文顶部的创建链接；确认已经登录 GitHub。

### 无法创建 Private Repository

可能是组织策略限制。改用个人 GitHub account，或让面试官在受控组织中预先创建私有仓库并邀请你。

### 面试官打不开 PR

检查 `Settings → Collaborators` 中的邀请是否已发送、面试官是否接受，以及 PR 是否创建在正确的 Private Repository。

### GitHub Actions 没运行

先在本地执行 `./verify-public.sh`。如果私有仓库禁用了 Actions，把终端输出贴到 PR，并让面试官继续使用独立 private verifier；不要为修 CI 消耗解题时间。

参考：[GitHub 创建模板仓库说明](https://docs.github.com/en/repositories/creating-and-managing-repositories/creating-a-repository-from-a-template)、[邀请私有仓库协作者](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/repository-access-and-collaboration/inviting-collaborators-to-a-personal-repository)。
