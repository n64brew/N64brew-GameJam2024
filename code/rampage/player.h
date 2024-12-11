#ifndef __RAMPAGE_PLAYER_H__
#define __RAMPAGE_PLAYER_H__

#include <t3d/t3danim.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dskeleton.h>
#include <libdragon.h>

#include "./math/vector2.h"
#include "./collision/dynamic_object.h"
#include "./health.h"
#include "./swing_effect.h"
#include "./redraw_manager.h"

#define MAX_HIT_COUNT   8

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
    struct dynamic_object_type damage_shape;
    struct dynamic_object damage_trigger;
    rspq_block_t* render_block;
    T3DSkeleton skeleton;
    T3DAnim animWalk;
    T3DAnim animIdle;
    T3DAnim animStun;
    T3DAnim animAttack;
    T3DAnim animWin;
    T3DAnim animLose;
    enum PlayerType type;
    T3DMat4FP mtx;
    T3DMat4FP pointer_mtx;
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
    uint32_t did_win: 1;
    uint32_t did_lose: 1;
    uint32_t tail_tip_index: 5;
    uint32_t next_shape_offset: 1;

    uint16_t score;
    uint16_t score_dirty;

    struct Vector3 current_target;
    struct swing_effect swing_effect;

    uint8_t already_hit_ids[MAX_HIT_COUNT];
    float attack_timer;
    float attack_delay;

    RedrawHandle redraw_handle;
};

void rampage_player_init(struct RampagePlayer* player, struct Vector3* start_position, struct Vector2* start_rotation, int player_index, enum PlayerType type);
void rampage_player_destroy(struct RampagePlayer* player);

void rampage_player_update(struct RampagePlayer* player, float delta_time);
void rampage_player_render(struct RampagePlayer* player);

void rampage_player_set_did_win(struct RampagePlayer* player, bool did_win);

void rampage_player_redraw_rect(T3DViewport* viewport, struct RampagePlayer* player);

#endif