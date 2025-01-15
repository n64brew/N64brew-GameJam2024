#ifndef GAMEJAM2024_LARCENYRUMBLEMANAGER_H
#define GAMEJAM2024_LARCENYRUMBLEMANAGER_H 

#include "./larcenygame.h"
#include "../../core.h"
#include "../../minigame.h"

    // struct current player rumble data
    typedef struct
    {
        joypad_port_t controllerPort;
        bool hasRumblePak;
        bool isActive;
        float currentRumbleTime; // measured in deltaTime (ms)
        float totalRumbleTime;
    } playerrumble_data;

    void rumbleManager_init(player_data* playerInfo);
    void rumbleManager_startRumble(int playerNumber, float timeToRumbleFor);
    void rumbleManager_stopRumble(int playerNumber);
    void rumbleManager_update(float deltaTime);
    void rumbleManager_cleanup();

#endif