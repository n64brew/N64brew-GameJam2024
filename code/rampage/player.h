#ifndef __RAMPAGE_PLAYER_H__
#define __RAMPAGE_PLAYER_H__

#include <t3d/t3dmath.h>
#include <libdragon.h>

#include "./math/vector2.h"
#include "./collision/dynamic_object.h"
#include "./health.h"

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
    struct dynamic_object dynamic_object;
    struct dynamic_object damage_trigger;
    rspq_block_t* render_block;
    enum PlayerType type;
    T3DMat4FP mtx;
    struct health health;
    int last_attack_state: 1;
};

void rampage_player_init(struct RampagePlayer* player, struct Vector3* start_position, int player_index, enum PlayerType type);
void rampage_player_destroy(struct RampagePlayer* player);

void rampage_player_update(struct RampagePlayer* player, float delta_time);
void rampage_player_render(struct RampagePlayer* player);

#endif