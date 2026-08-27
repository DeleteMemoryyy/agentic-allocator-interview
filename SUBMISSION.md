# Private PR / Snapshot 提交流程

Public repository 只用于分发初始题目。不要向 public upstream 提交解答。

## 面试前：创建 private workspace

打开：

<https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private>

确认 Visibility 为 **Private**，并邀请面试官账号 `DeleteMemoryyy` 为 collaborator。

```bash
git checkout -b solution
git push -u origin solution
```

在你的 private repo 内创建 `solution -> main` Draft PR。以上准备不占 20 分钟实现时间；若权限或网络受阻，直接使用面试官提供的 zip，不要消耗任务时间排查 GitHub。

## 20:00：冻结

听到 freeze 后立即停止修改代码、测试、`submission.json` 和 `HANDOFF.md`。传输和打包可以继续，但只能复制已经冻结的 bytes。

推荐：

```bash
sh ./verify.sh
python3 package_submission.py
git add workspace HANDOFF.md submission.json
git commit -m "freeze interview delivery"
git push
```

把 `submission.zip` 附到 Draft PR，或告知面试官 PR 已冻结。PR head 本身会记录 revision，因此不要求手抄 Commit SHA；不要在 freeze 后 amend、rebase、force-push 或补修。

最终交付至少应包含：

- `workspace/`
- `HANDOFF.md`
- `submission.json`

## GitHub 不可用时

直接运行 `python3 package_submission.py` 并上传 `submission.zip`。如果打包脚本失败，可以压缩上述三个项目；不要附带 `.git`、cache、build 目录或 Agent transcript。
