#include <libdragon.h>

#include "../../core.h"
#include "../../minigame.h"

#include "App.h"
#include "AF_Time.h"

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

AF_Time gameTime;

const MinigameDef minigame_def = {
    .gamename = "Old Gods",
    .developername = "mr_J05H",
    .description = "Work with or against your villager friends to feed the old gods to keep them contained.",
    .instructions = "Collect the sacrifices and feed them to the gods. Try not to kill each other along the way."
};

    


/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    App_Init(WINDOW_WIDTH, WINDOW_HEIGHT, &gameTime);// Initialize lastTime before the loop

    gameTime.lastTime = timer_ticks();
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
    App_Update_Wrapper(1);
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
    gameTime.currentTime = timer_ticks();
    gameTime.timeSinceLastFrame = deltatime;
    gameTime.lastTime = gameTime.currentTime;
    // render stuff as fast as possible, interdependent from other code
    App_Render_Update(&gameTime);
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    App_Shutdown();
}