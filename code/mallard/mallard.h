#ifndef __RAMPAGE_H__
#define __RAMPAGE_H__

#include "./player.h"
#include "./building.h"

#define BUILDING_COUNT_X    5
#define BUILDING_COUNT_Y    4

#define PLAYER_COUNT    4

#define SCALE_FIXED_POINT(value)    ((value) * 64.0f)

struct Mallard {
    struct MallardPlayer players[PLAYER_COUNT];
    struct MallardBuilding buildings[BUILDING_COUNT_Y][BUILDING_COUNT_X];
};

void mallard_init(struct Mallard* mallard);
void mallard_destroy(struct Mallard* mallard);

#endif