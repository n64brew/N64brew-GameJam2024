#include "./frame_malloc.h"

#include <stddef.h>

void frame_malloc_init(struct frame_malloc* fm) {
    fm->current_block = 0;
}

void* frame_malloc(struct frame_malloc* fm, int bytes) {
    int blocks = (bytes + 7) >> 3;

    if (blocks + fm->current_block > FRAME_MALLOC_BLOCKS) {
        return NULL;
    }

    void* result = &fm->blocks[fm->current_block];

    fm->current_block += blocks;

    return result;
}