#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include "sequence_1.h"
#include "sequence_2.h"
#include "sequence_3.h"

#include <stdio.h>
#include <unistd.h>

const MinigameDef minigame_def = {
    .gamename = "Mallard 64",
    .developername = "Josh Kautz",
    .description = "",
    .instructions = "",
};

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
    //     rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, SCREEN_WIDTH - 100, 30, "Quitting in %.2f", 3.0f - sequence_3_b_btn_held_duration);

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
    //     int width = (int)(50 * (sequence_3_b_btn_held_duration / 3.0f));
    //     rdpq_fill_rectangle(SCREEN_WIDTH - 100, 35, SCREEN_WIDTH - 100 + width, 45);
    // }
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    // rdpq_text_unregister_font(1);
    // rdpq_font_free(font);
}