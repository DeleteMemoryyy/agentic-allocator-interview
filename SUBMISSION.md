# 私有 PR 与快照提交流程

公开仓库只用于分发模板。不要向公开上游提交解答或本场阶段包。

## 面试前：创建私有工作区

打开：

<https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private>

确认可见性为 **Private**，并邀请面试官账号 `DeleteMemoryyy` 为协作者。

```bash
git checkout -b solution
git push -u origin solution
```

在自己的私有仓库内创建 `solution -> main` Draft PR。

面试官会发送本场专属 overlay。在仓库根目录运行：

```bash
unzip -o /path/to/agentic-takeover-session-overlay.zip -d .
git add submission.json stage-packets/
git commit -m "prepare interview session"
git push
```

确认 `assignmentId` 已替换且两个 `.packet` 文件存在，再开始计时。以上准备不占 20 分钟；不要把 overlay 或密钥推到公开上游。

## 20:00：冻结

听到“冻结”后立即停止修改代码、测试、`submission.json` 和 `HANDOFF.md`。传输和打包可以继续，但只能复制已经冻结的内容。

推荐：

```bash
sh ./verify.sh
python3 package_submission.py
git add workspace HANDOFF.md submission.json
git commit -m "freeze interview delivery"
git push
```

把 `submission.zip` 附到 Draft PR，或告知面试官 PR 已冻结。PR head 会记录版本，因此不要求手抄 Commit SHA；不要在冻结后 amend、rebase、force-push 或补修。

最终交付至少包含：

- `workspace/`
- `HANDOFF.md`
- `submission.json`

## GitHub 不可用时

直接使用面试官提供的完整候选人包，运行 `python3 package_submission.py` 并上传 `submission.zip`。如果打包脚本失败，可以压缩上述三个项目；不要附带 `.git`、缓存、构建目录或智能体对话记录。
