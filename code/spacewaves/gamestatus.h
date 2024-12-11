#ifndef GAMESTATUS_H
#define GAMESTATUS_H

#include "../../core.h"
#include "../../minigame.h"

typedef enum gamestate_s{
    GAMESTATE_PLAY,
    GAMESTATE_GETREADY,
    GAMESTATE_COUNTDOWN,
    GAMESTATE_PAUSED,
    GAMESTATE_TRANSITION,
    GAMESTATE_FINISHED
} gamestate_t;

typedef struct gamestatus_s{
    double currenttime;
    double realtime;
    double deltatime;
    double deltarealtime;
    double fixeddeltatime;
    double fixedtime;
    double fixedframerate;

    double gamespeed;
    bool   paused;

    int playerscores[MAXPLAYERS];
    gamestate_t state;
    float statetime;
    int winner;
} gamestatus_t;

extern gamestatus_t gamestatus;

#define CURRENT_TIME            gamestatus.currenttime
#define CURRENT_TIME_REAL       gamestatus.realtime

#define GAMESPEED               gamestatus.gamespeed
#define GAME_PAUSED             gamestatus.paused

#define DELTA_TIME              gamestatus.deltatime
#define DELTA_TIME_REAL         gamestatus.deltarealtime

#define DELTA_TIME_FIXED        gamestatus.fixeddeltatime
#define CURRENT_TIME_FIXED      gamestatus.fixedtime

void timesys_init();
void timesys_update();
bool timesys_update_fixed();
void timesys_close();

#endif