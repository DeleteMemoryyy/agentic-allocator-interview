# Agentic Allocator Incident V2

一场 60 分钟、允许使用异步 Coding Agent 的系统工程面试。你要修复一个固定堆 allocator 的状态一致性问题，补全独立 heap checker，并提交一个不超过 40 步、能区分正确实现与多种“看似合理修复”的反例 trace。

这不是从零实现 malloc，也不是比拼手写速度。我们看的是：如何拆任务给 Agent、如何 Review 生成代码、如何建立跨表示 invariant、如何用反例纠错，以及是否能对最终结果负责。

## 开始前

请不要 Fork 或向本公开仓库提交解答。使用下面的模板链接创建一个**候选人自己拥有的 Private Repository**：

<https://github.com/new?template_owner=DeleteMemoryyy&template_name=agentic-allocator-interview&visibility=private>

然后：

1. 邀请面试官 GitHub 账号 `DeleteMemoryyy` 为 collaborator；
2. 从 `main` 创建 `solution` 分支；
3. 在你的私有仓库内创建 `solution -> main` 的 Draft PR；
4. 在 25 分钟和最终提交时分别 push commit，并把两个 SHA 写进 PR 描述。

完整规则见 [TASK.md](TASK.md)，提交步骤见 [SUBMISSION.md](SUBMISSION.md)。

## 本地验证

环境只需要 macOS/Linux、`clang` 或 `cc`、POSIX shell，不需要 Docker 或额外依赖：

```bash
./verify-public.sh
```

Starter 会通过公开测试；这只证明接口基线可运行，不表示题目已完成。隐藏判题会额外检查状态一致性、checker 完备性、随机压力、搜索预算，以及你的 trace 能击穿多少个隐藏 mutation。

## 允许修改

- `allocator.c`
- `heap_checker.c`
- `candidate.trace`
- `decision.md`

其他文件视为合同。隐藏判题使用干净副本，不接受对 runner、头文件或测试脚本的修改。

## 时间盒

- 0–5 分钟：读合同、画状态模型、拆 Agent 任务；
- 5–25 分钟：实现 Phase A；25:00 冻结并提交 SHA；
- 25–28 分钟：运行一次 Phase A 隐藏判题，获得最多两个失败维度与 mutation 击杀数；
- 28–43 分钟：纠错并提交 Final SHA；
- 43–47 分钟：最终判题；
- 47–60 分钟：人工 Review 与追问。

同时最多运行两个 Coding Agent。你可以让它们异步探索，但最终合并、验证和解释必须由你完成。
