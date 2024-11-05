#ifndef __RAMPAGE_PLAYER_H__
#define __RAMPAGE_PLAYER_H__

#include <t3d/t3dmath.h>
#include <libdragon.h>

#include "./vector2.h"

enum PlayerType {
    PLAYER_TYPE_0,
    PLAYER_TYPE_1,
    PLAYER_TYPE_2,
    PLAYER_TYPE_3,
    PLAYER_TYPE_EASY,
    PLAYER_TYPE_MEDIUM,
    PLAYER_TYPE_HARD,
};

struct MallardPlayer {
    T3DVec3 position;
    struct Vector2 rotation;
    rspq_block_t* render_block;
    enum PlayerType type;
    T3DMat4FP mtx;
};

void mallard_player_init(struct MallardPlayer* player, T3DVec3* start_position, enum PlayerType type);
void mallard_player_destroy(struct MallardPlayer* player);

void mallard_player_update(struct MallardPlayer* player, float delta_time);
void mallard_player_render(struct MallardPlayer* player);

#endif