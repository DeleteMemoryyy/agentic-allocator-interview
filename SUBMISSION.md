# Private PR 提交流程

公开模板只用于分发题目，禁止把解答 PR 发到公开仓库。

## 1. 创建私有仓库

打开：

<https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private>

确认 Visibility 为 **Private**。建议仓库名 `allocator-interview-<your-name>`。

## 2. 邀请面试官

在你的私有仓库进入 `Settings → Collaborators → Add people`，邀请面试官账号：

```text
DeleteMemoryyy
```

面试官接受邀请后才能看到 PR 和 commit。不要把仓库临时改成 Public。

## 3. 建 Draft PR

```bash
git checkout -b solution
git push -u origin solution
```

在该私有仓库内创建 `solution -> main` 的 **Draft pull request**。PR 描述按模板填写 Agent 分工和验证证据。

## 4. 两次冻结

25:00 前：

```bash
./verify-public.sh
git add allocator.c heap_checker.c candidate.trace decision.md
git commit -m "phase-a: allocator recovery"
git push
git rev-parse HEAD
```

把输出 SHA 填入 PR 的 Phase A 区域。Phase A SHA 冻结后不要 rebase 或 force-push。

43:00 前提交 Final commit，再把 Final SHA 写入同一 PR。保留两个 commit，方便面试官 Review 你如何根据反馈纠错。

## 5. 无法使用 GitHub 时

将四个允许修改的文件放入同一目录后压缩交给面试官。时间点和两个版本仍需保留；不要附带 `.git`、build 目录或 Agent 对话记录。
