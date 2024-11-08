#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

const MinigameDef minigame_def = {
    .gamename = "Z MRE",
    .developername = "Your Name",
    .description = "This is an example game.",
    .instructions = "Press A to win."};

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
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
    joypad_buttons_t btn = joypad_get_buttons_pressed(core_get_playercontroller(0));
    if (btn.a)
        fprintf(stderr, "Player pressed A\n");
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    display_close();
}