#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

/*********************************
             Globals
*********************************/

// You need this function defined somewhere in your project
// so that the minigame manager can work
const MinigameDef minigame_def = {
    .gamename = "Mallard 64",
    .developername = "Josh Kautz",
    .description = "Migration is for cowards. Fight off the snowmen to keep the temperature warm!",
    .instructions = "Destroy the snowmen!"
};

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{

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

}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{

}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{

}