/***************************************************************
                             main.c
                               
The ROM entrypoint, which initializes the minigame template core
and provides a basic game loop.
***************************************************************/

#include <libdragon.h>
#include <time.h>
#include <unistd.h>
#include "core.h"
#include "menu.h"
#include "config.h"
#include "minigame.h"


/*==============================
    main
    The program main
==============================*/

int main()
{
    #if DEBUG
    	debug_init_isviewer();
    	debug_init_usblog();
    #endif
    
    // Initialize most subsystems
    asset_init_compression(2);
    asset_init_compression(3);
    dfs_init(DFS_DEFAULT_LOCATION);
    debug_init_usblog();
    debug_init_isviewer();
    joypad_init();
    timer_init();
    rdpq_init();
    minigame_loadall();
    audio_init(32000, 3);
    mixer_init(32);

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

    // Program Loop
    while (1)
    {
        char* game;
        float accumulator = 0;
        const float dt = DELTATIME;

        // Show the menu
        game = menu();
        
        // Set the initial minigame
        minigame_play(game);

        // Initialize the minigame
        core_reset_winners();
        minigame_get_game()->funcPointer_init();
        
        // Handle the engine loop
        while (!minigame_get_ended())
        {
            float frametime = display_get_delta_time();
            
            // In order to prevent problems if the game slows down significantly, we will clamp the maximum timestep the simulation can take
            if (frametime > 0.25f)
                frametime = 0.25f;
            
            // Perform the update in discrete steps (ticks)
            if (minigame_get_game()->funcPointer_fixedloop) {
                accumulator += frametime;
                while (accumulator >= dt)
                {
                    minigame_get_game()->funcPointer_fixedloop(dt);
                    accumulator -= dt;
                }
            }

            // Read controler data
            joypad_poll();
            mixer_try_play();
            
            // Perform the unfixed loop
            core_set_subtick(((double)accumulator)/((double)dt));
            minigame_get_game()->funcPointer_loop(frametime);
        }
        
        // End the current level
        rspq_wait();
        for (int i=0; i<32; i++)
            mixer_ch_stop(i);
        minigame_get_game()->funcPointer_cleanup();
        minigame_cleanup();
    }
}