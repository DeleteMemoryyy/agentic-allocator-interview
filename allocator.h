#ifndef INTERVIEW_ALLOCATOR_H
#define INTERVIEW_ALLOCATOR_H

#include "heap_model.h"

#include <stddef.h>

int allocator_init(void *heap, size_t heap_size);
void *allocator_malloc(size_t size);
void allocator_free(void *ptr);
void *allocator_realloc(void *ptr, size_t size);
void *allocator_calloc(size_t count, size_t size);

int allocator_snapshot(heap_snapshot_t *snapshot);
int allocator_check(void);

void allocator_reset_probe_count(void);
size_t allocator_probe_count(void);

#endif
