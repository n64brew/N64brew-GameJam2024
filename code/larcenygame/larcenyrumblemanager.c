#include "./larcenyrumblemanager.h"
#include <libdragon.h>

playerrumble_data playerRumbleInfo[MAXPLAYERS];

void rumbleManager_init(player_data* playerInfo)
{
    // go through and check if each controller has a rumble pak or not
    for(uint32_t iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        // get the controller port of each player and record it
        playerRumbleInfo[iDx].controllerPort = core_get_playercontroller(iDx);

        // identify if a rumble pak is inserted
        if(joypad_get_rumble_supported(playerRumbleInfo[iDx].controllerPort))
        {
            playerRumbleInfo[iDx].hasRumblePak = true;
        }
        else
        {
            playerRumbleInfo[iDx].hasRumblePak = false;
        }

        // AI guard
        if(playerInfo[iDx].isAi)
        {
            playerRumbleInfo[iDx].hasRumblePak = false;
        }

        playerRumbleInfo[iDx].isActive = false;
        playerRumbleInfo[iDx].currentRumbleTime = 0.0f;
        playerRumbleInfo[iDx].totalRumbleTime = 0.0f;

        joypad_set_rumble_active(playerRumbleInfo[iDx].controllerPort, false);
    }
}

void rumbleManager_startRumble(int playerNumber, float timeToRumbleFor)
{
    if(playerRumbleInfo[playerNumber].hasRumblePak)
    {
        playerRumbleInfo[playerNumber].isActive = true;
        playerRumbleInfo[playerNumber].currentRumbleTime = 0.0f;
        playerRumbleInfo[playerNumber].totalRumbleTime = timeToRumbleFor;

        joypad_set_rumble_active(playerRumbleInfo[playerNumber].controllerPort, true);
    }
}

void rumbleManager_stopRumble(int playerNumber)
{
    if(playerRumbleInfo[playerNumber].hasRumblePak)
    {
        playerRumbleInfo[playerNumber].isActive = false;
        playerRumbleInfo[playerNumber].currentRumbleTime = 0.0f;
        playerRumbleInfo[playerNumber].totalRumbleTime = 0.0f;

        joypad_set_rumble_active(playerRumbleInfo[playerNumber].controllerPort, false);
    }
}

void rumbleManager_update(float deltaTime)
{
    for(uint32_t iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(playerRumbleInfo[iDx].hasRumblePak && playerRumbleInfo[iDx].isActive)
        {
            playerRumbleInfo[iDx].currentRumbleTime += deltaTime;

            if(playerRumbleInfo[iDx].currentRumbleTime > playerRumbleInfo[iDx].totalRumbleTime)
            {
                rumbleManager_stopRumble(iDx);
            }
        }
    }
}

void rumbleManager_cleanup()
{
    return;
}