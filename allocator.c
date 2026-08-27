/*
 * V2 starter: boundary-tagged physical blocks plus segregated free bins.
 * The physical chain is authoritative; bins are a derived search index.
 */
#include "allocator.h"

#include <stdint.h>
#include <string.h>

typedef struct {
    size_t size_flags;
    size_t prev_size;
} block_t;

#define HEADER_SIZE ((size_t)sizeof(block_t))
#define ALLOCATED_FLAG ((size_t)1u)

static unsigned char *g_heap;
static size_t g_heap_size;
static size_t g_bin_heads[ALLOCATOR_BIN_COUNT];
static size_t g_probe_count;

static size_t align_up(size_t value) {
    return (value + ALLOCATOR_ALIGNMENT - 1u) & ~(ALLOCATOR_ALIGNMENT - 1u);
}

static size_t block_size(const block_t *block) {
    return block->size_flags & ~(ALLOCATOR_ALIGNMENT - 1u);
}

static int block_allocated(const block_t *block) {
    return (block->size_flags & ALLOCATED_FLAG) != 0;
}

static size_t block_offset(const block_t *block) {
    return (size_t)((const unsigned char *)block - g_heap);
}

static block_t *block_from_offset(size_t offset) {
    if (offset == ALLOCATOR_NIL || offset >= g_heap_size ||
        (offset % ALLOCATOR_ALIGNMENT) != 0) {
        return NULL;
    }
    return (block_t *)(g_heap + offset);
}

static size_t *free_prev_slot(block_t *block) {
    return (size_t *)((unsigned char *)block + HEADER_SIZE);
}

static size_t *free_next_slot(block_t *block) {
    return (size_t *)((unsigned char *)block + HEADER_SIZE + sizeof(size_t));
}

static size_t free_prev(const block_t *block) {
    return *(const size_t *)((const unsigned char *)block + HEADER_SIZE);
}

static size_t free_next(const block_t *block) {
    return *(const size_t *)((const unsigned char *)block + HEADER_SIZE + sizeof(size_t));
}

static block_t *physical_next(block_t *block) {
    unsigned char *next = (unsigned char *)block + block_size(block);
    return next < g_heap + g_heap_size ? (block_t *)next : NULL;
}

static block_t *physical_previous(block_t *block) {
    if (block->prev_size == 0 || block->prev_size > block_offset(block)) return NULL;
    return (block_t *)((unsigned char *)block - block->prev_size);
}

static void write_block(block_t *block, size_t size, int allocated, size_t prev_size) {
    block->size_flags = size | (allocated ? ALLOCATED_FLAG : 0u);
    block->prev_size = prev_size;
    unsigned char *after = (unsigned char *)block + size;
    if (after < g_heap + g_heap_size) {
        ((block_t *)after)->prev_size = size;
    }
}

static size_t request_size(size_t payload) {
    if (payload > SIZE_MAX - HEADER_SIZE - (ALLOCATOR_ALIGNMENT - 1u)) return 0;
    size_t total = align_up(payload + HEADER_SIZE);
    return total < ALLOCATOR_MIN_BLOCK ? ALLOCATOR_MIN_BLOCK : total;
}

static void link_free(block_t *block) {
    unsigned bin = heap_bin_for(block_size(block));
    size_t head = g_bin_heads[bin];
    *free_prev_slot(block) = ALLOCATOR_NIL;
    *free_next_slot(block) = head;
    block_t *head_block = block_from_offset(head);
    if (head_block) *free_prev_slot(head_block) = block_offset(block);
    g_bin_heads[bin] = block_offset(block);
}

static void unlink_free(block_t *block) {
    unsigned bin = heap_bin_for(block_size(block));
    size_t previous = free_prev(block);
    size_t next = free_next(block);
    block_t *previous_block = block_from_offset(previous);
    block_t *next_block = block_from_offset(next);
    if (previous_block) *free_next_slot(previous_block) = next;
    else g_bin_heads[bin] = next;
    if (next_block) *free_prev_slot(next_block) = previous;
    *free_prev_slot(block) = ALLOCATOR_NIL;
    *free_next_slot(block) = ALLOCATOR_NIL;
}

/* The input block is free but not yet present in a bin. Neighbouring free
 * blocks, if any, are already indexed. The returned block is still unindexed. */
static block_t *coalesce_unlinked(block_t *block) {
    size_t total = block_size(block);
    block_t *previous = physical_previous(block);
    if (previous && !block_allocated(previous)) {
        unlink_free(previous);
        total += block_size(previous);
        block = previous;
    }

    block_t *next = (block_t *)((unsigned char *)block + total);
    if ((unsigned char *)next < g_heap + g_heap_size && !block_allocated(next)) {
        unlink_free(next);
        total += block_size(next);
    }
    write_block(block, total, 0, block->prev_size);
    return block;
}

static block_t *find_fit(size_t needed) {
    unsigned first_bin = heap_bin_for(needed);
    for (unsigned bin = first_bin; bin < ALLOCATOR_BIN_COUNT; ++bin) {
        size_t offset = g_bin_heads[bin];
        size_t steps = 0;
        while (offset != ALLOCATOR_NIL) {
            block_t *block = block_from_offset(offset);
            if (!block || ++steps > ALLOCATOR_MAX_BLOCKS) return NULL;
            ++g_probe_count;
            if (!block_allocated(block) && block_size(block) >= needed) return block;
            offset = free_next(block);
        }
    }
    return NULL;
}

