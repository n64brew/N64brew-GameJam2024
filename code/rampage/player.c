#include "player.h"

#include "./assets.h"
#include "./math/vector2.h"
#include "./math/quaternion.h"
#include "../../core.h"
#include "./collision/collision_scene.h"
#include "./collision/capsule.h"
#include "./collision/cylinder.h"
#include "./rampage.h"
#include "./util/entity_id.h"
#include "./scene_query.h"

#include <stdint.h>

#define PLAYER_MOVE_SPEED   128
#define PLAYER_ATTACK_TIME  0.75f

#define FIRST_PLAYER_COLLIDER_GROUP 2

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

struct dynamic_object_type damage_trigger_collider = {
    .minkowsi_sum = cylinder_minkowski_sum,
    .bounding_box = cylinder_bounding_box,
    .data = {
        .cylinder = {
            .radius = SCALE_FIXED_POINT(0.5f),
            .half_height = SCALE_FIXED_POINT(1.0f),
        },
    }
};

#define DAMAGE_DISTANCE SCALE_FIXED_POINT(0.75f)

struct RampageInput {
    struct Vector2 direction;
    uint32_t do_attack:1;
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

int floatBits(void* input) {
    return *(int*)input;
}

bool rampage_player_is_touching_target(struct RampagePlayer* player) {
    struct contact* contact = player->dynamic_object.active_contacts;

    while (contact) {
        if (contact->other_object) {
            return true;
        }

        contact = contact->next;
    }

    return false;
}

struct RampageInput rampage_player_get_input(struct RampagePlayer* player, float delta_time) {
    if (player->type <= PLAYER_TYPE_3) {
        struct RampageInput result = {};

        joypad_port_t port = core_get_playercontroller(player->type);

        joypad_inputs_t inputs = joypad_get_inputs(port);

        result.direction.x = clamp_joy_input(inputs.stick_x);
        result.direction.y = -clamp_joy_input(inputs.stick_y);
        result.do_attack = inputs.btn.b;

        float mag = vector2MagSqr(&result.direction);
        if (mag > 1.0f) {
            vector2Scale(&result.direction, 1.0f / sqrtf(mag), &result.direction);
        }

        return result;
    }

    struct RampageInput result = {};

    if (player->moving_to_target) {
        struct Vector3 offset;

        vector3Sub(&player->current_target, &player->dynamic_object.position, &offset);
        offset.y = 0.0f;

        if (vector3MagSqrd(&offset) < SCALE_FIXED_POINT(0.2f)) {
            player->moving_to_target = 0;
        }

        vector3Normalize(&offset, &offset);

        result.direction.x = offset.x;
        result.direction.y = offset.z;

        if (rampage_player_is_touching_target(player)) {
            if (player->attack_timer <= 0.0f) {
                result.do_attack = true;
                player->attack_timer = PLAYER_ATTACK_TIME;
            } else {
                player->attack_timer -= delta_time;
            }
        } else {
            player->attack_timer = 0.0f;
        }
    } else {
        struct Vector3* new_target = find_nearest_target(&player->dynamic_object.position, 1.3f);

        if (new_target) {
            player->moving_to_target = 1;
            player->current_target = *new_target;
        }
    }

    return result;
}

void rampage_player_update_damager(struct RampagePlayer* player) {
    struct Vector3 offset = {
        player->dynamic_object.rotation.y * DAMAGE_DISTANCE,
        0.0f,
        player->dynamic_object.rotation.x * DAMAGE_DISTANCE
    };

    vector3Add(&player->dynamic_object.position, &offset, &player->damage_trigger.position);
}

void rampage_player_damage(void* data) {

}

void rampage_player_init(struct RampagePlayer* player, struct Vector3* start_position, int player_index, enum PlayerType type) {
    int entity_id = entity_id_next();
    dynamic_object_init(
        entity_id,
        &player->dynamic_object,
        &player_collider,
        COLLISION_LAYER_TANGIBLE,
        start_position,
        &gRight2
    );

    player->dynamic_object.center.y = SCALE_FIXED_POINT(1.0f);
    player->dynamic_object.collision_group = FIRST_PLAYER_COLLIDER_GROUP + player_index;

    collision_scene_add(&player->dynamic_object);

    dynamic_object_init(
        entity_id,
        &player->damage_trigger,
        &damage_trigger_collider,
        COLLISION_LAYER_TANGIBLE,
        start_position,
        &gRight2
    );

    player->damage_trigger.center.y = SCALE_FIXED_POINT(1.0f);
    player->damage_trigger.is_trigger = true;
    player->damage_trigger.collision_group = FIRST_PLAYER_COLLIDER_GROUP + player_index;

    collision_scene_add(&player->damage_trigger);

    player->type = type;

    rspq_block_begin();
        t3d_matrix_push(&player->mtx);
        t3d_model_draw(rampage_assets_get()->player);
        t3d_matrix_pop(1);
    player->render_block = rspq_block_end();

    health_register(entity_id, &player->health, rampage_player_damage, player);

    player->last_attack_state = 0;
    player->moving_to_target = 0;
    player->attacking_target = 0;
    player->current_target = gZeroVec;
    player->attack_timer = 0.0f;
}

void rampage_player_destroy(struct RampagePlayer* player) {
    rspq_block_free(player->render_block);
    collision_scene_remove(&player->dynamic_object);
    collision_scene_remove(&player->damage_trigger);
    health_unregister(player->dynamic_object.entity_id);
}

void rampage_player_update(struct RampagePlayer* player, float delta_time) {
    struct RampageInput input = rampage_player_get_input(player, delta_time);
    
    rampage_player_update_damager(player);

    player->dynamic_object.velocity.x = input.direction.x * PLAYER_MOVE_SPEED;
    player->dynamic_object.velocity.z = input.direction.y * PLAYER_MOVE_SPEED;

    if (player->dynamic_object.position.y < 0.0f) {
        player->dynamic_object.position.y = 0.0f;

        if (player->dynamic_object.velocity.y < 0.0f) {
            player->dynamic_object.velocity.y = 0.0f;
        }
    }

    struct Vector3 dir = player->dynamic_object.velocity;
    dir.y = 0.0f;

    if (vector3MagSqrd(&dir) > 0.001f) {
        vector3Normalize(&dir, &dir);
        player->dynamic_object.rotation.x = dir.z;
        player->dynamic_object.rotation.y = dir.x;
    }

    if (input.do_attack && !player->last_attack_state) {
        health_contact_damage(player->damage_trigger.active_contacts);
    }

    player->last_attack_state = input.do_attack;
}

void rampage_player_render(struct RampagePlayer* player) {
    struct Quaternion quat;
    quatAxisComplex(&gUp, &player->dynamic_object.rotation, &quat);
    T3DVec3 scale = {{1.0f, 1.0f, 1.0f}};

    t3d_mat4fp_from_srt(UncachedAddr(&player->mtx), scale.v, (float*)&quat, (float*)&player->dynamic_object.position);
    rspq_block_run(player->render_block);
}