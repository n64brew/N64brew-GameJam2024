#ifndef __RAMPAGE_PLAYER_H__
#define __RAMPAGE_PLAYER_H__

#include <t3d/t3danim.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dskeleton.h>
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
    T3DSkeleton skeleton;
    T3DAnim animWalk;
    T3DAnim animAttack;
    enum PlayerType type;
    T3DMat4FP mtx;
    struct health health;
    uint32_t last_attack_state: 1;
    uint32_t moving_to_target: 1;
    uint32_t attacking_target: 1;
    uint32_t is_jumping: 1;
    uint32_t was_jumping: 1;
    uint32_t is_slamming: 1;
    uint32_t is_attacking: 1;
    uint32_t player_index: 2;
    uint32_t is_active: 1;

    uint16_t score;

    struct Vector3 current_target;
    float attack_timer;
    float stun_timer;
};

void rampage_player_init(struct RampagePlayer* player, struct Vector3* start_position, struct Vector2* start_rotation, int player_index, enum PlayerType type);
void rampage_player_destroy(struct RampagePlayer* player);

void rampage_player_update(struct RampagePlayer* player, float delta_time);
void rampage_player_render(struct RampagePlayer* player);

#endif