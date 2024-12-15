#include "tank.h"

#include "./collision/collision_scene.h"
#include "./collision/box.h"
#include "./rampage.h"
#include "./util/entity_id.h"
#include "./math/quaternion.h"
#include "./assets.h"
#include "./math/mathf.h"
#include "./scene_query.h"

struct Vector2 tank_rotate_speed;

#define TANK_SPEED          SCALE_FIXED_POINT(0.5f)
#define TANK_SLOW_SPEED     SCALE_FIXED_POINT(0.01f)
#define TANK_ACCEL          SCALE_FIXED_POINT(0.5f)

#define TANK_DAMAGE_SPEED   SCALE_FIXED_POINT(0.8f)

#define MIN_FIRE_TIME   3.0f
#define MAX_FIRE_TIME   5.0f

struct dynamic_object_type tank_collider = {
    .minkowsi_sum = box_minkowski_sum,
    .bounding_box = box_bounding_box,
    .data = {
        .box = {
            .half_size = {
                SCALE_FIXED_POINT(1.06136f * 0.5f),
                SCALE_FIXED_POINT(0.63024f * 0.5f),
                SCALE_FIXED_POINT(1.27636f * 0.5f),
            }
        },
    },
    .bounce = 0.0f,
    .friction = 0.1f,
};

#define DIRECTION_COUNT 4

static struct Vector3 move_direction[DIRECTION_COUNT] = {
    {0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, -1.0f},
    {1.0f, 0.0f, 0.0f},
    {-1.0f, 0.0f, 0.0f},
};

static float max_bound[DIRECTION_COUNT] = {
    MAX_Z,
    -MIN_Z,
    MAX_X,
    -MIN_X,
};

bool rampage_is_valid_target(struct RampageTank* tank, int dir_index) {
    struct Vector3 forward = {
        tank->dynamic_object.rotation.y,
        0.0f,
        tank->dynamic_object.rotation.x
    };

    float dir_dot = vector3Dot(&move_direction[dir_index], &forward);

    if (dir_dot < -0.5f) {
        return false;
    }

    float distance = vector3Dot(&tank->dynamic_object.position, &move_direction[dir_index]) + BUILDING_SPACING;

    if (distance > max_bound[dir_index]) {
        return false;
    }

    return true;
}

void rampage_tank_next_target(struct RampageTank* tank) {
    if (!tank->is_active) {
        return;
    }

    int option = randomInRange(0, DIRECTION_COUNT);

    for (int i = 0; i < DIRECTION_COUNT; i += 1) {
        if (i != option || !rampage_is_valid_target(tank, i)) {
            continue;
        }

        struct Vector3 next_target;

        vector3AddScaled(
            &tank->current_target, 
            &move_direction[i], 
            BUILDING_SPACING, 
            &next_target
        );

        if (is_tank_target_used(&next_target)) {
            continue;
        }

        tank->current_target = next_target;
    }
}

struct Vector3 fire_offset = {
    0.0f,
    SCALE_FIXED_POINT(0.510332f),
    SCALE_FIXED_POINT(0.681162f),
};

void rampage_tank_fire(struct RampageTank* tank) {
    struct Vector3 fire_from = tank->dynamic_object.position;
    struct Vector2* rotation = &tank->dynamic_object.rotation;

    fire_from.x += fire_offset.x * rotation->x + fire_offset.z * rotation->y;
    fire_from.y = fire_offset.y;
    fire_from.z += fire_offset.z * rotation->x - fire_offset.x * rotation->y ;

    bullet_fire(&tank->bullet, &fire_from, rotation);
}

#define KNOCKBACK_VELOCITY  SCALE_FIXED_POINT(8.0f)

