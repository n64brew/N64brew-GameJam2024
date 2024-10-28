#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include "./src/game.hpp"

namespace {
  Game* game;
}

extern "C" const MinigameDef minigame_def = {
    .gamename = "Paintball",
    .developername = "Ali Naci Erdem",
    .description = "...",
    .instructions = "Move with analog stick, press A to shoot paintballs and aim with C/D pad."
};

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
extern "C" void minigame_init()
{
    game = new Game();
    game->setup();
}

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is 
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/
extern "C" void minigame_fixedloop(float deltatime)
{
    game->fixed_update(deltatime);
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
extern "C" void minigame_loop(float deltatime)
{
    game->update(deltatime);
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
extern "C" void minigame_cleanup()
{
    delete game;
}