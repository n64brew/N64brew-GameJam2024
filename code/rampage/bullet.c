#include "bullet.h"

#include "./collision/collision_scene.h"
#include "./collision/sphere.h"
#include "./util/entity_id.h"
#include "./rampage.h"
#include "./math/quaternion.h"
#include "./assets.h"

#define BULLET_SPEED    300

struct dynamic_object_type bullet_collider = {
    .minkowsi_sum = sphere_minkowski_sum,
    .bounding_box = sphere_bounding_box,
    .data = {
        .sphere = {
            .radius = SCALE_FIXED_POINT(0.125f),
        },
    },
    .bounce = 0.0f,
    .friction = 0.0f,
};

void bullet_deactivate(struct Bullet* bullet) {
    if (!bullet->is_active) {
        return;
    }

    bullet->is_active = false;
    collision_scene_remove(&bullet->dynamic_object);
}

void bullet_init(struct Bullet* bullet, int collision_group) {
    bullet->is_active = false;

    int entity_id = entity_id_next();

    dynamic_object_init(
        entity_id,
        &bullet->dynamic_object,
        &bullet_collider,
        COLLISION_LAYER_TANGIBLE,
        &gZeroVec,
        &gRight2
    );

    bullet->dynamic_object.collision_group = collision_group;
    bullet->dynamic_object.has_gravity = 0;
}

void bullet_fire(struct Bullet* bullet, struct Vector3* from, struct Vector2* rotation) {
    if (bullet->is_active) {
        return;
    }

    bullet->dynamic_object.position = *from;
    bullet->dynamic_object.rotation = *rotation;

    bullet->dynamic_object.velocity.x = rotation->y * BULLET_SPEED;
    bullet->dynamic_object.velocity.y = 0.0f;
    bullet->dynamic_object.velocity.z = rotation->x * BULLET_SPEED;

    collision_scene_add(&bullet->dynamic_object);

    bullet->is_active = true;
}

void bullet_update(struct Bullet* bullet, float delta_time) {
    if (!bullet->is_active) {
        return;
    }

    int is_outside_bounds = 
        bullet->dynamic_object.position.x > MAX_X ||
        bullet->dynamic_object.position.x < MIN_X ||
        bullet->dynamic_object.position.z > MAX_Z ||
        bullet->dynamic_object.position.z < MIN_Z;

    if (bullet->dynamic_object.active_contacts || is_outside_bounds) {
        health_contact_damage(
            bullet->dynamic_object.active_contacts, 
            1, 
            &bullet->dynamic_object.velocity, 
            bullet->dynamic_object.entity_id,
            NULL,
            0
        );
        bullet_deactivate(bullet);
        return;
    }
}

void bullet_render(struct Bullet* bullet) {
    if (!bullet->is_active) {
        return;
    }

    struct Quaternion quat;
    quatAxisComplex(&gUp, &bullet->dynamic_object.rotation, &quat);
    T3DVec3 scale = {{1.0f, 1.0f, 1.0f}};

    t3d_mat4fp_from_srt(UncachedAddr(&bullet->mtx), scale.v, (float*)&quat, (float*)&bullet->dynamic_object.position);
    t3d_matrix_push(&bullet->mtx);
    rspq_block_run(rampage_assets_get()->bullet->userBlock);
    t3d_matrix_pop(1);
}

void bullet_destroy(struct Bullet* bullet) {
    bullet_deactivate(bullet);
}