static void *place(block_t *block, size_t needed) {
    size_t current = block_size(block);
    size_t previous_size = block->prev_size;
    unlink_free(block);
    size_t remainder = current - needed;
    if (remainder >= ALLOCATOR_MIN_BLOCK) {
        write_block(block, needed, 1, previous_size);
        block_t *tail = (block_t *)((unsigned char *)block + needed);
        write_block(tail, remainder, 0, needed);
        link_free(tail);
    } else {
        write_block(block, current, 1, previous_size);
    }
    return (unsigned char *)block + HEADER_SIZE;
}

int allocator_init(void *heap, size_t heap_size) {
    g_heap = NULL;
    g_heap_size = 0;
    if (!heap || ((uintptr_t)heap % ALLOCATOR_ALIGNMENT) != 0) return -1;
    heap_size &= ~(ALLOCATOR_ALIGNMENT - 1u);
    if (heap_size < ALLOCATOR_MIN_BLOCK ||
        heap_size > ALLOCATOR_MAX_BLOCKS * ALLOCATOR_MIN_BLOCK) return -1;

    g_heap = (unsigned char *)heap;
    g_heap_size = heap_size;
    g_probe_count = 0;
    for (unsigned bin = 0; bin < ALLOCATOR_BIN_COUNT; ++bin) {
        g_bin_heads[bin] = ALLOCATOR_NIL;
    }
    memset(g_heap, 0, g_heap_size);
    block_t *initial = (block_t *)g_heap;
    write_block(initial, g_heap_size, 0, 0);
    link_free(initial);
    return 0;
}

void *allocator_malloc(size_t size) {
    if (!g_heap || size == 0) return NULL;
    size_t needed = request_size(size);
    if (needed == 0 || needed > g_heap_size) return NULL;
    block_t *block = find_fit(needed);
    return block ? place(block, needed) : NULL;
}

void allocator_free(void *ptr) {
    if (!ptr) return;
    block_t *block = (block_t *)((unsigned char *)ptr - HEADER_SIZE);
    if (!g_heap || (unsigned char *)block < g_heap ||
        (unsigned char *)block >= g_heap + g_heap_size || !block_allocated(block)) return;
    write_block(block, block_size(block), 0, block->prev_size);
    block = coalesce_unlinked(block);
    link_free(block);
}

void *allocator_realloc(void *ptr, size_t size) {
    if (!ptr) return allocator_malloc(size);
    if (size == 0) {
        allocator_free(ptr);
        return NULL;
    }

    block_t *block = (block_t *)((unsigned char *)ptr - HEADER_SIZE);
    size_t old_total = block_size(block);
    size_t old_payload = old_total - HEADER_SIZE;
    size_t needed = request_size(size);
    if (needed == 0) return NULL;

    if (needed <= old_total) {
        size_t remainder = old_total - needed;
        if (remainder >= ALLOCATOR_MIN_BLOCK) {
            size_t previous_size = block->prev_size;
            write_block(block, needed, 1, previous_size);
            block_t *tail = (block_t *)((unsigned char *)block + needed);
            write_block(tail, remainder, 0, needed);
            tail = coalesce_unlinked(tail);
            link_free(tail);
        }
        return ptr;
    }

    block_t *next = physical_next(block);
    if (next && !block_allocated(next)) {
        size_t combined = old_total + block_size(next);
        unlink_free(next);
        if (combined >= needed) {
            size_t remainder = combined - needed;
            size_t previous_size = block->prev_size;
            if (remainder >= ALLOCATOR_MIN_BLOCK) {
                write_block(block, needed, 1, previous_size);
                block_t *tail = (block_t *)((unsigned char *)block + needed);
                write_block(tail, remainder, 0, needed);
                link_free(tail);
            } else {
                write_block(block, combined, 1, previous_size);
            }
            return ptr;
        }
    }

    void *replacement = allocator_malloc(size);
    if (!replacement) return NULL;
    size_t copy_size = old_payload < size ? old_payload : size;
    memcpy(replacement, ptr, copy_size);
    allocator_free(ptr);
    return replacement;
}

void *allocator_calloc(size_t count, size_t size) {
    if (size != 0 && count > SIZE_MAX / size) return NULL;
    size_t bytes = count * size;
    void *ptr = allocator_malloc(bytes);
    if (ptr) memset(ptr, 0, bytes);
    return ptr;
}

int allocator_snapshot(heap_snapshot_t *snapshot) {
    if (!snapshot || !g_heap) return -1;
    snapshot->heap_size = g_heap_size;
    for (unsigned bin = 0; bin < ALLOCATOR_BIN_COUNT; ++bin) {
        snapshot->bin_heads[bin] = g_bin_heads[bin];
    }

    size_t count = 0;
    size_t offset = 0;
    while (offset < g_heap_size && count < ALLOCATOR_MAX_BLOCKS) {
        block_t *block = (block_t *)(g_heap + offset);
        heap_block_view_t *view = &snapshot->blocks[count++];
        size_t size = block_size(block);
        view->offset = offset;
        view->size = size;
        view->prev_size = block->prev_size;
        view->allocated = (uint8_t)block_allocated(block);
        view->free_prev_offset = view->allocated ? ALLOCATOR_NIL : free_prev(block);
        view->free_next_offset = view->allocated ? ALLOCATOR_NIL : free_next(block);
        if (size < ALLOCATOR_MIN_BLOCK || (size % ALLOCATOR_ALIGNMENT) != 0 ||
            size > g_heap_size - offset) break;
        offset += size;
    }
    snapshot->block_count = count;
    return 0;
}

int allocator_check(void) {
    static heap_snapshot_t snapshot;
    return allocator_snapshot(&snapshot) == 0 && heap_check(&snapshot) == HEAP_OK;
}

void allocator_reset_probe_count(void) {
    g_probe_count = 0;
}

size_t allocator_probe_count(void) {
    return g_probe_count;
}
