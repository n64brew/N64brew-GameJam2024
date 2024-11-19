#include <libdragon.h>
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "sequence_game_graphics.h"
#include "../../minigame.h"

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////
sprite_t *sequence_game_mallard_idle_sprite;
sprite_t *sequence_game_background_lakeview_terrace_sprite;

sprite_t *sequence_game_start_button_sprite;

int sequence_game_frame = 0;
bool sequence_game_initialized = false;
bool sequence_game_paused = false;

void sequence_game_init()
{
    ///////////////////////////////////////////////////////////
    //                  Set up Display                       //
    ///////////////////////////////////////////////////////////

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    ///////////////////////////////////////////////////////////
    //                  Set up Graphics                      //
    ///////////////////////////////////////////////////////////

    // Game - Mallard
    sequence_game_mallard_idle_sprite = sprite_load("rom:/mallard/mallard_idle.rgba32.sprite");

    // Gmae - Background
    sequence_game_background_lakeview_terrace_sprite = sprite_load("rom:/mallard/mallard_background_park.rgba32.sprite");

    sequence_game_start_button_sprite = sprite_load("rom:/core/StartButton.sprite");

    sequence_game_initialized = true;
}

void sequence_game_cleanup()
{
    // Free the sprites.

    // Game
    sprite_free(sequence_game_mallard_idle_sprite);
    sprite_free(sequence_game_background_lakeview_terrace_sprite);

    sprite_free(sequence_game_start_button_sprite);

    // Close the display and free the allocated memory.
    rspq_wait();
    display_close();

    // Reset the state.
    sequence_game_initialized = false;

    // TODO: Check to make sure that we're resetting the state of a lot of things...

    // End the sequence.
    sequence_game_finished = true;
}

void sequence_game(float deltatime)
{
    sequence_game_process_controller(deltatime);

    if (!sequence_game_initialized)
    {
        sequence_game_init();
    }

    if (sequence_game_finished)
    {
        sequence_game_cleanup();
        return;
    }

    rdpq_attach(display_get(), NULL);
    rdpq_clear(BLACK);

    sequence_game_render(deltatime);

    rdpq_detach_show();
}