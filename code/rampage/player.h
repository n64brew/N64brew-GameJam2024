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

struct RampagePlayer {
    T3DVec3 position;
    struct Vector2 rotation;
    rspq_block_t* render_block;
    enum PlayerType type;
    T3DMat4FP mtx;
};

void rampage_player_init(struct RampagePlayer* player, T3DVec3* start_position, enum PlayerType type);
void rampage_player_destroy(struct RampagePlayer* player);

void rampage_player_update(struct RampagePlayer* player, float delta_time);
void rampage_player_render(struct RampagePlayer* player);

#endif