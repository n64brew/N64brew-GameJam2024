#ifndef __RAMPAGE_BUILDING_H__
#define __RAMPAGE_BUILDING_H__

#include <libdragon.h>
#include <t3d/t3dmath.h>
#include <stdint.h>

#include "./collision/dynamic_object.h"
#include "./health.h"
#include "./redraw_manager.h"

struct RampageBuilding {
    struct dynamic_object dynamic_object;
    uint32_t hp:5;
    uint32_t is_destroyed:1;
    uint32_t is_collapsing:1;
    uint32_t height: 3;
    uint32_t rotation: 3;
    T3DMat4FP mtx;
    struct health health;
    float shake_timer;

    RedrawHandle redraw_handle;
};

void rampage_building_init(struct RampageBuilding* building, T3DVec3* position, int rotation);
void rampage_building_destroy(struct RampageBuilding* building);

void rampage_building_render(struct RampageBuilding* building, int height_pass);
void rampage_building_update(struct RampageBuilding* building, float delta_time);

void rampage_building_redraw_rect(T3DViewport* viewport, struct RampageBuilding* building);

#endif