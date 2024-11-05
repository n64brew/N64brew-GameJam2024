#ifndef __RAMPAGE_BUILDING_H__
#define __RAMPAGE_BUILDING_H__

#include <libdragon.h>
#include <t3d/t3dmath.h>
#include <stdint.h>

#include "./collision/dynamic_object.h"

struct RampageBuilding {
    struct dynamic_object dynamic_object;
    short hp;
    T3DMat4FP mtx;
};

void rampage_building_init(struct RampageBuilding* building, T3DVec3* position);
void rampage_building_destroy(struct RampageBuilding* building);

void rampage_building_render(struct RampageBuilding* building);
void rampage_building_update(struct RampageBuilding* building, float delta_time);

#endif