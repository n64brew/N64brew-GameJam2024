#include "player.h"

#include "./assets.h"
#include "./math/vector2.h"
#include "../../core.h"
#include "./collision/collision_scene.h"
#include "./collision/capsule.h"
#include "./rampage.h"
#include "./util/entity_id.h"

#include <stdint.h>

#define PLAYER_MOVE_SPEED   128

struct dynamic_object_type player_collider = {
    .minkowsi_sum = capsule_minkowski_sum,
    .bounding_box = capsule_bounding_box,
    .data = {
        .capsule = {
            .radius = SCALE_FIXED_POINT(0.5f),
            .inner_half_height = SCALE_FIXED_POINT(0.5f),
        },
    },
    .bounce = 0.0f,
    .friction = 0.5f,
};

struct RampageInput {
    struct Vector2 direction;
    uint32_t start_attack:1;
};

float clamp_joy_input(int8_t input) {
    if (input < -80) {
        return -1.0f;
    } else if (input > 80) {
        return 1.0f;
    } else {
        return input * (1.0f / 80.0f);
    }
}

struct RampageInput rampage_player_get_input(struct RampagePlayer* player) {
    if (player->type <= PLAYER_TYPE_3) {
        struct RampageInput result = {};

        joypad_port_t port = core_get_playercontroller(player->type);

        joypad_buttons_t pressed = joypad_get_buttons_pressed(port);
        joypad_inputs_t inputs = joypad_get_inputs(port);

        result.direction.x = clamp_joy_input(inputs.stick_x);
        result.direction.y = -clamp_joy_input(inputs.stick_y);
        result.start_attack = pressed.b;

        float mag = vector2MagSqr(&result.direction);
        if (mag > 1.0f) {
            vector2Scale(&result.direction, 1.0f / sqrtf(mag), &result.direction);
        }

        return result;
    }

    struct RampageInput result = {};

    result.direction.x = player->dynamic_object.position.z;
    result.direction.y = -player->dynamic_object.position.x;

    vector2Normalize(&result.direction, &result.direction);

    return result;
} 

void rampage_player_init(struct RampagePlayer* player, struct Vector3* start_position, enum PlayerType type) {
    dynamic_object_init(
        entity_id_next(),
        &player->dynamic_object,
        &player_collider,
        COLLISION_LAYER_TANGIBLE,
        start_position,
        &gRight2
    );

    player->dynamic_object.center.y = SCALE_FIXED_POINT(1.0f);

    collision_scene_add(&player->dynamic_object);

    player->type = type;

    rspq_block_begin();
        t3d_matrix_push(&player->mtx);
        t3d_model_draw(rampage_assets_get()->player);
        t3d_matrix_pop(1);
    player->render_block = rspq_block_end();
}

void rampage_player_destroy(struct RampagePlayer* player) {
    rspq_block_free(player->render_block);
    collision_scene_remove(&player->dynamic_object);
}

void rampage_player_update(struct RampagePlayer* player, float delta_time) {
    struct RampageInput input = rampage_player_get_input(player);

    player->dynamic_object.velocity.x = input.direction.x * PLAYER_MOVE_SPEED;
    player->dynamic_object.velocity.z = input.direction.y * PLAYER_MOVE_SPEED;

    if (player->dynamic_object.position.y < 0.0f) {
        player->dynamic_object.position.y = 0.0f;

        if (player->dynamic_object.velocity.y < 0.0f) {
            player->dynamic_object.velocity.y = 0.0f;
        }
    }
}

void rampage_player_render(struct RampagePlayer* player) {
    T3DQuat rotation;
    t3d_quat_identity(&rotation);
    T3DVec3 scale = {{1.0f, 1.0f, 1.0f}};

    t3d_mat4fp_from_srt(UncachedAddr(&player->mtx), scale.v, rotation.v, (float*)&player->dynamic_object.position);
    rspq_block_run(player->render_block);
}