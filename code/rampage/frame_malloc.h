#ifndef __FRAME_MALLOC_H__
#define __FRAME_MALLOC_H__

#include <stdint.h>

#define FRAME_MALLOC_SIZE   4096
#define FRAME_MALLOC_BLOCKS (FRAME_MALLOC_SIZE / sizeof(uint64_t))

struct frame_malloc {
    uint64_t blocks[FRAME_MALLOC_BLOCKS];
    int current_block;
};

void frame_malloc_init(struct frame_malloc* fm);
void* frame_malloc(struct frame_malloc* fm, int bytes);

#endif