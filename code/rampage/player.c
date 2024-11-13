#include "player.h"

#include "./assets.h"
#include "./math/mathf.h"
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
#define PLAYER_ACCEL        5000
#define PLAYER_AIR_ACCEL    2000
#define PLAYER_ATTACK_DAMAGE_DELAY  0.5f

#define PLAYER_JUMP_IMPULSE 250
#define PLAYER_JUMP_FLOAT   400

#define SLAM_SPEED          -400

#define FIRST_PLAYER_COLLIDER_GROUP 2

#define PLAYER_STUN_TIME        3.0f
#define PLAYER_STUN_IMPULSE     200
#define PLAYER_ATTACK_VELOCITY  100

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
    .friction = 0.0f,
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
    uint32_t do_jump:1;
    uint32_t do_slam:1;
};

struct Vector2 max_rotate;
struct Vector2 input_rotation = {
    4.0f / 5.0f,
    -3.0f / 5.0f,
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

bool rampage_player_is_touching_target(struct RampagePlayer* player, struct Vector2* input_dir) {
    struct contact* contact = player->dynamic_object.active_contacts;

    for (
        struct contact* contact = player->dynamic_object.active_contacts;
        contact; 
        contact = contact->next
    ) {
        if (!contact->other_object) {
            continue;
        }

        enum HealthStatus status = health_status(contact->other_object);

        if (status == HEALTH_STATUS_ALIVE) {
            return true;
        } else if (status == HEALTH_STATUS_DEAD) {
            input_dir->x = contact->normal.z;
            input_dir->y = -contact->normal.x;
            player->moving_to_target = 0;
            vector2Normalize(input_dir, input_dir);
        }
    }

    return false;
}

struct RampageInput rampage_player_get_input(struct RampagePlayer* player, float delta_time) {
    if (!player->is_active) {
        return (struct RampageInput){};
    }

