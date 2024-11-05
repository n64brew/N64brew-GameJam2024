#ifndef __RAMPAGE_BUILDING_H__
#define __RAMPAGE_BUILDING_H__

#include <libdragon.h>
#include <t3d/t3dmath.h>
#include <stdint.h>

struct MallardBuilding {
    T3DVec3 position;
    short hp;
    T3DMat4FP mtx;
};

void mallard_building_init(struct MallardBuilding* building, T3DVec3* position);
void mallard_building_destroy(struct MallardBuilding* building);

void mallard_building_render(struct MallardBuilding* building);
void mallard_building_update(struct MallardBuilding* building, float delta_time);

#endif