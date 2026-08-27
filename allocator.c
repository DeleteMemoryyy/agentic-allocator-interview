/*
 * Agent-era Malloc Lab starter.
 *
 * This is a deliberately small implicit-list allocator over a fixed heap.
 * It passes the public suite but contains at least one production bug.
 * Candidates may replace any implementation detail but must keep allocator.h.
 */
#include "allocator.h"

#include <stdint.h>
#include <string.h>

#define ALIGNMENT 16u
#define HEADER_BYTES 16u
#define FOOTER_BYTES 8u
#define OVERHEAD (HEADER_BYTES + FOOTER_BYTES)
#define MIN_BLOCK 32u
#define ALLOCATED_FLAG ((size_t)1u)
#define COOKIE UINT64_C(0x9e3779b97f4a7c15)

static unsigned char *g_heap;
static size_t g_heap_size;

static size_t align_up(size_t value) {
    return (value + (ALIGNMENT - 1u)) & ~(ALIGNMENT - 1u);
}

static size_t read_tag(const unsigned char *block) {
    return *(const size_t *)block;
}

static size_t block_size(const unsigned char *block) {
    return read_tag(block) & ~(ALIGNMENT - 1u);
}

static int block_allocated(const unsigned char *block) {
    return (read_tag(block) & ALLOCATED_FLAG) != 0;
}

static void write_block(unsigned char *block, size_t size, int allocated) {
    size_t tag = size | (allocated ? ALLOCATED_FLAG : 0u);
    *(size_t *)block = tag;
    *(uint64_t *)(block + sizeof(size_t)) = COOKIE ^ (uint64_t)tag;
    *(size_t *)(block + size - FOOTER_BYTES) = tag;
}

static size_t request_size(size_t payload) {
    if (payload > SIZE_MAX - OVERHEAD - (ALIGNMENT - 1u)) {
        return 0;
    }
    size_t total = align_up(payload + OVERHEAD);
    return total < MIN_BLOCK ? MIN_BLOCK : total;
}

static unsigned char *next_block(unsigned char *block) {
    unsigned char *next = block + block_size(block);
    return next < g_heap + g_heap_size ? next : NULL;
}

static unsigned char *previous_block(unsigned char *block) {
    if (block == g_heap) {
        return NULL;
    }
    size_t previous_tag = *(size_t *)(block - FOOTER_BYTES);
    size_t previous_size = previous_tag & ~(ALIGNMENT - 1u);
    if (previous_size < MIN_BLOCK || previous_size > (size_t)(block - g_heap)) {
        return NULL;
    }
    return block - previous_size;
}

static unsigned char *coalesce(unsigned char *block) {
    size_t total = block_size(block);
    unsigned char *previous = previous_block(block);
    if (previous && !block_allocated(previous)) {
        total += block_size(previous);
        block = previous;
    }

    unsigned char *next = block + total;
    if (next < g_heap + g_heap_size && !block_allocated(next)) {
        total += block_size(next);
    }
    write_block(block, total, 0);
    return block;
}

int allocator_init(void *heap, size_t heap_size) {
    if (!heap || ((uintptr_t)heap % ALIGNMENT) != 0) {
        return -1;
    }
    heap_size &= ~(ALIGNMENT - 1u);
    if (heap_size < MIN_BLOCK) {
        return -1;
    }
    g_heap = (unsigned char *)heap;
    g_heap_size = heap_size;
    memset(g_heap, 0, g_heap_size);
    write_block(g_heap, g_heap_size, 0);
    return 0;
}

void *allocator_malloc(size_t size) {
    if (!g_heap || size == 0) {
        return NULL;
    }
    size_t needed = request_size(size);
    if (needed == 0 || needed > g_heap_size) {
        return NULL;
    }

    for (unsigned char *block = g_heap; block < g_heap + g_heap_size;
         block += block_size(block)) {
        size_t current = block_size(block);
        if (!block_allocated(block) && current >= needed) {
            size_t remainder = current - needed;
            if (remainder >= MIN_BLOCK) {
                write_block(block, needed, 1);
                write_block(block + needed, remainder, 0);
            } else {
                write_block(block, current, 1);
            }
            return block + HEADER_BYTES;
        }
    }
    return NULL;
}

void allocator_free(void *ptr) {
    if (!ptr) {
        return;
    }
    unsigned char *block = (unsigned char *)ptr - HEADER_BYTES;
    if (!g_heap || block < g_heap || block >= g_heap + g_heap_size) {
        return;
    }
    write_block(block, block_size(block), 0);
    coalesce(block);
}

void *allocator_realloc(void *ptr, size_t size) {
    if (!ptr) {
        return allocator_malloc(size);
    }
    if (size == 0) {
        allocator_free(ptr);
        return NULL;
    }

    unsigned char *block = (unsigned char *)ptr - HEADER_BYTES;
    size_t old_total = block_size(block);
    size_t old_capacity = old_total - OVERHEAD;
    size_t needed = request_size(size);
    if (needed == 0) {
        return NULL;
    }

    if (needed <= old_total) {
        size_t remainder = old_total - needed;
        write_block(block, needed, 1);
        if (remainder >= MIN_BLOCK) {
            unsigned char *tail = block + needed;
            write_block(tail, remainder, 0);
            coalesce(tail);
        }
        return ptr;
    }

    unsigned char *next = next_block(block);
    if (next && !block_allocated(next)) {
        size_t combined = old_total + block_size(next);
        if (combined >= needed) {
            size_t remainder = combined - needed;
            if (remainder >= MIN_BLOCK) {
                write_block(block, needed, 1);
                write_block(block + needed, remainder, 0);
            } else {
                write_block(block, combined, 1);
            }
            return ptr;
        }
    }

    void *replacement = allocator_malloc(size);
    if (!replacement) {
        return NULL;
    }
    size_t copy_size = old_capacity < size ? old_capacity : size;
    memcpy(replacement, ptr, copy_size);
    allocator_free(ptr);
    return replacement;
}

void *allocator_calloc(size_t count, size_t size) {
    if (size != 0 && count > SIZE_MAX / size) {
        return NULL;
    }
    size_t bytes = count * size;
    void *ptr = allocator_malloc(bytes);
    if (ptr) {
        memset(ptr, 0, bytes);
    }
    return ptr;
}

int allocator_check(void) {
    if (!g_heap || ((uintptr_t)g_heap % ALIGNMENT) != 0) {
        return 0;
    }

    unsigned char *cursor = g_heap;
    int previous_was_free = 0;
    size_t blocks = 0;
    while (cursor < g_heap + g_heap_size) {
        if (((uintptr_t)cursor % ALIGNMENT) != 0) {
            return 0;
        }
        size_t tag = read_tag(cursor);
        size_t size = tag & ~(ALIGNMENT - 1u);
        if (size < MIN_BLOCK || (size % ALIGNMENT) != 0 ||
            size > (size_t)(g_heap + g_heap_size - cursor)) {
            return 0;
        }
        if (*(uint64_t *)(cursor + sizeof(size_t)) != (COOKIE ^ (uint64_t)tag)) {
            return 0;
        }
        if (*(size_t *)(cursor + size - FOOTER_BYTES) != tag) {
            return 0;
        }
        int free_now = (tag & ALLOCATED_FLAG) == 0;
        if (free_now && previous_was_free) {
            return 0;
        }
        previous_was_free = free_now;
        cursor += size;
        if (++blocks > g_heap_size / MIN_BLOCK + 1u) {
            return 0;
        }
    }
    return cursor == g_heap + g_heap_size;
}
