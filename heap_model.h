#ifndef HEAP_MODEL_H
#define HEAP_MODEL_H

#include <stddef.h>
#include <stdint.h>

#define ALLOCATOR_ALIGNMENT 16u
#define ALLOCATOR_MIN_BLOCK 32u
#define ALLOCATOR_BIN_COUNT 12u
#define ALLOCATOR_MAX_BLOCKS 8192u
#define ALLOCATOR_NIL SIZE_MAX

typedef struct {
    size_t offset;
    size_t size;
    size_t prev_size;
    size_t free_prev_offset;
    size_t free_next_offset;
    uint8_t allocated;
} heap_block_view_t;

typedef struct {
    size_t heap_size;
    size_t block_count;
    size_t bin_heads[ALLOCATOR_BIN_COUNT];
    heap_block_view_t blocks[ALLOCATOR_MAX_BLOCKS];
} heap_snapshot_t;

typedef enum {
    HEAP_OK = 0,
    HEAP_ERR_SNAPSHOT,
    HEAP_ERR_COVERAGE,
    HEAP_ERR_ALIGNMENT,
    HEAP_ERR_SIZE,
    HEAP_ERR_PREV_SIZE,
    HEAP_ERR_ADJACENT_FREE,
    HEAP_ERR_BIN_POINTER,
    HEAP_ERR_BIN_LINK,
    HEAP_ERR_BIN_CLASS,
    HEAP_ERR_BIN_CYCLE,
    HEAP_ERR_FREE_INDEX_BIJECTION
} heap_error_t;

static inline unsigned heap_bin_for(size_t size) {
    size_t ceiling = ALLOCATOR_MIN_BLOCK;
    for (unsigned bin = 0; bin + 1u < ALLOCATOR_BIN_COUNT; ++bin) {
        if (size <= ceiling) return bin;
        ceiling <<= 1u;
    }
    return ALLOCATOR_BIN_COUNT - 1u;
}

heap_error_t heap_check(const heap_snapshot_t *snapshot);
const char *heap_error_name(heap_error_t error);

#endif
