#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include <stdio.h>
#include <unistd.h>

const MinigameDef minigame_def = {
    .gamename = "A Mallard 64",
    .developername = "Josh Kautz",
    .description = "",
    .instructions = "",
};

// Target screen resolution that we render at.
// Choosing a resolution above 240p (interlaced) can't be recommended for video playback.
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    joypad_init();
    debug_init_isviewer();
    debug_init_usblog();
    core_set_winner(0);

    fprintf(stderr, "Mallard 64 Init\n");

    // minigame_end();
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