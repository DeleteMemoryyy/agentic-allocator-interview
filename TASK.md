# Heap Guardian：完整题目描述

## 1. 背景

你接手了一个运行在固定 64 KiB heap 上的内存分配器。它已经通过全部公开回归测试，但线上仍在少量 block resize 场景后出现 heap corruption。最近一次实现由 Coding Agent 协助完成，没有可靠的设计说明；单纯再问 Agent “修好 allocator”并不能构成可信结论。

你需要像接手真实事故一样工作：先建立系统 invariant，再并行委派调查或测试生成，审查 Agent diff，用独立反例验证结论，并在私测给出反证后修正自己的错误假设。

这不是一道“从零背写 malloc”的题，也不要求追求最复杂或最快的数据结构。

## 2. 任务目标

在不改变公开 API 的前提下：

1. 找出并修复 starter 中至少一个潜伏的正确性问题。
2. 保证所有 allocator 合同和 heap 结构 invariant 始终成立。
3. 编写一个尽量小的 `candidate.trace`：
   - 在你的最终实现上通过；
   - 能让原始 starter 失败；
   - 失败原因来自通用 invariant，而不是硬编码实现细节。
4. 在 `decision.md` 中记录 Agent 建议取舍、证据、Phase A 到 Final 的认知变化和剩余风险。
5. 对最终 diff 负责，并能够在人工 Review 中逐行解释关键修改。

## 3. Starter 模型

候选人主要阅读和修改 `allocator.c`。公开接口位于 `allocator.h`：

```c
int allocator_init(void *heap, size_t heap_size);
void *allocator_malloc(size_t size);
void allocator_free(void *ptr);
void *allocator_realloc(void *ptr, size_t size);
void *allocator_calloc(size_t count, size_t size);
int allocator_check(void);
```

Starter 使用一个固定大小的模拟 heap 和按物理地址排列的 block。每个 block 都包含 header、payload 和 footer；实现可以重构，但不能脱离调用方提供的 heap，也不能转发给系统 allocator。

公开 runner 会记录存活 allocation，写入确定性 payload pattern，并在每一步后检查对齐、范围、重叠、payload 保留和 `allocator_check()`。

## 4. 必须满足的合同

### 4.1 Heap 与 block

- `allocator_init` 接收 16-byte aligned heap；题目 runner 使用 64 KiB heap。
- 整个 heap 必须始终被一组连续、不重叠的物理 block 完整覆盖：不能出现 gap，也不能越界。
- block 起始地址和 block size 都按 16 bytes 对齐。
- 任意物理 block 不小于 32 bytes；不能产生小于该下限、无法继续表示的 splinter。
- header/footer 对同一 block 的 size 和 allocation state 必须一致。
- 相邻 free block 必须及时合并。
- 存活 payload 位于 heap 内、16-byte aligned，彼此不重叠。

### 4.2 `allocator_malloc`

- `allocator_malloc(0)` 返回 `NULL`。
- 成功时返回满足对齐和容量要求的 payload。
- OOM 或 size 计算溢出时返回 `NULL`，且不能破坏已有 allocation。

### 4.3 `allocator_free`

- `allocator_free(NULL)` 无操作。
- 对有效存活 pointer 的释放不得破坏其他 payload。
- double free 和任意非法 pointer 不在本题合同内；不要为它们牺牲已声明合同的正确性。

### 4.4 `allocator_realloc`

- `allocator_realloc(NULL, n)` 等价于 `allocator_malloc(n)`。
- `allocator_realloc(p, 0)` 释放 `p` 并返回 `NULL`。
- 成功时保留原 payload 的前 `min(old_payload_size, new_size)` 字节。
- 允许原地扩缩或移动，但失败必须返回 `NULL` 并保持原 allocation 有效且内容不变。
- 任何 split、merge 或移动之后，全部 heap invariant 立即成立。

### 4.5 `allocator_calloc`

- 必须检查 `count * size` 的乘法溢出。
- 成功 allocation 的请求范围全部清零。
- OOM 或溢出返回 `NULL`。

### 4.6 `allocator_check`

- 当且仅当 heap 的结构 invariant 成立时返回 1。
- 检查器不能修改 heap，不能只针对公开 trace，不能通过直接返回 1 绕过验证。
- Runner 会将它与外部 payload/range 检查结合使用；它不是唯一判据。

## 5. 允许和禁止的修改

允许：

- 修改或重写 `allocator.c` 内部实现。
- 在本地添加临时调试代码或额外 trace。
- 使用 debugger、sanitizer 和最多两个 Coding Agent 工作流。

禁止：

