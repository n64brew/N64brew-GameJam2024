#include "./larcenyrumblemanager.h"
#include <libdragon.h>

playerrumble_data playerRumbleInfo[MAXPLAYERS];

/*==============================
    rumbleManager_init
    enumerates controllers and ascertain vibration capability
    for all players
==============================*/
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

        // If they're AI, they don't have RumblePak's 
        if(playerInfo[iDx].isAi)
        {
            playerRumbleInfo[iDx].hasRumblePak = false;
        }

        playerRumbleInfo[iDx].isActive = false;
        playerRumbleInfo[iDx].currentRumbleTime = 0.0f;
        playerRumbleInfo[iDx].totalRumbleTime = 0.0f;

        // make sure a previous rumble is cancelled, just in case
        joypad_set_rumble_active(playerRumbleInfo[iDx].controllerPort, false);
    }
}

/*==============================
    rumbleManager_startRumble
    starts a rumble for the passed in player, for the amount of time in seconds
==============================*/
void rumbleManager_startRumble(int playerNumber, float timeToRumbleFor)
{
    // make sure the player attempting to have their world
    // rocked actually is blessed enough to have a rumble pak
    if(playerRumbleInfo[playerNumber].hasRumblePak)
    {
        // if they do, set the active flag and set the time
        playerRumbleInfo[playerNumber].isActive = true;
        playerRumbleInfo[playerNumber].currentRumbleTime = 0.0f;
        playerRumbleInfo[playerNumber].totalRumbleTime = timeToRumbleFor;

        // rock their world
        joypad_set_rumble_active(playerRumbleInfo[playerNumber].controllerPort, true);
    }
}

/*==============================
    rumbleManager_stopRumble
    used to stop rumble effects, can be called prematurely
==============================*/
void rumbleManager_stopRumble(int playerNumber)
{
    // make sure there's a rumble pak to talk to, don't fill up joybus for no reason
    if(playerRumbleInfo[playerNumber].hasRumblePak)
    {
        playerRumbleInfo[playerNumber].isActive = false;
        playerRumbleInfo[playerNumber].currentRumbleTime = 0.0f;
        playerRumbleInfo[playerNumber].totalRumbleTime = 0.0f;

        joypad_set_rumble_active(playerRumbleInfo[playerNumber].controllerPort, false);
    }
}

/*==============================
    rumbleManager_update
    checks and updates all rumble timers currently underway
==============================*/
void rumbleManager_update(float deltaTime)
{
    // check if each player has an active rumble underway
    for(uint32_t iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(playerRumbleInfo[iDx].hasRumblePak && playerRumbleInfo[iDx].isActive)
        {
            // if they do, increment the timer by deltatime
            playerRumbleInfo[iDx].currentRumbleTime += deltaTime;

            // if it's run out of time, stop it
            if(playerRumbleInfo[iDx].currentRumbleTime > playerRumbleInfo[iDx].totalRumbleTime)
            {
                rumbleManager_stopRumble(iDx);
            }
        }
    }
}

/*==============================
    rumbleManager_cleanup
    ensures there's no hanging rumbles upon exit of the game
==============================*/
void rumbleManager_cleanup()
{
    // when cleaning up, make sure that there's no errant rumble going
    for(uint32_t iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(playerRumbleInfo[iDx].hasRumblePak && playerRumbleInfo[iDx].isActive)
        {
            rumbleManager_stopRumble(iDx);
        }
    }

    // send it twice in case the first packet is missed, for safety    
    for(uint32_t iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(playerRumbleInfo[iDx].hasRumblePak && playerRumbleInfo[iDx].isActive)
        {
            rumbleManager_stopRumble(iDx);
        }
    }

    return;
}