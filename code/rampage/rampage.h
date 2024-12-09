#ifndef __RAMPAGE_H__
#define __RAMPAGE_H__

#include "./player.h"
#include "./building.h"
#include "./tank.h"
#include "./props.h"
#include "./spark_effect.h"
#include "./redraw_manager.h"

#define BUILDING_COUNT_X    5
#define BUILDING_COUNT_Y    4

#define PLAYER_COUNT    4
#define TANK_COUNT      4

#define SCALE_FIXED_POINT(value)    ((value) * 64.0f)

#define BUILDING_SPACING    SCALE_FIXED_POINT(3.0f)

#define MIN_X               SCALE_FIXED_POINT(-9.0f)
#define MAX_X               SCALE_FIXED_POINT(8.5f)
#define MIN_Z               SCALE_FIXED_POINT(-8.0f)
#define MAX_Z               SCALE_FIXED_POINT(8.0f)

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
    RedrawHandle center_text_redraw;
    RedrawHandle score_redraw[PLAYER_COUNT];
};

void rampage_init(struct Rampage* rampage);
void rampage_destroy(struct Rampage* rampage);

#endif