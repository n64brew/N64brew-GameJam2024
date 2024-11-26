#include <libdragon.h>
#include "../../../minigame.h"
#include "../mallard.h"
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "sequence_game_graphics.h"
#include "sequence_game_initialize.h"

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////
sprite_t *sequence_game_mallard_idle_sprite;
sprite_t *sequence_game_background_lakeview_terrace_sprite;

sprite_t *sequence_game_start_button_sprite;
sprite_t *sequence_game_paused_text_sprite;

int sequence_game_frame = 0;
bool sequence_game_initialized = false;
bool sequence_game_paused = false;

struct Character *characters;
struct Controller *controllers;

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

    sequence_game_paused_text_sprite = sprite_load("rom:/mallard/mallard_game_paused_text.rgba32.sprite");

    sequence_game_initialized = true;

    initialize_characters();
    initialize_controllers();
}

void sequence_game_cleanup()
{
    // Free the sprites.

    // Game
    sprite_free(sequence_game_mallard_idle_sprite);
    sprite_free(sequence_game_background_lakeview_terrace_sprite);

    sprite_free(sequence_game_start_button_sprite);
    sprite_free(sequence_game_paused_text_sprite);

    free_characters();
    free_controllers();

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
    if (!sequence_game_initialized)
    {
        sequence_game_init();
    }

    if (sequence_game_finished)
    {
        sequence_game_cleanup();
        return;
    }

    sequence_game_update(deltatime);

    sequence_game_render(deltatime);
}