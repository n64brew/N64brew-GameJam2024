#ifndef __RAMPAGE_H__
#define __RAMPAGE_H__

#include "./player.h"
#include "./building.h"
#include "./tank.h"
#include "./props.h"
#include "./spark_effect.h"

#define BUILDING_COUNT_X    5
#define BUILDING_COUNT_Y    4

#define PLAYER_COUNT    4
#define TANK_COUNT      4

#define SCALE_FIXED_POINT(value)    ((value) * 64.0f)

#define BUILDING_SPACING    SCALE_FIXED_POINT(3.0f)

enum RampageState {
    RAMPAGE_STATE_START,
    RAMPAGE_STATE_PLAYING,
    RAMPAGE_STATE_FINISHED,
    RAMPAGE_STATE_END_SCREEN,
};

struct Rampage {
    struct RampagePlayer players[PLAYER_COUNT];
    struct RampageBuilding buildings[BUILDING_COUNT_Y][BUILDING_COUNT_X];
    struct RampageTank tanks[TANK_COUNT];
    struct AllProps props;
    enum RampageState state;
    float delay_timer;
    int winner_count;
    int winner_mask;
};

void rampage_init(struct Rampage* rampage);
void rampage_destroy(struct Rampage* rampage);

#endif