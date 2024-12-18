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
    .description = "Paint enemies to capture & score for your team. Winning color or last one standing gets more points!",
    .instructions = "Move with analog stick, press C/D pad to shoot. Press START to pause."
};

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
extern "C" void minigame_init()
{
    game = new Game();
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
    game->fixedUpdate(deltatime);
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
extern "C" void minigame_loop(float deltatime)
{
    game->render(deltatime);
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
extern "C" void minigame_cleanup()
{
    delete game;
}