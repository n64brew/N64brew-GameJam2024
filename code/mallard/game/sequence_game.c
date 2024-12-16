#include <libdragon.h>
#include "../../../core.h"
#include "../../../minigame.h"
#include "../mallard.h"
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "sequence_game_graphics.h"
#include "sequence_game_initialize.h"
#include "sequence_game_snowman.h"
#include "sequence_game_duck.h"

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

sprite_t *sequence_game_mallard_one_walk_sprite;
sprite_t *sequence_game_mallard_two_walk_sprite;
sprite_t *sequence_game_mallard_three_walk_sprite;
sprite_t *sequence_game_mallard_four_walk_sprite;

sprite_t *sequence_game_mallard_one_slap_sprite;
sprite_t *sequence_game_mallard_two_slap_sprite;
sprite_t *sequence_game_mallard_three_slap_sprite;
sprite_t *sequence_game_mallard_four_slap_sprite;

sprite_t *sequence_game_mallard_one_run_sprite;
sprite_t *sequence_game_mallard_two_run_sprite;
sprite_t *sequence_game_mallard_three_run_sprite;
sprite_t *sequence_game_mallard_four_run_sprite;

sprite_t *sequence_game_mallard_one_idle_sprite;
sprite_t *sequence_game_mallard_two_idle_sprite;
sprite_t *sequence_game_mallard_three_idle_sprite;
sprite_t *sequence_game_mallard_four_idle_sprite;

sprite_t *sequence_game_mallard_one_damage_sprite;
sprite_t *sequence_game_mallard_two_damage_sprite;
sprite_t *sequence_game_mallard_three_damage_sprite;
sprite_t *sequence_game_mallard_four_damage_sprite;

sprite_t *sequence_game_snowman_idle_sprite;
sprite_t *sequence_game_snowman_damage_sprite;
sprite_t *sequence_game_snowman_jump_sprite;

sprite_t *sequence_game_map;

sprite_t *sequence_game_start_button_sprite;
sprite_t *sequence_game_paused_text_sprite;

bool sequence_game_should_initialize = true;
bool sequence_game_did_initialize = false;
bool sequence_game_should_cleanup = false;
bool sequence_game_did_cleanup = false;

bool sequence_game_paused = false;

xm64player_t sequence_game_xm;

AiDiff difficulty;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

Duck *ducks;
Snowman *snowmen;
Controller *controllers;