    if (player->type <= PLAYER_TYPE_3) {
        struct RampageInput result = {};

        joypad_port_t port = core_get_playercontroller(player->type);

        joypad_inputs_t inputs = joypad_get_inputs(port);

        result.direction.x = clamp_joy_input(inputs.stick_x);
        result.direction.y = -clamp_joy_input(inputs.stick_y);

        vector2ComplexMul(&result.direction, &input_rotation, &result.direction);

        result.do_attack = inputs.btn.b;
        result.do_jump = inputs.btn.a;
        result.do_slam = inputs.btn.z;

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

        if (rampage_player_is_touching_target(player, &result.direction)) {
            if (!player->is_attacking) {
                result.do_attack = true;
            }
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

void rampage_player_damage(void* data, int amount, struct Vector3* velocity, int source_id) {
    struct RampagePlayer* player = (struct RampagePlayer*)data;

    if (player->stun_timer <= 0.0f) {
        player->stun_timer = PLAYER_STUN_TIME;
        player->is_attacking = 0;
        player->dynamic_object.velocity = (struct Vector3){velocity->x, PLAYER_STUN_IMPULSE, velocity->z};
    }
}

void rampage_player_init(struct RampagePlayer* player, struct Vector3* start_position, struct Vector2* start_rotation, int player_index, enum PlayerType type) {
    int entity_id = entity_id_next();
    dynamic_object_init(
        entity_id,
        &player->dynamic_object,
        &player_collider,
        COLLISION_LAYER_TANGIBLE,
        start_position,
        start_rotation
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
    player->player_index = player_index;
    player->score = 0;

    T3DModel* model = rampage_assets_get()->player;

    player->skeleton = t3d_skeleton_create(model);
    player->animWalk = t3d_anim_create(model, "Walk");
    t3d_anim_attach(&player->animWalk, &player->skeleton);
    player->animAttack = t3d_anim_create(model, "Left_Swipe");
    t3d_anim_attach(&player->animAttack, &player->skeleton);
    t3d_anim_set_looping(&player->animAttack, false);
    t3d_anim_set_playing(&player->animAttack, false);

    collision_scene_add(&player->damage_trigger);

    player->type = type;

    rspq_block_begin();
        t3d_matrix_push(&player->mtx);
        t3d_model_draw_skinned(model, &player->skeleton);
        t3d_matrix_pop(1);
    player->render_block = rspq_block_end();

    health_register(entity_id, &player->health, rampage_player_damage, player);

    player->last_attack_state = 0;
    player->moving_to_target = 0;
    player->attacking_target = 0;
    player->current_target = gZeroVec;
    player->attack_timer = 0.0f;
    player->is_jumping = 0;
    player->was_jumping = 1;
    player->is_slamming = 0;
    player->stun_timer = 0.0f;
    player->is_attacking = 0;
    player->is_active = 0;

    vector2ComplexFromAngle(4.14f / 30.0f, &max_rotate);
}

void rampage_player_destroy(struct RampagePlayer* player) {
    rspq_block_free(player->render_block);
    t3d_anim_destroy(&player->animWalk);
    t3d_anim_destroy(&player->animAttack);
    t3d_skeleton_destroy(&player->skeleton);
    collision_scene_remove(&player->dynamic_object);
    collision_scene_remove(&player->damage_trigger);
    health_unregister(player->dynamic_object.entity_id);
}

bool rampage_player_is_grounded(struct RampagePlayer* player) {
    struct contact* curr = player->dynamic_object.active_contacts;

    while (curr) {
        if (curr->normal.y > 0.5f) {
            return true;
        }

        curr = curr->next;
    }

    return false;
}

void rampage_player_handle_ground_movement(struct RampagePlayer* player, struct RampageInput* input, float delta_time) {
    struct Vector2 target_vel;
    vector2Scale(&input->direction, PLAYER_MOVE_SPEED, &target_vel);

    struct Vector2 current_forward = {
        player->dynamic_object.rotation.y,
        player->dynamic_object.rotation.x
    };

    struct Vector2 target_dir;

    if (vector2MagSqr(&target_vel) > 0.01f) {
        vector2Normalize(&target_vel, &target_dir);
        vector2RotateTowards(&current_forward, &target_dir, &max_rotate, &current_forward);
        player->dynamic_object.rotation.x = current_forward.y;
        player->dynamic_object.rotation.y = current_forward.x;
    }

    float vel_scale = vector2Dot(&current_forward, &target_vel);

    if (vel_scale < 0.0f) {
        vel_scale = 0.0f;
    }

    float accel = PLAYER_ACCEL * delta_time * vel_scale;

    player->dynamic_object.velocity.x = mathfMoveTowards(
        player->dynamic_object.velocity.x,
        current_forward.x * vel_scale,
        PLAYER_ACCEL * delta_time
    );

    player->dynamic_object.velocity.z = mathfMoveTowards(
        player->dynamic_object.velocity.z,
        current_forward.y * vel_scale,
        PLAYER_ACCEL * delta_time
    );

    if (player->is_slamming) {
        health_contact_damage(player->dynamic_object.active_contacts, 3, &gZeroVec, player->dynamic_object.entity_id);
        player->is_slamming = false;
    }

    // if (input->do_jump && !player->was_jumping) {
    //     player->dynamic_object.velocity.y = PLAYER_JUMP_IMPULSE;
    //     player->is_jumping = 1;
    // }
}

void rampage_player_handle_air(struct RampagePlayer* player, struct RampageInput* input, float delta_time) {
    if (player->is_jumping && input->do_jump) {
        player->dynamic_object.velocity.y += PLAYER_JUMP_FLOAT * delta_time;
    } else {
        player->is_jumping = false;
    }

    if (input->do_slam) {
        player->is_slamming = true;
    }

    player->dynamic_object.velocity.x = mathfMoveTowards(
        player->dynamic_object.velocity.x,
        input->direction.x * PLAYER_MOVE_SPEED,
        PLAYER_AIR_ACCEL * delta_time
    );

    player->dynamic_object.velocity.z = mathfMoveTowards(
        player->dynamic_object.velocity.z,
        input->direction.y * PLAYER_MOVE_SPEED,
        PLAYER_AIR_ACCEL * delta_time
    );
}

void rampage_player_handle_slam(struct RampagePlayer* player, struct RampageInput* input, float delta_time) {
    player->dynamic_object.velocity.x = 0.0f;
    player->dynamic_object.velocity.y = SLAM_SPEED;
    player->dynamic_object.velocity.z = 0.0f;
}

void rampage_player_update(struct RampagePlayer* player, float delta_time) {
    struct RampageInput input = rampage_player_get_input(player, delta_time);

    bool is_grounded = rampage_player_is_grounded(player);
    
    rampage_player_update_damager(player);

    if (player->stun_timer > 0.0f) {
        player->stun_timer -= delta_time;

        if (is_grounded && player->dynamic_object.velocity.y < 0.0f) {
            player->dynamic_object.velocity.x *= 0.5f;
            player->dynamic_object.velocity.z *= 0.5f;
        }
        return;
    } else if (player->is_attacking) {
        player->dynamic_object.velocity.x *= 0.5f;
        player->dynamic_object.velocity.z *= 0.5f;
    } if (is_grounded) {
        rampage_player_handle_ground_movement(player, &input, delta_time);
    } else if (player->is_slamming) {
        rampage_player_handle_slam(player, &input, delta_time);
    } else {
        rampage_player_handle_air(player, &input, delta_time);
    }

    if (input.do_attack && !player->last_attack_state && !player->is_attacking) {
        t3d_anim_set_playing(&player->animAttack, true);
        t3d_anim_set_time(&player->animAttack, 0.0f);
        player->is_attacking = 1;
        player->attack_timer = PLAYER_ATTACK_DAMAGE_DELAY;
    }

    if (player->is_attacking && player->attack_timer > 0.0f) {
        player->attack_timer -= delta_time;

        if (player->attack_timer <= 0.0f) {
            struct Vector3 attack_velocity = (struct Vector3){
                player->dynamic_object.rotation.y * PLAYER_ATTACK_VELOCITY, 
                0.0f, 
                player->dynamic_object.rotation.x * PLAYER_ATTACK_VELOCITY
            };
            health_contact_damage(player->damage_trigger.active_contacts, 1, &attack_velocity, player->dynamic_object.entity_id);
            player->attack_timer = 0.0f;
        }
    }

    player->last_attack_state = input.do_attack;
    player->was_jumping = input.do_jump;

    if (player->is_attacking) {
        t3d_anim_update(&player->animAttack, delta_time);

        if (!player->animAttack.isPlaying) {
            player->is_attacking = 0;
        }
    } else {
        t3d_anim_update(&player->animWalk, delta_time);
    }
}

#define PLAYER_SCALE    0.5f

static color_t player_colors[] = {
    {0x00, 0x54, 0x74, 0xFF},
    {0x74, 0x00, 0x37, 0xFF},
    {0x74, 0x3B, 0x00, 0xFF},
    {0x00, 0x0A, 0x74, 0xFF},
};

void rampage_player_render(struct RampagePlayer* player) {
    struct Quaternion quat;
    quatAxisComplex(&gUp, &player->dynamic_object.rotation, &quat);
    T3DVec3 scale = {{PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE}};

    // TODO sync point

    rdpq_set_prim_color(player_colors[player->player_index]);

    t3d_skeleton_update(&player->skeleton);
    t3d_mat4fp_from_srt(UncachedAddr(&player->mtx), scale.v, (float*)&quat, (float*)&player->dynamic_object.position);
    rspq_block_run(player->render_block);
}