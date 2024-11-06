/***************************************************************
                         examplegame.c
                               
An example minigame to demonstrate how to use the template for
the game jam.
***************************************************************/

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

/*********************************
             Globals
*********************************/

float timer;

// You need this function defined somewhere in your project
// so that the minigame manager can work
const MinigameDef minigame_def = {
    .gamename = "RiPpEr's Game",
    .developername = "RiPpEr253",
    .description = "This is an example game.",
    .instructions = "Mash A to win."
};

/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    timer = 0.0f;

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);

    console_init();
    console_set_render_mode(RENDER_MANUAL);

    return;
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
    timer += deltatime;

    return;
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/

void minigame_loop(float deltatime)
{
    console_clear();
    printf("Time running: %f \n", timer);
    printf("Press Start to quit\n");

    joypad_buttons_t btn = joypad_get_buttons_pressed(core_get_playercontroller(1));

    if(btn.start) minigame_end();

    console_render();

    //rdpq_attach(display_get(), display_get_zbuf());

    //rdpq_detach_show();
    return;
}


/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    console_close();
    display_close();
    return;
}