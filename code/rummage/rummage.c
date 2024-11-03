#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#define COUNTDOWN_DELAY     3.0f
#define GAME_BACKGROUND     0xffff00ff


/*********************************
             Globals
*********************************/

const MinigameDef minigame_def = {
    .gamename = "Rummage",
    .developername = "tfmoe__",
    .description = "Find the key and be the first to open the safe!",
    .instructions = "Press A to rummage through the furniture or to steal the key."
};

float countdown_timer;


/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    
    countdown_timer = COUNTDOWN_DELAY;
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
    if (countdown_timer > 0)
    {
        countdown_timer -= deltatime;
    } else {
        core_set_winner(0);
        minigame_end();
    }
}


/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/

void minigame_loop(float deltatime)
{
    // Render the UI
    rdpq_attach(display_get(), NULL);
    rdpq_clear(color_from_packed32(GAME_BACKGROUND));
    rdpq_detach_show();
}


/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    display_close();
}
