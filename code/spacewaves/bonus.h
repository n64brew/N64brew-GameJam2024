#ifndef SPACEWAVES_BONUS_H
#define SPACEWAVES_BONUS_H

#include <libdragon.h>
#include <time.h>
#include <unistd.h>
#include <display.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "../../core.h"
#include "../../minigame.h"
#include "world.h"
#include "gamestatus.h"

#define MAX_BONUSES 10

typedef enum bonustype_s{
    BONUS_ROCKETS,
    BONUS_UPGRADE,
    BONUS_SHIELD,
    BONUS_POINTS
} bonustype_t;

typedef struct bonus_s{
    bool enabled;
    bonustype_t type;
    T3DVec3 polarpos;
    float time;
    float speedx, speedy;
} bonus_t;

extern bonus_t bonuses[MAX_BONUSES];

extern void bonus_init();
extern void bonus_update();
extern void bonus_apply(int bindex, PlyNum playernum, DefenseStation* station, int craft);
extern void bonus_draw();
extern void bonus_close();

#endif