#ifndef __RAMPAGE_TANK_H__
#define __RAMPAGE_TANK_H__

#include <t3d/t3dmath.h>
#include <libdragon.h>
#include "./redraw_manager.h"

#include "./collision/dynamic_object.h"
#include "./health.h"
#include "./bullet.h"

#define MAX_HIT_COUNT   8

struct RampageTank {
    struct dynamic_object dynamic_object;
    T3DMat4FP mtx;
    struct Vector3 current_target;
    uint32_t is_active: 1;
    struct Bullet bullet;
    float fire_timer;
    struct health health;
    int last_hit_by;
    uint8_t already_hit_ids[MAX_HIT_COUNT];

    RedrawHandle redraw_handle;
    RedrawHandle bullet_redraw_handle;
};

void rampage_tank_init(struct RampageTank* tank, struct Vector3* start_position);
void rampage_tank_destroy(struct RampageTank* tank);

void rampage_tank_update(struct RampageTank* tank, float delta_time);
void rampage_tank_render(struct RampageTank* tank);
void rampage_tank_render_bullets(struct RampageTank* tank);

void rampage_tank_redraw_rect(T3DViewport* viewport, struct RampageTank* tank);

#endif