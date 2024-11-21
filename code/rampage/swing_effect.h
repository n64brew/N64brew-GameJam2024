#ifndef __UTIL_SWING_EFFECT_H__
#define __UTIL_SWING_EFFECT_H__

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <stdint.h>

#define MAX_TAIL_LENGTH 8
#define MAX_TAIL_VERTEX (MAX_TAIL_LENGTH * 2)

struct swing_effect {
    T3DVertPacked vertices[MAX_TAIL_LENGTH];
    uint16_t current_tail_length;
    uint16_t first_vertex;
} __attribute__((aligned(8)));

void swing_effect_init(struct swing_effect* effect);
void swing_effect_start(struct swing_effect* effect);
void swing_effect_update(struct swing_effect* effect, T3DVec3* a, T3DVec3* b);
void swing_effect_end(struct swing_effect* effect);

void swing_effect_render(struct swing_effect* effect);

#endif