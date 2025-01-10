/***************************************************************
                             main.c
                               
The ROM entrypoint, which initializes the minigame template core
and provides a basic game loop.
***************************************************************/

#include <libdragon.h>
#include <time.h>
#include <unistd.h>
#include "core.h"
#include "logo.h"
#include "menu.h"
#include "config.h"
#include "minigame.h"
#include "savestate.h"

#define DEBUG 1


/*==============================
    main
    The program main
==============================*/

int main()
{
    #if DEBUG_LOG == 1
    	debug_init_isviewer();
    	debug_init_usblog();
    #endif
    
    // Initialize most subsystems
    asset_init_compression(2);
    asset_init_compression(3);
    wav64_init_compression(3);
    dfs_init(DFS_DEFAULT_LOCATION);

    joypad_init();
    timer_init();
    rdpq_init();
    audio_init(32000, 3);
    mixer_init(32);
    savestate_initialize();

    // Enable RDP debugging
    #if DEBUG_RDP
        rdpq_debug_start();
        rdpq_debug_log(true);
        rspq_profile_start();
    #endif

    // Initialize the random number generator, then call rand() every
    // frame so to get random behavior also in emulators.
    uint32_t seed;
    getentropy(&seed, sizeof(seed));
    srand(seed);
    register_VI_handler((void(*)(void))rand);

    // Show logos
    if (sys_reset_type() == RESET_COLD) {
       n64brew_logo();
       libdragon_logo();
    }

    // Load all minigames
    minigame_loadall();

    // Initialize the level system
    core_initlevels();
    core_level_changeto(LEVEL_LOADSAVE);

    // Program Loop
    while (1)
    {
        float accumulator = 0;
        const float dt = DELTATIME;

        // Initialize the level
        core_level_doinit();
        
        // Handle the engine loop
        while (!core_level_waschanged())
        {
            float frametime = display_get_delta_time();
            
            // In order to prevent problems if the game slows down significantly, we will clamp the maximum timestep the simulation can take
            if (frametime > 0.25f)
                frametime = 0.25f;
            
            // Perform the update in discrete steps (ticks)
            accumulator += frametime;
            while (accumulator >= dt)
            {
                core_level_dofixedloop(dt);
                accumulator -= dt;
            }

            // Read controler data
            joypad_poll();
            mixer_try_play();
            
            // Perform the unfixed loop
            core_set_subtick(((double)accumulator)/((double)dt));
            core_level_doloop(frametime);
        }
        
        // End the current level
        core_level_docleanup();
    }
}
