#include <libdragon.h>
#include "sequence_4.h"
#include "sequence_4_input.h"
#include "sequence_4_graphics.h"
#include "../../minigame.h"

#define NUM_DISPLAY 3

///////////////////////////////////////////////////////////
//                  Colors                               //
///////////////////////////////////////////////////////////

#define SEQEUENCE_4_PARARGAPH_01 "$02Hark, ye gamer folk, to the tale of Minneapolis Mallard. That noble band who didst rise like the morning sun to lay waste upon the North Central that fateful weekend in late September."
#define SEQEUENCE_4_PARARGAPH_02 "$02Known not for long years nor honored lineage, yet with spirit ablaze they took siege, keen as the falcon and fierce as the storm."
#define SEQEUENCE_4_PARARGAPH_03 "$02From that day forth, the name Mallard echoed through village and vale, a legend born of victory. Leaving banners fallen and pride humbled, this is the tale of that very day..."

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////
sprite_t *sequence_4_mallard_libdragon_sprite;
sprite_t *sequence_4_mallard_logo_black_sprite;
sprite_t *sequence_4_mallard_logo_white_sprite;
sprite_t *sequence_4_mallard_menu_1_sprite;
sprite_t *sequence_4_mallard_menu_2_sprite;

sprite_t *sequence_4_mallard_background_clouds_front_fc_sprite;
sprite_t *sequence_4_mallard_background_clouds_front_t_fc_sprite;
sprite_t *sequence_4_mallard_background_clouds_mid_fc_sprite;
sprite_t *sequence_4_mallard_background_clouds_mid_t_fc_sprite;
sprite_t *sequence_4_mallard_background_far_mountains_fc_sprite;
sprite_t *sequence_4_mallard_background_grassy_mountains_fc_sprite;
sprite_t *sequence_4_mallard_background_hill_sprite;
sprite_t *sequence_4_mallard_background_sky_fc_sprite;

sprite_t *sequence_4_a_button_sprite;
sprite_t *sequence_4_start_button_sprite;

sprite_t *sequence_4_mallard_idle_sprite;

xm64player_t xm;
int sequence_4_currentXMPattern = 0;

rdpq_font_t *sequence_4_font_pacifico;
rdpq_font_t *sequence_4_font_celtic_garamond_the_second;
rdpq_font_t *sequence_4_font_halo_dek;

float sequence_4_time = 0.0f;
int sequence_4_frame;
bool sequence_4_initialized = false;
bool sequence_4_finished = false;

// Libdragon Logo
bool sequence_4_libdragon_logo_started = false;
bool sequence_4_libdragon_logo_finished = false;
float sequence_4_libdragon_logo_elapsed = 0.0f;

// Mallard Logo
bool sequence_4_mallard_logo_started = false;
bool sequence_4_mallard_logo_finished = false;
float sequence_4_mallard_logo_elapsed = 0.0f;

// Paragraphs
bool sequence_4_paragraphs_started = false;
bool sequence_4_paragraphs_finished = false;
int sequence_4_current_paragraph = 0;
int sequence_4_current_paragraph_speed = 4;
bool sequence_4_current_paragraph_finished = false;
char *sequence_4_current_paragraph_string;
int sequence_4_current_paragraph_drawn_characters = 0;

bool sequence_4_paragraph_fade_out_started = false;
float sequence_4_paragraph_fade_out_elapsed = 0.0f;
bool sequence_4_paragraph_fade_out_finished = false;

// Menu
float sequence_4_menu_fade_in_duration = 0.0f;

