#ifndef INTERVIEW_ALLOCATOR_H
#define INTERVIEW_ALLOCATOR_H

#include <stddef.h>

int allocator_init(void *heap, size_t heap_size);
void *allocator_malloc(size_t size);
void allocator_free(void *ptr);
void *allocator_realloc(void *ptr, size_t size);
void *allocator_calloc(size_t count, size_t size);
int allocator_check(void);

#endif
