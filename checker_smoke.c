#include "heap_model.h"

#include <stdio.h>
#include <string.h>

static void reset(heap_snapshot_t *snapshot) {
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->heap_size = 128;
    for (unsigned bin = 0; bin < ALLOCATOR_BIN_COUNT; ++bin) {
        snapshot->bin_heads[bin] = ALLOCATOR_NIL;
    }
}

int main(void) {
    heap_snapshot_t snapshot;
    reset(&snapshot);
    snapshot.block_count = 2;
    snapshot.blocks[0] = (heap_block_view_t){
        .offset = 0, .size = 64, .prev_size = 0, .allocated = 1,
        .free_prev_offset = ALLOCATOR_NIL, .free_next_offset = ALLOCATOR_NIL,
    };
    snapshot.blocks[1] = (heap_block_view_t){
        .offset = 64, .size = 64, .prev_size = 64, .allocated = 0,
        .free_prev_offset = ALLOCATOR_NIL, .free_next_offset = ALLOCATOR_NIL,
    };
    snapshot.bin_heads[heap_bin_for(64)] = 64;
    if (heap_check(&snapshot) != HEAP_OK) return 1;

    snapshot.blocks[1].prev_size = 32;
    if (heap_check(&snapshot) == HEAP_OK) return 1;
    puts("CHECKER SMOKE PASS");
    return 0;
}