void rampage_tank_damage(void* data, int amount, struct Vector3* velocity, int source_id) {
    struct RampageTank* tank = (struct RampageTank*)data;

    if (!is_player(source_id) || source_id == tank->last_hit_by) {
        return;
    }

    tank->dynamic_object.velocity = (struct Vector3){velocity->x, 0.0f, velocity->z};
    vector3Normalize(&tank->dynamic_object.velocity, &tank->dynamic_object.velocity);
    vector3Scale(&tank->dynamic_object.velocity, &tank->dynamic_object.velocity, KNOCKBACK_VELOCITY);
    tank->last_hit_by = source_id;
    memset(tank->already_hit_ids, 0, MAX_HIT_COUNT);
}

void rampage_tank_contact_damage(struct RampageTank* tank) {
    if (vector3MagSqrd(&tank->dynamic_object.velocity) < TANK_DAMAGE_SPEED * TANK_DAMAGE_SPEED || !tank->last_hit_by) {
        tank->last_hit_by = 0;
        return;
    }

    health_contact_damage(
        tank->dynamic_object.active_contacts, 
        1, 
        &tank->dynamic_object.velocity, 
        tank->last_hit_by,
        tank->already_hit_ids,
        MAX_HIT_COUNT
    );
}

void rampage_tank_init(struct RampageTank* tank, struct Vector3* start_position) {
    int entity_id = entity_id_next();
    dynamic_object_init(
        entity_id,
        &tank->dynamic_object,
        &tank_collider,
        COLLISION_LAYER_TANGIBLE,
        start_position,
        &gRight2
    );

    if (start_position->z < 0.0f) {
        tank->dynamic_object.rotation.x = -1.0f;
    }

    tank->dynamic_object.center.y = tank_collider.data.box.half_size.y;
    tank->dynamic_object.collision_group = entity_id;
    tank->current_target = *start_position;
    tank->is_active = 0;
    tank->fire_timer = randomInRangef(MIN_FIRE_TIME, MAX_FIRE_TIME) + 4.0f;

    collision_scene_add(&tank->dynamic_object);

    health_register(entity_id, &tank->health, rampage_tank_damage, tank);

    vector2ComplexFromAngle(1.0f / 30.0f, &tank_rotate_speed);
    bullet_init(&tank->bullet, entity_id);

    tank->redraw_handle = redraw_aquire_handle();
    tank->bullet_redraw_handle = redraw_aquire_handle();
    tank->last_hit_by = 0;
}

void rampage_tank_destroy(struct RampageTank* tank) {
    collision_scene_remove(&tank->dynamic_object);
    bullet_destroy(&tank->bullet);
    health_unregister(tank->dynamic_object.entity_id);
}

bool rampage_tank_has_forward_hit(struct RampageTank* tank, struct Vector2* offset, struct Vector2* current_dir) {
    struct contact* curr = tank->dynamic_object.active_contacts;

    while (curr) {
        struct Vector2 normal = { curr->normal.x, curr->normal.z };

        if (vector2Dot(offset, &normal) < 0.1f && vector2Dot(current_dir, offset) > 0.5f) {
            return true;
        }

        curr = curr->next;
    }

    return false;
}

