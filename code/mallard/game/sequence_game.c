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
sprite_t *sequence_game_mallard_one_base_sprite;
sprite_t *sequence_game_mallard_two_base_sprite;
sprite_t *sequence_game_mallard_three_base_sprite;
sprite_t *sequence_game_mallard_four_base_sprite;

sprite_t *sequence_game_mallard_one_walk_sprite;
sprite_t *sequence_game_mallard_two_walk_sprite;
sprite_t *sequence_game_mallard_three_walk_sprite;
sprite_t *sequence_game_mallard_four_walk_sprite;

sprite_t *sequence_game_background_lakeview_terrace_sprite;

sprite_t *sequence_game_start_button_sprite;
sprite_t *sequence_game_paused_text_sprite;

int sequence_game_frame = 0;
bool sequence_game_initialized = false;
bool sequence_game_paused = false;

xm64player_t sequence_game_xm;

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
    sequence_game_mallard_one_base_sprite = sprite_load("rom:/mallard/one/duck_base.rgba32.sprite");
    sequence_game_mallard_two_base_sprite = sprite_load("rom:/mallard/two/duck_base.rgba32.sprite");
    sequence_game_mallard_three_base_sprite = sprite_load("rom:/mallard/three/duck_base.rgba32.sprite");
    sequence_game_mallard_four_base_sprite = sprite_load("rom:/mallard/four/duck_base.rgba32.sprite");

    sequence_game_mallard_one_walk_sprite = sprite_load("rom:/mallard/one/duck_walk_1.rgba32.sprite");
    sequence_game_mallard_two_walk_sprite = sprite_load("rom:/mallard/two/duck_walk_1.rgba32.sprite");
    sequence_game_mallard_three_walk_sprite = sprite_load("rom:/mallard/three/duck_walk_1.rgba32.sprite");
    sequence_game_mallard_four_walk_sprite = sprite_load("rom:/mallard/four/duck_walk_1.rgba32.sprite");

    // Gmae - Background
    sequence_game_background_lakeview_terrace_sprite = sprite_load("rom:/mallard/mallard_background_park.rgba32.sprite");

    sequence_game_start_button_sprite = sprite_load("rom:/core/StartButton.sprite");

    sequence_game_paused_text_sprite = sprite_load("rom:/mallard/mallard_game_paused_text.rgba32.sprite");

    sequence_game_initialized = true;

    initialize_characters();
    initialize_controllers();

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    xm64player_open(&sequence_game_xm, "rom:/mallard/mallard_game_music.xm64");
    xm64player_play(&sequence_game_xm, 0);
}

void sequence_game_cleanup()
{
    // Free the sprites.

    // Game
    sprite_free(sequence_game_mallard_one_base_sprite);
    sprite_free(sequence_game_mallard_two_base_sprite);
    sprite_free(sequence_game_mallard_three_base_sprite);
    sprite_free(sequence_game_mallard_four_base_sprite);
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

    sequence_game_frame++;
}