/* Candidate-owned checker. The starter is intentionally incomplete; make its
 * behaviour satisfy the full snapshot contract in TASK.md. */
#include "heap_model.h"

heap_error_t heap_check(const heap_snapshot_t *snapshot) {
    if (!snapshot || snapshot->heap_size == 0 || snapshot->block_count == 0 ||
        snapshot->block_count > ALLOCATOR_MAX_BLOCKS) {
        return HEAP_ERR_SNAPSHOT;
    }

    size_t expected_offset = 0;
    size_t previous_size = 0;
    int previous_free = 0;
    for (size_t i = 0; i < snapshot->block_count; ++i) {
        const heap_block_view_t *block = &snapshot->blocks[i];
        if (block->offset != expected_offset) return HEAP_ERR_COVERAGE;
        if ((block->offset % ALLOCATOR_ALIGNMENT) != 0) return HEAP_ERR_ALIGNMENT;
        if (block->size < ALLOCATOR_MIN_BLOCK ||
            (block->size % ALLOCATOR_ALIGNMENT) != 0) return HEAP_ERR_SIZE;
        if (block->size > snapshot->heap_size - block->offset) return HEAP_ERR_COVERAGE;
        if (block->prev_size != previous_size) return HEAP_ERR_PREV_SIZE;
        if (!block->allocated && previous_free) return HEAP_ERR_ADJACENT_FREE;
        previous_free = !block->allocated;
        previous_size = block->size;
        expected_offset += block->size;
    }
    return expected_offset == snapshot->heap_size ? HEAP_OK : HEAP_ERR_COVERAGE;
}

const char *heap_error_name(heap_error_t error) {
    static const char *const names[] = {
        "ok", "snapshot", "coverage", "alignment", "size", "prev-size",
        "adjacent-free", "bin-pointer", "bin-link", "bin-class",
        "bin-cycle", "free-index-bijection"
    };
    size_t index = (size_t)error;
    return index < sizeof(names) / sizeof(names[0]) ? names[index] : "unknown";
}