void rampage_tank_update(struct RampageTank* tank, float delta_time) {
    struct Vector2 offset = (struct Vector2){
        tank->current_target.x - tank->dynamic_object.position.x,
        tank->current_target.z - tank->dynamic_object.position.z,
    };

    if (tank->dynamic_object.position.x < MIN_X) {
        tank->dynamic_object.position.x = MIN_X;
    } else if (tank->dynamic_object.position.x > MAX_X) {
        tank->dynamic_object.position.x = MAX_X;
    }

    if (tank->dynamic_object.position.z < MIN_Z) {
        tank->dynamic_object.position.z = MIN_Z;
    } else if (tank->dynamic_object.position.z > MAX_Z) {
        tank->dynamic_object.position.z = MAX_Z;
    }

    struct Vector2 dir;
    vector2Normalize(&offset, &dir);

    struct Vector2 current_dir = (struct Vector2){
        tank->dynamic_object.rotation.y,
        tank->dynamic_object.rotation.x,
    };

    if (vector2MagSqr(&offset) < 5.0f) {
        rampage_tank_next_target(tank);

        tank->dynamic_object.velocity.x = mathfMoveTowards(
            tank->dynamic_object.velocity.x,
            0.0f,
            TANK_ACCEL * delta_time
        );
        tank->dynamic_object.velocity.z = mathfMoveTowards(
            tank->dynamic_object.velocity.z,
            0.0f,
            TANK_ACCEL * delta_time
        );
    } else if (vector2RotateTowards(
        &current_dir,
        &dir,
        &tank_rotate_speed,
        &current_dir
    )) {
        float distance = vector2Dot(&offset, &dir);
        float speed = dir.x * tank->dynamic_object.velocity.x +
            dir.y * tank->dynamic_object.velocity.z;

        float stopping_distance = stoppingDistance(fabsf(speed), TANK_ACCEL);

        bool should_stop = stopping_distance > distance;

        tank->dynamic_object.velocity.x = mathfMoveTowards(
            tank->dynamic_object.velocity.x,
            should_stop ? dir.x * TANK_SLOW_SPEED : dir.x * TANK_SPEED,
            TANK_ACCEL * delta_time
        );
        tank->dynamic_object.velocity.z = mathfMoveTowards(
            tank->dynamic_object.velocity.z,
            should_stop ? dir.y * TANK_SLOW_SPEED : dir.y * TANK_SPEED,
            TANK_ACCEL * delta_time
        );

        if (tank->fire_timer < 0.0f) {
            rampage_tank_fire(tank);
            tank->fire_timer = randomInRangef(MIN_FIRE_TIME, MAX_FIRE_TIME);
        }
    } else {
        tank->dynamic_object.velocity.x = mathfMoveTowards(
            tank->dynamic_object.velocity.x,
            0.0f,
            TANK_ACCEL * delta_time
        );
        tank->dynamic_object.velocity.z = mathfMoveTowards(
            tank->dynamic_object.velocity.z,
            0.0f,
            TANK_ACCEL * delta_time
        );
    }

    tank->dynamic_object.rotation.x = current_dir.y;
    tank->dynamic_object.rotation.y = current_dir.x;

    bullet_update(&tank->bullet, delta_time);

    rampage_tank_contact_damage(tank);

    tank->fire_timer -= delta_time;
}

void rampage_tank_render(struct RampageTank* tank) {
    struct Quaternion quat;
    quatAxisComplex(&gUp, &tank->dynamic_object.rotation, &quat);
    T3DVec3 scale = {{1.0f, 1.0f, 1.0f}};

    t3d_mat4fp_from_srt(UncachedAddr(&tank->mtx), scale.v, (float*)&quat, (float*)&tank->dynamic_object.position);
    t3d_matrix_push(&tank->mtx);
    rspq_block_run(rampage_assets_get()->tankSplit.mesh);
    t3d_matrix_pop(1);
}

void rampage_tank_render_bullets(struct RampageTank* tank) {
    bullet_render(&tank->bullet);
}

#define TANK_RADIUS SCALE_FIXED_POINT(1.27636f * 0.5f)
#define TANK_HEIGHT SCALE_FIXED_POINT(0.63024f)

#define BULLET_RADIUS   SCALE_FIXED_POINT(0.4f)
#define BULLET_HEIGHT   SCALE_FIXED_POINT(0.2f)

void rampage_tank_redraw_rect(T3DViewport* viewport, struct RampageTank* tank) {
    struct RedrawRect rect;
    redraw_get_screen_rect(viewport, &tank->dynamic_object.position, TANK_RADIUS, 0.0f, TANK_HEIGHT, &rect);
    redraw_update_dirty(tank->redraw_handle, &rect);

    if (tank->bullet.is_active) {
        redraw_get_screen_rect(viewport, &tank->bullet.dynamic_object.position, BULLET_RADIUS, -BULLET_HEIGHT, BULLET_HEIGHT * 2.0f, &rect);
        redraw_update_dirty(tank->bullet_redraw_handle, &rect);
    }
}