void sequence_4_init()
{
    ///////////////////////////////////////////////////////////
    //                  Set up Display                       //
    ///////////////////////////////////////////////////////////

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, NUM_DISPLAY, GAMMA_NONE, FILTERS_RESAMPLE);

    ///////////////////////////////////////////////////////////
    //                  Set Fonts                            //
    ///////////////////////////////////////////////////////////

    rdpq_fontstyle_t fontstyle_white = {
        .color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF), // White
    };

    rdpq_fontstyle_t fontstyle_black = {
        .color = RGBA32(0x00, 0x00, 0x00, 0x00), // Black
    };

    sequence_4_font_pacifico = rdpq_font_load("rom:/mallard/Pacifico.font64");
    sequence_4_font_celtic_garamond_the_second = rdpq_font_load("rom:/mallard/CelticGaramondTheSecond.font64");
    sequence_4_font_halo_dek = rdpq_font_load("rom:/mallard/HaloDek.font64");

    rdpq_font_style(sequence_4_font_halo_dek, 0, &fontstyle_white);
    rdpq_font_style(sequence_4_font_halo_dek, 1, &fontstyle_black);
    rdpq_font_style(sequence_4_font_pacifico, 0, &fontstyle_white);
    rdpq_font_style(sequence_4_font_celtic_garamond_the_second, 0, &fontstyle_white);

    rdpq_text_register_font(FONT_PACIFICO, sequence_4_font_pacifico);
    rdpq_text_register_font(FONT_CELTICGARMONDTHESECOND, sequence_4_font_celtic_garamond_the_second);
    rdpq_text_register_font(FONT_HALODEK, sequence_4_font_halo_dek);

    ///////////////////////////////////////////////////////////
    //                  Set up Graphics                      //
    ///////////////////////////////////////////////////////////

    // Libdragon
    sequence_4_mallard_libdragon_sprite = sprite_load("rom:/mallard/libdragon.rgba32.sprite");

    // Mallard Logo
    sequence_4_mallard_logo_black_sprite = sprite_load("rom:/mallard/mallard_logo_black.rgba32.sprite");
    sequence_4_mallard_logo_white_sprite = sprite_load("rom:/mallard/mallard_logo_white.rgba32.sprite");

    // Intro UI
    sequence_4_start_button_sprite = sprite_load("rom:/core/StartButton.sprite");
    sequence_4_a_button_sprite = sprite_load("rom:/core/AButton.sprite");

    // Menu
    sequence_4_mallard_menu_1_sprite = sprite_load("rom:/mallard/mallard_menu_1.rgba32.sprite");
    sequence_4_mallard_menu_2_sprite = sprite_load("rom:/mallard/mallard_menu_2.rgba32.sprite");

    // Game
    sequence_4_mallard_idle_sprite = sprite_load("rom:/mallard/mallard_idle.rgba32.sprite");

    sequence_4_mallard_background_clouds_front_fc_sprite = sprite_load("rom:/mallard/mallard_background_clouds_front_fc.rgba32.sprite");
    sequence_4_mallard_background_clouds_front_t_fc_sprite = sprite_load("rom:/mallard/mallard_background_clouds_front_t_fc.rgba32.sprite");
    sequence_4_mallard_background_clouds_mid_fc_sprite = sprite_load("rom:/mallard/mallard_background_clouds_mid_fc.rgba32.sprite");
    sequence_4_mallard_background_clouds_mid_t_fc_sprite = sprite_load("rom:/mallard/mallard_background_clouds_mid_t_fc.rgba32.sprite");
    sequence_4_mallard_background_far_mountains_fc_sprite = sprite_load("rom:/mallard/mallard_background_far_mountains_fc.rgba32.sprite");
    sequence_4_mallard_background_grassy_mountains_fc_sprite = sprite_load("rom:/mallard/mallard_background_grassy_mountains_fc.rgba32.sprite");
    sequence_4_mallard_background_hill_sprite = sprite_load("rom:/mallard/mallard_background_hill.rgba32.sprite");
    sequence_4_mallard_background_sky_fc_sprite = sprite_load("rom:/mallard/mallard_background_sky_fc.rgba32.sprite");

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    xm64player_open(&xm, "rom:/mallard/mallard_intro_music.xm64");
    xm64player_play(&xm, 0);
    xm64player_seek(&xm, sequence_4_currentXMPattern, 0, 0);

    sequence_4_frame = 0;
    sequence_4_initialized = true;

    sequence_4_libdragon_logo_started = true;
}

