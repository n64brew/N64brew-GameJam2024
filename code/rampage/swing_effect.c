#include "swing_effect.h"

#include <memory.h>
#include <stdio.h>

void swing_effect_init(struct swing_effect* effect) {
    memset(&effect->vertices[0], 0, sizeof(effect->vertices));
    effect->current_tail_length = 0;
    effect->first_vertex = 0;

    T3DVec3 normal = {{0.0f, 1.0f, 0.0f}};

    for (int i = 0; i < MAX_TAIL_LENGTH; i += 1) {
        effect->vertices[i].normA = 
        effect->vertices[i].normB = t3d_vert_pack_normal(&normal);

        effect->vertices[i].rgbaA = 
        effect->vertices[i].rgbaB = 0xFFFFFFFF;
    }

    data_cache_hit_writeback(&effect->vertices[0], sizeof(effect->vertices));
}

void swing_effect_start(struct swing_effect* effect) {
    effect->current_tail_length = 0;
    effect->first_vertex = 0;
}

void swing_effect_update(struct swing_effect* effect, T3DVec3* a, T3DVec3* b) {
    if (!a) {
        if (effect->current_tail_length > 0) {
            effect->current_tail_length -= 1;

            effect->first_vertex += 1;

            if (effect->first_vertex == MAX_TAIL_LENGTH) {
                effect->first_vertex = 0;
            }
        }

        return;
    }

    uint32_t next = (effect->first_vertex + effect->current_tail_length) & (MAX_TAIL_LENGTH - 1);
    T3DVertPacked* vertex = &effect->vertices[next];

    vertex->posA[0] = (int16_t)a->v[0];
    vertex->posA[1] = (int16_t)a->v[1];
    vertex->posA[2] = (int16_t)a->v[2];

    vertex->posB[0] = (int16_t)b->v[0];
    vertex->posB[1] = (int16_t)b->v[1];
    vertex->posB[2] = (int16_t)b->v[2];

    if (effect->current_tail_length < MAX_TAIL_LENGTH) {
        effect->current_tail_length += 1;
    } else {
        effect->first_vertex += 1;

        if (effect->first_vertex == MAX_TAIL_LENGTH) {
            effect->first_vertex = 0;
        }
    }
}

void swing_effect_end(struct swing_effect* effect) {

}

void swing_effect_render(struct swing_effect* effect) {
    if (effect->current_tail_length == 0) {
        return;
    }

    t3d_vert_load(&effect->vertices[0], 0, MAX_TAIL_VERTEX);

    int curr_index = effect->first_vertex * 2;
    int next_index = (curr_index + 2) & (MAX_TAIL_VERTEX - 1);

    for (int i = 0 ; i + 1 < effect->current_tail_length; i += 1) {
        t3d_tri_draw(curr_index, curr_index + 1, next_index);
        t3d_tri_draw(curr_index + 1, next_index + 1, next_index);
    }

    t3d_tri_sync();
}