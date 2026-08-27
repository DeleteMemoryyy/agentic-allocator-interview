#include "allocator.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_SIZE (64u * 1024u)
#define MAX_IDS 256

typedef struct {
    unsigned char *ptr;
    size_t requested;
    int live;
} slot_t;

_Alignas(16) static unsigned char heap[HEAP_SIZE];
static slot_t slots[MAX_IDS];

static unsigned char pattern(int id, size_t offset) {
    return (unsigned char)((id * 131u + offset * 17u + 0x5au) & 0xffu);
}

static int fail(const char *category, int line, const char *detail) {
    printf("RESULT FAIL category=%s line=%d detail=%s\n", category, line, detail);
    return 1;
}

static int valid_id(int id) {
    return id >= 0 && id < MAX_IDS;
}

static int check_bytes(int id) {
    for (size_t i = 0; i < slots[id].requested; ++i) {
        if (slots[id].ptr[i] != pattern(id, i)) {
            return 0;
        }
    }
    return 1;
}

static void fill_bytes(int id) {
    for (size_t i = 0; i < slots[id].requested; ++i) {
        slots[id].ptr[i] = pattern(id, i);
    }
}

static int validate_all(int line) {
    if (!allocator_check()) {
        return fail("heap-invariant", line, "allocator_check_rejected_heap");
    }
    for (int i = 0; i < MAX_IDS; ++i) {
        if (!slots[i].live) continue;
        uintptr_t lo = (uintptr_t)slots[i].ptr;
        uintptr_t hi = lo + slots[i].requested;
        if ((lo % 16u) != 0) {
            return fail("alignment", line, "payload_not_16_byte_aligned");
        }
        if (lo < (uintptr_t)heap || hi > (uintptr_t)heap + HEAP_SIZE || hi < lo) {
            return fail("heap-range", line, "payload_outside_heap");
        }
        if (!check_bytes(i)) {
            return fail("payload-corruption", line, "live_payload_changed");
        }
        for (int j = i + 1; j < MAX_IDS; ++j) {
            if (!slots[j].live) continue;
            uintptr_t other_lo = (uintptr_t)slots[j].ptr;
            uintptr_t other_hi = other_lo + slots[j].requested;
            if (lo < other_hi && other_lo < hi) {
                return fail("overlap", line, "live_payloads_overlap");
            }
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: runner TRACE_FILE\n");
        return 2;
    }
    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror("open trace");
        return 2;
    }
    memset(slots, 0, sizeof(slots));
    if (allocator_init(heap, sizeof(heap)) != 0) {
        fclose(input);
        return fail("init", 0, "allocator_init_failed");
    }

    char linebuf[512];
    int line = 0;
    while (fgets(linebuf, sizeof(linebuf), input)) {
        ++line;
        char *cursor = linebuf;
        while (*cursor == ' ' || *cursor == '\t') ++cursor;
        if (*cursor == '\0' || *cursor == '\n' || *cursor == '#') continue;

        char op[40];
        int id;
        size_t a, b;
        if (sscanf(cursor, "%39s", op) != 1) continue;

        if (strcmp(op, "alloc") == 0) {
            if (sscanf(cursor, "%*s %d %zu", &id, &a) != 2 || !valid_id(id) || slots[id].live) {
                fclose(input); return fail("trace", line, "invalid_alloc");
            }
            void *ptr = allocator_malloc(a);
            if (!ptr) { fclose(input); return fail("allocation", line, "unexpected_null"); }
            slots[id] = (slot_t){(unsigned char *)ptr, a, 1};
            fill_bytes(id);
        } else if (strcmp(op, "calloc") == 0) {
            if (sscanf(cursor, "%*s %d %zu %zu", &id, &a, &b) != 3 ||
                !valid_id(id) || slots[id].live || (b != 0 && a > SIZE_MAX / b)) {
                fclose(input); return fail("trace", line, "invalid_calloc");
            }
            size_t bytes = a * b;
            unsigned char *ptr = allocator_calloc(a, b);
            if (!ptr) { fclose(input); return fail("allocation", line, "unexpected_null"); }
            for (size_t i = 0; i < bytes; ++i) {
                if (ptr[i] != 0) { fclose(input); return fail("calloc-zero", line, "nonzero_byte"); }
            }
            slots[id] = (slot_t){ptr, bytes, 1};
            fill_bytes(id);
        } else if (strcmp(op, "realloc") == 0) {
            if (sscanf(cursor, "%*s %d %zu", &id, &a) != 2 || !valid_id(id) || !slots[id].live) {
                fclose(input); return fail("trace", line, "invalid_realloc");
            }
            if (!check_bytes(id)) { fclose(input); return fail("payload-corruption", line, "before_realloc"); }
            size_t preserved = slots[id].requested < a ? slots[id].requested : a;
            unsigned char *ptr = allocator_realloc(slots[id].ptr, a);
            if (a == 0) {
                if (ptr != NULL) { fclose(input); return fail("realloc-zero", line, "expected_null"); }
                slots[id] = (slot_t){0};
            } else {
                if (!ptr) { fclose(input); return fail("allocation", line, "realloc_unexpected_null"); }
                for (size_t i = 0; i < preserved; ++i) {
                    if (ptr[i] != pattern(id, i)) {
                        fclose(input); return fail("realloc-preservation", line, "old_payload_not_preserved");
                    }
                }
                slots[id] = (slot_t){ptr, a, 1};
                fill_bytes(id);
            }
        } else if (strcmp(op, "free") == 0) {
            if (sscanf(cursor, "%*s %d", &id) != 1 || !valid_id(id) || !slots[id].live) {
                fclose(input); return fail("trace", line, "invalid_free");
            }
            if (!check_bytes(id)) { fclose(input); return fail("payload-corruption", line, "before_free"); }
            allocator_free(slots[id].ptr);
            slots[id] = (slot_t){0};
        } else if (strcmp(op, "expect_null_alloc") == 0) {
            if (sscanf(cursor, "%*s %zu", &a) != 1 || allocator_malloc(a) != NULL) {
                fclose(input); return fail("overflow-or-oom", line, "malloc_should_return_null");
            }
        } else if (strcmp(op, "expect_null_calloc") == 0) {
            if (sscanf(cursor, "%*s %zu %zu", &a, &b) != 2 || allocator_calloc(a, b) != NULL) {
                fclose(input); return fail("calloc-overflow", line, "calloc_should_return_null");
            }
        } else if (strcmp(op, "check") != 0) {
            fclose(input); return fail("trace", line, "unknown_operation");
        }

        int status = validate_all(line);
        if (status != 0) { fclose(input); return status; }
    }
    fclose(input);
    printf("RESULT PASS trace=%s\n", argv[1]);
    return 0;
}
