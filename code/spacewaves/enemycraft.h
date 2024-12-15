#ifndef ENEMYCRAFT_H
#define ENEMYCRAFT_H

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

#define NUM_CRAFTS 3
#define MAX_PROJECTILES 16

typedef struct enemycraft_s{
    PlyNum currentplayer;
    joypad_port_t currentplayerport;
    color_t color;
    
    T3DVec4     rotq;
    T3DMat4FP*  matx;

    float yaw, pitch;
    float yawoff, pitchoff;
    float distance, distanceoff;

    float hp;
    float maxhp;

    struct{
        float asteroidnexttime;
        struct{
            bool enabled;
            T3DVec3 polarpos;
            float xspeed, yspeed;
            float rotation;
            T3DMat4FP* matx;
            float hp;
        } asteroids[MAX_PROJECTILES];
        struct{
            bool enabled;
            T3DVec3 polarpos;
            T3DMat4FP* matx;
            float hp;
        } rockets[MAX_PROJECTILES];
        float rocketnexttime;
        int rocketcount;
        rspq_block_t* rocketdl;
        rspq_block_t* asteroiddl;

        float powerup;
        float shield;
    } arm;

    bool enabled;
    bool bot;

} enemycraft_t;

extern enemycraft_t crafts[3];

void crafts_update();
void crafts_init(PlyNum currentstation);
void crafts_draw();
void crafts_close();

#endif