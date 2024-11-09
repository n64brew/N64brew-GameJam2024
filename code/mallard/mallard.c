#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include "sequence1.h"
#include "sequence2.h"
#include "sequence3.h"

#include <stdio.h>
#include <unistd.h>

const MinigameDef minigame_def = {
    .gamename = "Mallard 64",
    .developername = "Josh Kautz",
    .description = "",
    .instructions = "",
};

// Maximum target audio frequency.
//
// Needs to be 48 kHz if Opus audio compression is used.
// In this example, we are using VADPCM audio compression
// which means we can use the real frequency of the audio track.
// #define AUDIO_HZ 32000.0f

// wav64_t audio_track;

// rdpq_font_t *font;

bool sequence_1_libdragon = false;
bool sequence_2_ascii = false;
bool sequence_3_video = false;
bool sequence_4_BLANK = false;

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{

    ///////////////////////////////////////////////////////////
    //                  Set up Font                          //
    ///////////////////////////////////////////////////////////

    // font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    // rdpq_text_register_font(1, font);

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    // Open the audio track and start playing it in channel 0.
    // wav64_open(&audio_track, "rom:/mallard/video.wav64");
    // wav64_play(&audio_track, 0);

    sequence_1_libdragon = true;
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
    if (sequence_1_libdragon)
    {
        sequence_1(deltatime);
        return;
    }

    if (sequence_2_ascii)
    {
        sequence_2(deltatime);
        return;
    }

    if (sequence_3_video)
    {
        sequence_3(deltatime);
        return;
    }

    minigame_end();
    return;

    // handle_input(deltatime);

    ///////////////////////////////////////////////////////////
    //                  Render UI - Pause                    //
    ///////////////////////////////////////////////////////////

    // rdpq_set_mode_standard();
    // if (paused)
    // {
    //     rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 30, 30, "Paused");
    // }

    // ///////////////////////////////////////////////////////////
    // //                  Render UI - Quitting                 //
    // ///////////////////////////////////////////////////////////

    // if (b_btn_held)
    // {
    //     rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, SCREEN_WIDTH - 100, 30, "Quitting in %.2f", 3.0f - sequence3_b_btn_held_duration);

    //     rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    //     rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
    //     rdpq_fill_rectangle(SCREEN_WIDTH - 100, 35, SCREEN_WIDTH - 30, 37);

    //     rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    //     rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
    //     rdpq_fill_rectangle(SCREEN_WIDTH - 100, 43, SCREEN_WIDTH - 30, 45);

    //     rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    //     rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
    //     rdpq_fill_rectangle(SCREEN_WIDTH - 100, 35, SCREEN_WIDTH - 128, 45);

    //     rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    //     rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
    //     rdpq_fill_rectangle(SCREEN_WIDTH - 32, 35, SCREEN_WIDTH - 30, 45);

    //     rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    //     rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
    //     int width = (int)(50 * (sequence3_b_btn_held_duration / 3.0f));
    //     rdpq_fill_rectangle(SCREEN_WIDTH - 100, 35, SCREEN_WIDTH - 100 + width, 45);
    // }

    //////////////////////////////////////////////////////////////
    //                  Pause or Resume Audio                   //
    //////////////////////////////////////////////////////////////

    // TODO: Pause the audio. Or stop it and get it to start at the same spot.
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    // wav64_close(&audio_track);
    // rdpq_text_unregister_font(1);
    // rdpq_font_free(font);
}