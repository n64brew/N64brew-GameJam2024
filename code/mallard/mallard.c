#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include "sequence_4.h"

#include <stdio.h>
#include <unistd.h>

const MinigameDef minigame_def = {
    .gamename = "Mallard",
    .developername = "Josh Kautz",
    .description = "",
    .instructions = "",
};

bool sequence_introduction_started = true;
bool sequence_introduction_finished = false;
bool sequence_menu_started = false;
bool sequence_menu_finished = false;

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
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
    if (sequence_introduction_started == true && sequence_introduction_finished == false)
    {
        sequence_introduction(deltatime);
        return;
    }

    minigame_end();
    return;

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
}
