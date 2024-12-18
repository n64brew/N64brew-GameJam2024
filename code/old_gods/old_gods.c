#include <libdragon.h>

#include "../../core.h"
#include "../../minigame.h"

#include "App.h"
#include "AF_Time.h"
#include "GameplayData.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 240
resolution_t resolution;
AppData g_appData;

const MinigameDef minigame_def = {
    .gamename = "Sneks",
    .developername = "mr_J05H",
    .description = "Mother snek must be fed rats.",
    .instructions = "Watch out for sneaky sharks."
};


/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{   
    resolution = RESOLUTION_640x240;
    //Initialise the app data structure to default values or zero before use
    AppData_Init(&g_appData, resolution.width, resolution.height);
    g_appData.gameplayData.isDebug = FALSE;
    g_appData.gameTime.lastTime = timer_ticks();

    // init the other app things
    App_Init(&g_appData);// Initialize lastTime before the loop
}

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is 
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/
void minigame_fixedloop(float deltatime)
{
    // set framerate to target 60fp and call the app update function
    //App_Update_Wrapper(1);
    App_Update(&g_appData);
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
    g_appData.gameTime.currentTime = timer_ticks();
    g_appData.gameTime.timeSinceLastFrame = deltatime;
    g_appData.gameTime.lastTime = g_appData.gameTime.currentTime;
    // render stuff as fast as possible, interdependent from other code
    App_Render_Update(&g_appData);
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    App_Shutdown(&g_appData);
}