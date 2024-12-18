#ifndef LUCKER_H
#define LUCKER_H
#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>


typedef struct
{
    bool isSpinning;
    float slotTimer;
    int currentSelection[3];
    bool finished[3];
    
    bool left;
    
    T3DVec3 pos[3];
    T3DVec3 rot[3];
    T3DMat4FP* firstMatFP;
    T3DMat4FP* secondMatFP;
    T3DMat4FP* thirdMatFP;
    rspq_block_t *dplWheel;
    
} slot;

typedef struct 
{
    T3DVec3 pos;
    T3DVec3 rot;
    T3DAnim animAttack;
    T3DAnim animWalk;
    T3DAnim animIdle;
    T3DAnim animDeath;
    T3DAnim animJump;
    T3DSkeleton skelBlend;
    T3DSkeleton skel;
    T3DModel* model;
    color_t color;
    T3DMat4FP* fighterMatFP;
    float animBlend;
    bool onLeftSide;
    bool boom;
    
    bool isAttack;
    bool isStunned;
    float stunTimer;
    float attackTimer;
    float abilityTimers[10];
    int messageboard[4];
    bool abilityActive[10];
    int lastDamageCrit;
    float hp;
    
} fighterData;

typedef struct
{
    PlyNum  playerNumber;
    bool isHuman;
    int wins;
    slot sl;
    fighterData fighter;

} player;

typedef enum {
        SWORD = 0,
        HEART = 1,
        BOMB = 2,
        SWAP_HP = 3,
        EVADE_REMOVE = 4,
        BURST = 5,
        CRIT = 6,
        VAMPIRISM = 7,
        LIGHTNING = 8,
        EVADE = 9,
} wheel;

static inline int deca(wheel icon) {
    return (icon + 1) * 36;
}
static inline int rad_to_deg(float radian) 
{
    int x = (int)floorf(radian * (180/3.14f));
    return x%360;
}
static inline int randomSelection() {
    int x = rand()%800;
    
    //do nothing!
    if (x >= 0 && x < 480)
    {
        return -1;
    } else if (x >= 480 && x < 680) 
    {
        //200 common
        if (x >= 480 && x < 530) 
        {
            return LIGHTNING;
        } else if (x >= 530 && x < 580)
        {
            return HEART;
        } else if (x >= 580 && x < 630)
        {
            return SWORD;
        } else // x >= 630 && x < 680
        {
            return EVADE;
        }

    } else if (x >= 680 && x < 780) 
    {
        //100 uncommon
        if (x >= 680 && x < 705) 
        {
            return VAMPIRISM;
        } else if (x >= 705 && x < 730)
        {
            return CRIT;
        } else if (x >= 730 && x < 755)
        {
            return BURST;
        } else // x >= 755 && x < 780
        {
            return EVADE_REMOVE;
        }
    } else {
        //20 rare
        if (x >= 780 && x < 790) 
        {
            return SWAP_HP;
        } else 
        {
            return BOMB;
        }
    }
}
#endif  // BATTLE_H