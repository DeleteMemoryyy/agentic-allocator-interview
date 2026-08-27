# Heap Guardian：一次“公开测试全绿”的 Allocator 事故

你接手了一个运行在固定 64 KiB heap 上的 allocator。公开回归测试全部通过，但线上在一类 block resize 后偶发 heap corruption。代码曾由 Agent 优化过，没有可靠的变更说明。

你的任务不是重写一个教科书 malloc，而是：定位错误假设、修复实现、构造能够证明修复有效的反例，并对 Agent 产出负责。

## 时间与 Agent

- 总时间 60 分钟。
- 第 25 分钟冻结并提交 Phase A；收到私测的失败类别后可以继续修正。
- 第 43 分钟提交 Final；最后 13 分钟人工 Review。
- 最多同时运行两个异步 Coding Agent。可以采纳、重写或拒绝其建议，但你必须能解释最终 diff。

## 只能提交

1. `allocator.c`
2. `candidate.trace`：必须在你的实现上通过，并让原始错误实现失败。
3. `decision.md`：不超过 300 字，记录 invariant、Agent 建议取舍、修正过程和剩余风险。

不要修改 `allocator.h`、公开 runner 或 trace 语义；不要调用系统 `malloc`；不要按 trace 文件名或具体操作序列硬编码。

## API 合同

- 返回的 payload 必须 16-byte aligned，位于给定 heap 内，存活 payload 不得重叠。
- heap 必须始终由连续 block 完整覆盖；block size 是 16 的倍数且不小于 32 bytes；不得留下无法表示的碎片，也不得存在相邻 free block。
- `malloc(0)` 返回 `NULL`；OOM/size overflow 返回 `NULL`，失败不能破坏现有分配。
- `free(NULL)` 无操作。
- `realloc(NULL,n)` 等价于 `malloc(n)`；`realloc(p,0)` 释放并返回 `NULL`；成功后保留 `min(old_payload,new_size)` 字节。
- `calloc` 检查乘法溢出，并将成功分配的 payload 清零。
- `allocator_check()` 返回 1 当且仅当上述结构 invariant 成立。

## Trace 格式

```text
alloc ID SIZE
calloc ID COUNT SIZE
realloc ID NEW_SIZE
free ID
check
expect_null_alloc SIZE
expect_null_calloc COUNT SIZE
```

ID 范围为 0–255；`#` 开头为注释。

## 本地验证

```bash
./verify-public.sh
```

公开测试只证明基本行为，不代表完成。Phase A/Final 由面试官上传到私有 verifier；私测只返回失败类别，例如 `heap-invariant`、`realloc-preservation`、`overflow-or-oom`，不会返回隐藏 trace。
