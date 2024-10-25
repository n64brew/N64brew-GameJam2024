#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include "./src/game.hpp"

Game* game;

const MinigameDef minigame_def = {
    .gamename = "Paintball",
    .developername = "Ali Naci Erdem",
    .description = "...",
    .instructions = "Move with analog stick, press A to shoot paintballs and aim with C/D pad."
};

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    game = Game_new();
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
    Game_fixed_update(game, deltatime);
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
    Game_update(game, deltatime);
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    Game_destroy(game);
}