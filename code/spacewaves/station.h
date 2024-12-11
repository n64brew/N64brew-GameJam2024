#ifndef STATION_H
#define STATION_H

#include <unistd.h>
#include <time.h>
#include <libdragon.h>
#include <display.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "types.h"

#define MAX_PROJECTILES 16

typedef struct defensestation_t{
    PlyNum currentplayer;
    joypad_port_t currentplayerport;
    color_t color;

    float yaw, pitch;
    float yawoff, pitchoff;
    float yawcam, pitchcam;
    T3DVec4     rotq;
    T3DMat4FP*  matx;
    T3DVec3     forward;

    struct{
        float bulletnexttime;
        struct{
            bool enabled;
            T3DVec3 polarpos;
        } bullets[MAX_PROJECTILES];
        struct{
            bool enabled;
            T3DVec3 polarpos;
            T3DMat4FP* matx;
        } rockets[MAX_PROJECTILES];
        float rocketnexttime;
        int rocketcount;
        rspq_block_t* rocketdl;
        T3DMat4FP*  rocketgunmatx;
        T3DMat4FP*  machinegunmatx;
        float rocketgunoffset;
        float machinegunoffset;
        float powerup;
        float shield;
    } arm;

    float hp;
    float maxhp;
} DefenseStation;
extern DefenseStation station;

void station_init(PlyNum player);

void station_update();

void station_apply_camera();
void station_draw();
void station_close();

#endif