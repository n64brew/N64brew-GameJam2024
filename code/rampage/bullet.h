#ifndef __BULLET_H__
#define __BULLET_H__

#include <t3d/t3dmath.h>
#include <libdragon.h>

#include "./collision/dynamic_object.h"
#include "./health.h"
#include "./bullet.h"

struct Bullet {
    struct dynamic_object dynamic_object;
    T3DMat4FP mtx;
    uint32_t is_active: 1;
    int last_hit_by;
    struct health health;
};

void bullet_init(struct Bullet* bullet, int collision_group);
void bullet_fire(struct Bullet* bullet, struct Vector3* from, struct Vector2* rotation);

void bullet_update(struct Bullet* bullet, float delta_time);
void bullet_render(struct Bullet* bullet);

void bullet_destroy(struct Bullet* bullet);

#endif