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
    player_init
    The player initialization function, determines number of 
    players and assigns AI to players that aren't human
==============================*/
void player_init();
/*==============================
    player_loop
    The scans player controllers and updates player entities
    accordingly
==============================*/
void player_loop(float deltaTime, int playerNumber)
{
    joypad_port_t controllerPort = core_get_playercontroller(playerNumber);
    
    if(!joypad_is_connected(controllerPort))
    {
        return;
    }

    joypad_buttons_t btn = joypad_get_buttons_held(controllerPort);

    if(btn.start) minigame_end();

    printf("Buttons pressed by player: %i", playerNumber + 1);

    if(btn.a) printf(" A");
    if(btn.b) printf(" B");
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_X) == -1) printf(" Stick Left");
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_X) == 1) printf(" Stick Right");
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_Y) == 1) printf(" Stick Up");
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_Y) == -1) printf(" Stick Down");

    printf("\n");

    // rumble pak test code
    if(joypad_get_rumble_supported(controllerPort) && btn.a)
    {
        joypad_set_rumble_active(controllerPort, true);
    }

    if(joypad_get_rumble_supported(controllerPort) && btn.b)
    {
        joypad_set_rumble_active(controllerPort, false);
    }
}

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

    for(int i = 0; i < core_get_playercount(); i++)
    {
        player_loop(deltatime, i);
    }

    console_render();

    //attach the rdp queue in order to dispatch commands to the RDP
    //rdpq_attach(display_get(), display_get_zbuf());

    //detach the queue to flip the buffers and show it on screen
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