void sequence_4_cleanup()
{
    // Unregister the font and free the allocated memory.
    rdpq_text_unregister_font(FONT_PACIFICO);
    rdpq_font_free(sequence_4_font_pacifico);
    rdpq_text_unregister_font(FONT_CELTICGARMONDTHESECOND);
    rdpq_font_free(sequence_4_font_celtic_garamond_the_second);
    rdpq_text_unregister_font(FONT_HALODEK);
    rdpq_font_free(sequence_4_font_halo_dek);

    // Free the sprites.

    // Libdragon
    sprite_free(sequence_4_mallard_libdragon_sprite);

    // Mallard Logo
    sprite_free(sequence_4_mallard_logo_black_sprite);
    sprite_free(sequence_4_mallard_logo_white_sprite);

    // Intro UI
    sprite_free(sequence_4_start_button_sprite);
    sprite_free(sequence_4_a_button_sprite);

    // Menu
    sprite_free(sequence_4_mallard_menu_1_sprite);
    sprite_free(sequence_4_mallard_menu_2_sprite);

    // Game
    sprite_free(sequence_4_mallard_idle_sprite);

    sprite_free(sequence_4_mallard_background_clouds_front_fc_sprite);
    sprite_free(sequence_4_mallard_background_clouds_front_t_fc_sprite);
    sprite_free(sequence_4_mallard_background_clouds_mid_fc_sprite);
    sprite_free(sequence_4_mallard_background_clouds_mid_t_fc_sprite);
    sprite_free(sequence_4_mallard_background_far_mountains_fc_sprite);
    sprite_free(sequence_4_mallard_background_grassy_mountains_fc_sprite);
    sprite_free(sequence_4_mallard_background_hill_sprite);
    sprite_free(sequence_4_mallard_background_sky_fc_sprite);

    // Stop the music and free the allocated memory.
    xm64player_stop(&xm);
    xm64player_close(&xm);

    // Close the display and free the allocated memory.
    rspq_wait();
    display_close();

    // Reset the state.
    sequence_4_initialized = false;
    sequence_4_finished = false;
    // TODO: Check to make sure that we're resetting the state of a lot of things...

    // End the sequence.
    sequence_4_story = false;
    sequence_5_BLANK = true;
}

///////////////////////////////////////////////////////////
//                  Utility Functions                    //
///////////////////////////////////////////////////////////

int fade_in_alpha(float percentage)
{
    return (int)(percentage * 255.0f);
}

int fade_in_color(float percentage)
{
    return (int)((1.0f - percentage) * 255.0f);
}

// color_t background_color()
// {
//     // White for Mallard Logo.
//     if (sequence_4_time <= DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION)
//     {
//         return WHITE;
//     }

//     // Fade to Black for Paragraph.
//     if (sequence_4_time > DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION &&
//         sequence_4_time <= DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION + DRAW_FADE_WHITE_TO_BLACK_DURATION)
//     {
//         float percentage = (sequence_4_time - (DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION)) / DRAW_FADE_WHITE_TO_BLACK_DURATION;
//         uint8_t color = fade_in_color(percentage);
//         uint8_t alpha = fade_in_alpha(percentage);
//         return RGBA32(color, color, color, alpha);
//     }

//     // Black for Paragraph.
//     return BLACK;
// }

void sequence_4(float deltatime)
{
    sequence_4_frame++;
    sequence_4_time += deltatime;

    sequence_4_process_controller(deltatime);

    if (!sequence_4_initialized)
    {
        sequence_4_init();
    }

    if (sequence_4_finished)
    {
        sequence_4_cleanup();
        return;
    }

    rdpq_attach(display_get(), NULL);
    rdpq_clear(BLACK);

    ///////////////////////////////////////////////////////////
    //                  Intro Sequence                       //
    ///////////////////////////////////////////////////////////
    sequence_4_draw_libdragon_logo(deltatime);
    sequence_4_draw_mallard_logo(deltatime);
    sequence_4_draw_press_a_for_next();
    sequence_4_draw_press_start_to_skip();
    sequence_4_draw_paragraph(deltatime);
    sequence_4_menu(deltatime);

    rdpq_detach_show();

    ///////////////////////////////////////////////////////////
    //                  Handle Audio                         //
    ///////////////////////////////////////////////////////////

    int patidx, row;

    xm64player_tell(&xm, &patidx, &row, NULL);

    // If the pattern index is greater than the currently allowed pattern, loop back to the start of the currently allowed pattern.
    if (patidx > sequence_4_currentXMPattern)
        xm64player_seek(&xm, sequence_4_currentXMPattern, 0, 0);
}