- 修改 `allocator.h`、`public_runner.c`、`public-tests/` 或判题协议。
- 调用系统 `malloc/calloc/realloc/free` 承担实际分配。
- 根据 trace 文件名、固定操作序列、特定 ID 或已知隐藏用例硬编码。
- 关闭 sanitizer、伪造退出码或弱化 `allocator_check()` 来掩盖结构错误。
- 将解答提交到本公开仓库。

最终 PR 可以包含额外调试文件，但正式评分只读取三项交付物。

## 6. Trace 格式

`candidate.trace` 是逐行执行的文本文件，`#` 开头为注释，ID 范围为 0–255：

```text
alloc ID SIZE
calloc ID COUNT SIZE
realloc ID NEW_SIZE
free ID
check
expect_null_alloc SIZE
expect_null_calloc COUNT SIZE
```

规则：

- 同一 ID 在释放前不能再次 allocation。
- `free`/`realloc` 只能引用存活 ID。
- Runner 在 allocation 后写入确定性 pattern，并在 realloc/free 及每一步全局验证时检查内容。
- 反例以操作数量少、能清晰解释为优；单纯堆积随机操作不算高质量证据。

## 7. Agent 使用规则

- 整个面试最多使用两个 Coding Agent 工作流，可以异步并行。
- 推荐拆分为“实现/invariant 审计”和“反例/测试生成”，但候选人可以自行设计分工。
- Agent 的结论不是证据。采纳修改前，候选人必须阅读 diff，并用合同、trace 或 sanitizer 独立验证。
- 可以拒绝 Agent patch；高质量拒绝和有证据的重写同样计分。
- 不要求提交完整 Agent transcript，但 `decision.md` 必须说明委派目标、关键建议和你的审查结论。
- 人工 Review 中无法解释的 Agent 代码视为未完成，即使自动测试通过。

## 8. 时间与两阶段提交

### 0–5 分钟：建模

- 阅读代码和 API 合同。
- 写出最关键的 3–5 条 invariant。
- 决定两个 Agent 是否启动以及如何分工。

### 5–25 分钟：Phase A

- 调查、审查 Agent 结果、实现修复。
- 运行 `./verify-public.sh`。
- 编写 `candidate.trace` 和第一版 `decision.md`。
- 第 25 分钟必须冻结 commit；不因 Agent 尚未结束而延期。

### 25–28 分钟：私测反馈

面试官对冻结 commit 运行私有 verifier。反馈只包含状态和失败类别，例如：

- `heap-invariant`
- `payload-corruption`
- `realloc-preservation`
- `overlap`
- `alignment`
- `calloc-overflow`
- `overflow-or-oom`
- `crash-or-sanitizer`
- `timeout`

不会提供隐藏 trace、期望实现或具体修复位置。

### 28–43 分钟：Final

- 根据反馈重新检查原假设。
- 在同一个 Private PR 分支继续提交。
- 更新 `decision.md`，记录 Phase A → Final 的变化。
- 第 43 分钟冻结 Final commit。

### 43–60 分钟：验证与 Review

- 面试官运行最终私测。
- 候选人主导讲解 block 模型、关键 diff、反例、Agent 取舍和剩余风险。

## 9. 提交物要求

### `allocator.c`

- `-std=c11 -Wall -Wextra -Werror` 编译通过。
- 公开和私有 runner 在 sanitizer 下运行。
- 不得改动 API。

### `candidate.trace`

- 至少包含一条非注释操作。
- 在最终实现上通过。
- 能杀死私有 verifier 保存的原始 starter mutation。
- 候选人能解释每一步为什么必要。

### `decision.md`

不超过 300 个中文字符或约 180 个英文词，回答：

1. 你依赖的核心 invariant 是什么？
2. Agent 提议了什么？你采纳、重写或拒绝了什么，为什么？
3. Phase A 私测如何改变了你的错误假设？
4. 仍有哪些风险尚未证明？

## 10. 评分

自动验证 60 分：

- 编译、公开 API 合同：10
- 隐藏正确性与 sanitizer：25
- `candidate.trace` 能杀死 starter mutation：15
- Final 回归稳定且无超时：10

人工 Review 40 分：

- 系统模型与 invariant：10
- Agent 委派和 diff 审查：10
- 独立反例与验证质量：10
- Phase A → Final 的纠错和 ownership：10

仅仅“全部测试通过”不是满分条件；无法解释的代码、无效反例或绕过 verifier 会显著扣分。

## 11. 完成定义

以下条件全部满足才算完成：

- Private PR 中记录了 Phase A 和 Final 两个 commit SHA。
- Final 自动验证通过。
- `candidate.trace` 在最终实现上通过并杀死 starter mutation。
- `decision.md` 完整且未保留模板占位符。
- 候选人能够解释关键内存布局、修复理由和未覆盖风险。

提交操作见 [SUBMISSION.md](SUBMISSION.md)。
