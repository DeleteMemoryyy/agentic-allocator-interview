# 题目：Agentic Allocator Incident V2

## 1. 事故背景

服务内有一个只管理固定内存区间的 allocator。它同时维护两种表示：

- **物理 block chain**：用 block size 与 `prev_size` 覆盖整个 heap；它是 authoritative state；
- **segregated free bins**：按 size class 索引空闲 block；它是为了加速搜索而维护的 derived state。

混合 `malloc/free/realloc/calloc`、内存压力和失败返回组合出现时，线上观察到“物理上仍有空闲空间，但后续请求无法找到”的现象。现有 checker 也可能把损坏状态判为正常。Starter 通过全部公开测试，但至少存在一条真实状态转换缺陷，checker 也不满足完整合同。

你的任务不是重写整个 allocator，而是在现有实现上完成以下交付：

1. 修复 `allocator.c` 中会破坏合同的状态转换；
2. 补全 `heap_checker.c`，让它能独立验证任意 `heap_snapshot_t`；
3. 编写一个最多 40 步的 `candidate.trace`，通过你的实现，并尽可能击穿隐藏的 plausible mutations；
4. 在 `decision.md` 记录状态模型、Agent Review、Phase A 后的假设变化和剩余风险。

## 2. 必须保持的合同

### 2.1 固定堆与 block

- allocator 只能使用 `allocator_init` 提供的 heap；不得调用系统 `malloc/calloc/realloc/free`；
- heap 起始地址及所有返回 payload 均为 16-byte aligned；
- 物理 block size 包含 header，必须是 16 的倍数且至少为 32 bytes；
- 所有物理 block 必须无缝、无重叠地覆盖完整 heap；
- 除第一个 block 外，`prev_size` 必须等于前一个物理 block 的 size；
- 相邻 free block 必须及时 coalesce。

### 2.2 API 语义

- `allocator_malloc(0)` 返回 `NULL`；无法满足或 size 计算溢出时返回 `NULL`；
- `allocator_free(NULL)` 无操作。测试只会传入当前有效 allocation 的起始地址；
- `allocator_realloc(NULL, n)` 等价于 malloc；`allocator_realloc(p, 0)` 释放并返回 `NULL`；
- realloc 成功时保留 `min(old_payload, new_size)` 字节；
- realloc 失败必须具备 **failure atomicity**：旧 allocation 的地址、内容和全部 allocator 元数据保持可用且一致；
- calloc 乘法溢出时返回 `NULL`；成功时 payload 全部清零。

### 2.3 Free-bin 派生索引

对 snapshot 中的 free bins，`heap_check` 必须证明：

- 每个物理 free block 恰好出现一次；
- allocated block 不得出现在 bin 中；
- block 位于 `heap_bin_for(block.size)` 指定的 size class；
- `free_prev_offset/free_next_offset` 双向链接互相一致；
- 所有 offset 指向真实 block 起点且在 heap 内；
- 链表无环；损坏输入也必须有界终止，不能越界访问或无限循环。

`heap_check` 接收的是独立 snapshot。它不能调用 allocator、依赖 allocator 私有全局变量、修改 snapshot，或因为“这个状态正常实现不会产生”而跳过检查。

### 2.4 搜索预算

`allocator_probe_count()` 表示 `find_fit` 自上次 reset 后实际检查过的 free-list node 数。每检查一个候选 node 必须计一次。隐藏测试会在固定 workload 上设置确定性上限；禁止伪造或绕过计数。

## 3. 可修改与不可修改

只允许修改：

- `allocator.c`
- `heap_checker.c`
- `candidate.trace`
- `decision.md`

不得：

- 修改头文件、runner、公开测试或 CI 来改变合同；
- 使用系统分配器、扩大 heap、把 allocation 放到 heap 外；
- 让 checker 恒定返回成功，或让 probe counter 不反映真实搜索；
- 针对已知 trace、固定行号或某几个 size 硬编码；
- 提交生成物、Agent 日志、隐藏判题输出或公开解答。

## 4. Candidate trace

`candidate.trace` 最多 40 个非注释操作。合法指令已列在模板中。它必须：

- 在你的实现上通过；
- 包含具体可解释的反例，而不是无目的堆砌操作；
- 尽可能让隐藏的错误实现失败。

隐藏 judge 会把同一条 trace 分别运行在多种单故障 mutation 上，只返回击杀数量，不返回 mutation 名称。Phase A 至少击杀 3 个，Final 至少击杀 5 个；参考解不是满分要求，但更高击杀数会体现在评分中。

## 5. Agent 使用规则

- 允许使用 Codex、Claude Code 等 Coding Agent，同时最多两个；
- 可以异步做代码审查、模型推导、测试设计或实现，但要在 Draft PR 描述中写清任务拆分；
- 不要求上交完整对话；需要指出至少一处你**没有直接接受**的 Agent 建议，以及如何验证；
- 候选人本人对最终 diff、测试和解释负责。

## 6. 判题协议

### Public

`./verify-public.sh` 给出具体测试名和诊断。Starter 本来就应为绿色。

### Phase A（25:00）

面试官对冻结 SHA 运行一次隐藏判题。返回：

- PASS/FAIL；
- 最多两个粗粒度失败维度，例如 `allocator-state`、`checker-soundness`、`candidate-evidence`；
- candidate trace 的 operation count、mutation 击杀数与门槛。

不会返回隐藏 case、触发 size、mutation 名称或正确 patch。

### Final（43:00）

除 Phase A 项目外，再运行 sanitizer 随机压力与 search-budget workload。最终机器门槛：

- 所有 public/private runtime checks 通过；
- checker soundness 通过；
- trace 在本人实现通过且至少击杀 5 个 mutation；
- stress 与 search budget 通过；
- `decision.md` 已实质填写。

机器 PASS 不是面试自动通过；47–60 分钟仍会做人工代码与架构 Review。

## 7. 评分（100）

机器 60 分：

- API、隐藏状态转换与 failure atomicity：20；
- checker soundness：15；
- candidate trace 与 mutation score：15；
- sanitizer stress 与搜索预算：10。

人工 40 分：

- authoritative/derived state 建模与 invariant：10；
- Agent 拆分、diff Review 与验证证据：10；
- Phase A → Final 的纠错过程：10；
- 修复边界、复杂度、剩余风险与表达：10。