void sequence_game_init()
{
    ///////////////////////////////////////////////////////////
    //                  Set up Display                       //
    ///////////////////////////////////////////////////////////

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    ///////////////////////////////////////////////////////////
    //                  Set up Graphics                      //
    ///////////////////////////////////////////////////////////

    sequence_game_snowman_idle_sprite = sprite_load("rom:/mallard/snowman/snowman_idle_evil.rgba32.sprite");
    sequence_game_snowman_damage_sprite = sprite_load("rom:/mallard/snowman/snowman_damage_evil.rgba32.sprite");
    sequence_game_snowman_jump_sprite = sprite_load("rom:/mallard/snowman/snowman_jump_evil.rgba32.sprite");

    sequence_game_mallard_one_walk_sprite = sprite_load("rom:/mallard/one/duck_walk_1.rgba32.sprite");
    sequence_game_mallard_two_walk_sprite = sprite_load("rom:/mallard/two/duck_walk_1.rgba32.sprite");
    sequence_game_mallard_three_walk_sprite = sprite_load("rom:/mallard/three/duck_walk_1.rgba32.sprite");
    sequence_game_mallard_four_walk_sprite = sprite_load("rom:/mallard/four/duck_walk_1.rgba32.sprite");

    sequence_game_mallard_one_slap_sprite = sprite_load("rom:/mallard/one/duck_slap.rgba32.sprite");
    sequence_game_mallard_two_slap_sprite = sprite_load("rom:/mallard/two/duck_slap.rgba32.sprite");
    sequence_game_mallard_three_slap_sprite = sprite_load("rom:/mallard/three/duck_slap.rgba32.sprite");
    sequence_game_mallard_four_slap_sprite = sprite_load("rom:/mallard/four/duck_slap.rgba32.sprite");

    sequence_game_mallard_one_run_sprite = sprite_load("rom:/mallard/one/duck_run.rgba32.sprite");
    sequence_game_mallard_two_run_sprite = sprite_load("rom:/mallard/two/duck_run.rgba32.sprite");
    sequence_game_mallard_three_run_sprite = sprite_load("rom:/mallard/three/duck_run.rgba32.sprite");
    sequence_game_mallard_four_run_sprite = sprite_load("rom:/mallard/four/duck_run.rgba32.sprite");

    sequence_game_mallard_one_idle_sprite = sprite_load("rom:/mallard/one/duck_idle.rgba32.sprite");
    sequence_game_mallard_two_idle_sprite = sprite_load("rom:/mallard/two/duck_idle.rgba32.sprite");
    sequence_game_mallard_three_idle_sprite = sprite_load("rom:/mallard/three/duck_idle.rgba32.sprite");
    sequence_game_mallard_four_idle_sprite = sprite_load("rom:/mallard/four/duck_idle.rgba32.sprite");

    sequence_game_mallard_one_damage_sprite = sprite_load("rom:/mallard/one/duck_damage.rgba32.sprite");
    sequence_game_mallard_two_damage_sprite = sprite_load("rom:/mallard/two/duck_damage.rgba32.sprite");
    sequence_game_mallard_three_damage_sprite = sprite_load("rom:/mallard/three/duck_damage.rgba32.sprite");
    sequence_game_mallard_four_damage_sprite = sprite_load("rom:/mallard/four/duck_damage.rgba32.sprite");

    // Gmae - Background
    sequence_game_map = sprite_load("rom:/mallard/mallard_background_park.rgba32.sprite");

    sequence_game_start_button_sprite = sprite_load("rom:/core/StartButton.sprite");

    sequence_game_paused_text_sprite = sprite_load("rom:/mallard/mallard_game_paused_text.rgba32.sprite");

    sequence_game_should_initialize = false;
    sequence_game_did_initialize = true;
    difficulty = core_get_aidifficulty();

    initialize_ducks();
    initialize_controllers();

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    xm64player_open(&sequence_game_xm, "rom:/mallard/mallard_game_music.xm64");

    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");

    xm64player_play(&sequence_game_xm, 0);
}

void sequence_game_cleanup()
{
    // Free the sprites.
    sprite_free(sequence_game_mallard_one_slap_sprite);
    sprite_free(sequence_game_mallard_two_slap_sprite);
    sprite_free(sequence_game_mallard_three_slap_sprite);
    sprite_free(sequence_game_mallard_four_slap_sprite);

    sprite_free(sequence_game_mallard_one_walk_sprite);
    sprite_free(sequence_game_mallard_two_walk_sprite);
    sprite_free(sequence_game_mallard_three_walk_sprite);
    sprite_free(sequence_game_mallard_four_walk_sprite);

    sprite_free(sequence_game_mallard_one_run_sprite);
    sprite_free(sequence_game_mallard_two_run_sprite);
    sprite_free(sequence_game_mallard_three_run_sprite);
    sprite_free(sequence_game_mallard_four_run_sprite);

    sprite_free(sequence_game_mallard_one_idle_sprite);
    sprite_free(sequence_game_mallard_two_idle_sprite);
    sprite_free(sequence_game_mallard_three_idle_sprite);
    sprite_free(sequence_game_mallard_four_idle_sprite);

    sprite_free(sequence_game_snowman_idle_sprite);
    sprite_free(sequence_game_snowman_damage_sprite);
    sprite_free(sequence_game_snowman_jump_sprite);

    sprite_free(sequence_game_map);

    sprite_free(sequence_game_start_button_sprite);
    sprite_free(sequence_game_paused_text_sprite);

    free_ducks();
    free_snowmen();
    free_controllers();

    // Stop the music and free the allocated memory.
    xm64player_stop(&sequence_game_xm);
    xm64player_close(&sequence_game_xm);

    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);

    // Close the display and free the allocated memory.
    rspq_wait();
    display_close();

    // End the sequence.
    sequence_game_did_cleanup = true;
    sequence_game_finished = true;
}

void sequence_game(float deltatime)
{
    if (sequence_game_should_initialize && !sequence_game_did_initialize)
    {
        sequence_game_init();
    }

    sequence_game_update(deltatime);

    sequence_game_render(deltatime);

    if (sequence_game_should_cleanup && !sequence_game_did_cleanup)
    {
        sequence_game_cleanup();
    }
}