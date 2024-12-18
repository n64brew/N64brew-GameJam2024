#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "mallard.h"
#include "introduction/sequence_introduction.h"
#include "game/sequence_game.h"

const MinigameDef minigame_def = {
    .gamename = "Mallard",
    .developername = "Josh Kautz",
    .description = "Don't migrate - fight off winter!",
    .instructions = "A to slap, B to run. Highest score wins!",
};

rdpq_font_t *font_halo_dek;
rdpq_font_t *font_halo_dek_medium;
rdpq_font_t *font_halo_dek_big;
rdpq_font_t *font_celtic_garamond_the_second;

bool sequence_introduction_started = true;
bool sequence_introduction_finished = false;
bool sequence_menu_started = false;
bool sequence_menu_finished = false;
bool sequence_game_started = false;
bool sequence_game_finished = false;

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    ///////////////////////////////////////////////////////////
    //                  Set Fonts                            //
    ///////////////////////////////////////////////////////////

    rdpq_fontstyle_t fontstyle_white = {
        .color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF),
    };
    rdpq_fontstyle_t fontstyle_white_transparent = {
        .color = RGBA32(0xFF, 0xFF, 0xFF, (int)(255.0f * 0.5f)),
    };
    rdpq_fontstyle_t fontstyle_white_outlined = {
        .color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF),
        .outline_color = RGBA32(0x00, 0x00, 0x00, 0xFF),
    };
    rdpq_fontstyle_t fontstyle_player1_outlined = {
        .color = PLAYERCOLOR_1,
        .outline_color = RGBA32(0x00, 0x00, 0x00, 0xFF),
    };
    rdpq_fontstyle_t fontstyle_player2_outlined = {
        .color = PLAYERCOLOR_2,
        .outline_color = RGBA32(0x00, 0x00, 0x00, 0xFF),
    };
    rdpq_fontstyle_t fontstyle_player3_outlined = {
        .color = PLAYERCOLOR_3,
        .outline_color = RGBA32(0x00, 0x00, 0x00, 0xFF),
    };
    rdpq_fontstyle_t fontstyle_player4_outlined = {
        .color = PLAYERCOLOR_4,
        .outline_color = RGBA32(0x00, 0x00, 0x00, 0xFF),
    };

    font_halo_dek = rdpq_font_load("rom:/mallard/HaloDek.font64");
    font_halo_dek_medium = rdpq_font_load("rom:/mallard/HaloDekMedium.font64");
    font_halo_dek_big = rdpq_font_load("rom:/mallard/HaloDekBig.font64");
    font_celtic_garamond_the_second = rdpq_font_load("rom:/mallard/CelticGaramondTheSecond.font64");

    rdpq_font_style(font_halo_dek, 0, &fontstyle_white);
    rdpq_font_style(font_halo_dek, 1, &fontstyle_white_transparent);

    rdpq_font_style(font_halo_dek_medium, 0, &fontstyle_white_outlined);
    rdpq_font_style(font_halo_dek_medium, 1, &fontstyle_player1_outlined);
    rdpq_font_style(font_halo_dek_medium, 2, &fontstyle_player2_outlined);
    rdpq_font_style(font_halo_dek_medium, 3, &fontstyle_player3_outlined);
    rdpq_font_style(font_halo_dek_medium, 4, &fontstyle_player4_outlined);

    rdpq_font_style(font_halo_dek_big, 0, &fontstyle_white_outlined);
    rdpq_font_style(font_halo_dek_big, 1, &fontstyle_player1_outlined);
    rdpq_font_style(font_halo_dek_big, 2, &fontstyle_player2_outlined);
    rdpq_font_style(font_halo_dek_big, 3, &fontstyle_player3_outlined);
    rdpq_font_style(font_halo_dek_big, 4, &fontstyle_player4_outlined);

    rdpq_font_style(font_celtic_garamond_the_second, 0, &fontstyle_white);

    rdpq_text_register_font(FONT_HALODEK, font_halo_dek);
    rdpq_text_register_font(FONT_HALODEK_MEDIUM, font_halo_dek_medium);
    rdpq_text_register_font(FONT_HALODEK_BIG, font_halo_dek_big);
    rdpq_text_register_font(FONT_CELTICGARMONDTHESECOND, font_celtic_garamond_the_second);
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
    if (sequence_introduction_started && !sequence_introduction_finished)
    {
        sequence_introduction(deltatime);
        return;
    }

    if (sequence_game_started && !sequence_game_finished)
    {
        sequence_game(deltatime);
        return;
    }

    minigame_end();
    return;
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    // Unregister the font and free the allocated memory.
    rdpq_text_unregister_font(FONT_HALODEK);
    rdpq_font_free(font_halo_dek);
    rdpq_text_unregister_font(FONT_HALODEK_MEDIUM);
    rdpq_font_free(font_halo_dek_medium);
    rdpq_text_unregister_font(FONT_HALODEK_BIG);
    rdpq_font_free(font_halo_dek_big);
    rdpq_text_unregister_font(FONT_CELTICGARMONDTHESECOND);
    rdpq_font_free(font_celtic_garamond_the_second);
}
