#include <libdragon.h>
#include "sequence_4.h"
#include "sequence_4_input.h"
#include "sequence_4_graphics.h"
#include "../../minigame.h"

#define NUM_DISPLAY 3

///////////////////////////////////////////////////////////
//                  Colors                               //
///////////////////////////////////////////////////////////

#define BLACK RGBA32(0x00, 0x00, 0x00, 0xFF)
#define WHITE RGBA32(0xFF, 0xFF, 0xFF, 0xFF)
#define ASH_GRAY RGBA32(0xAD, 0xBA, 0xBD, 0xFF)
#define MAYA_BLUE RGBA32(0x6C, 0xBE, 0xED, 0xFF)
#define GUN_METAL RGBA32(0x31, 0x39, 0x3C, 0xFF)
#define REDWOOD RGBA32(0xB2, 0x3A, 0x7A, 0xFF)
#define BREWFONT RGBA32(242, 209, 155, 0xFF)

#define SEQEUENCE_4_PARARGAPH_01 "$02Hark, ye gamer folk, to the tale of Minneapolis Mallard. That noble band who didst rise like the morning sun to lay waste upon the North Central that fateful weekend in late September."
#define SEQEUENCE_4_PARARGAPH_02 "$02Known not for long years nor honored lineage, yet with spirit ablaze they took siege, keen as the falcon and fierce as the storm."
#define SEQEUENCE_4_PARARGAPH_03 "$02From that day forth, the name Mallard echoed through village and vale, a legend born of victory. Leaving banners fallen and pride humbled, this is the tale of that very day..."

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////
sprite_t *sequence_4_mallard_logo_black_sprite;

sprite_t *sequence_4_start_button_sprite;

sprite_t *sequence_4_mallard_idle_sprite;

xm64player_t xm;
// Chunk 1: Loop 4
// Chunk 2: Loop 14
// Chunk 3: Loop 18
// Chunk 4: Loop 23
// Chunk 5: Loop 30
// Chunk 6: End on 31
int sequence_4_currentXMPattern = 4;

rdpq_font_t *sequence_4_font_pacifico;
rdpq_font_t *sequence_4_font_celtic_garamond_the_second;
rdpq_font_t *sequence_4_font_halo_dek;

float sequence_4_time = 0.0f;
int sequence_4_frame;
bool sequence_4_initialized = false;
bool sequence_4_finished = false;
int sequence_4_drawn_characters = 0;

float sequence_4_time_to_fade_in_mallard_logo = 1.0f;
float sequence_4_time_to_show_mallard_logo = 2.0f;
float sequence_4_time_to_fade_out_mallard_logo = 1.0f;

float sequence_4_time_to_fade_in_paragraph = 1.0f;

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
        .color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF),         // White
        .outline_color = RGBA32(0x00, 0x00, 0x00, 0x00), // None
    };

    rdpq_fontstyle_t fontstyle_white_outlined = {
        .color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF),         // White
        .outline_color = RGBA32(0x00, 0x00, 0x00, 0xFF), // Black
    };

    sequence_4_font_pacifico = rdpq_font_load("rom:/mallard/Pacifico.font64");
    sequence_4_font_celtic_garamond_the_second = rdpq_font_load("rom:/mallard/CelticGaramondTheSecond.font64");
    sequence_4_font_halo_dek = rdpq_font_load("rom:/mallard/HaloDek.font64");

    rdpq_font_style(sequence_4_font_pacifico, 0, &fontstyle_white);
    rdpq_font_style(sequence_4_font_celtic_garamond_the_second, 0, &fontstyle_white);
    rdpq_font_style(sequence_4_font_halo_dek, 0, &fontstyle_white_outlined);

    rdpq_text_register_font(FONT_PACIFICO, sequence_4_font_pacifico);
    rdpq_text_register_font(FONT_CELTICGARMONDTHESECOND, sequence_4_font_celtic_garamond_the_second);
    rdpq_text_register_font(FONT_HALODEK, sequence_4_font_halo_dek);

    ///////////////////////////////////////////////////////////
    //                  Set up Graphics                      //
    ///////////////////////////////////////////////////////////

    sequence_4_mallard_logo_black_sprite = sprite_load("rom:/mallard/mallard_logo_black.rgba32.sprite");

    ///////////////////////////////////////////////////////////
    //                  Set up UI Elements                   //
    ///////////////////////////////////////////////////////////

    sequence_4_start_button_sprite = sprite_load("rom:/core/StartButton.sprite");

    ///////////////////////////////////////////////////////////
    //                  Set up Game Elements                 //
    ///////////////////////////////////////////////////////////

    sequence_4_mallard_idle_sprite = sprite_load("rom:/mallard/mallard_idle.rgba32.sprite");

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    xm64player_open(&xm, "rom:/mallard/mallard_intro_music.xm64");
    xm64player_play(&xm, 0);
    xm64player_seek(&xm, sequence_4_currentXMPattern, 0, 0);

    sequence_4_frame = 0;
    sequence_4_initialized = true;
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
    sprite_free(sequence_4_start_button_sprite);
    sprite_free(sequence_4_mallard_idle_sprite);

    // Stop the music and free the allocated memory.
    xm64player_stop(&xm);
    xm64player_close(&xm);

    // Close the display and free the allocated memory.
    rspq_wait();
    display_close();

    // Reset the state.
    // TODO: Check to make sure that we're resetting the state of a lot of things...
    sequence_4_initialized = false;
    sequence_4_finished = false;

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

color_t background_color()
{
    // Time: 0.0 - 3.0 = White for mallard logo
    // Time: 3.0 - 4.0 = Fade to Black for paragraph.
    // Time: 4.0 - ??? = Black for paragraph.
    if (sequence_4_time < 3.0f)
    {
        return WHITE;
    }
    else if (sequence_4_time >= 3.0f && sequence_4_time <= 4.0f)
    {
        float percentage = (sequence_4_time - 3.0f) / 1.0f;
        uint8_t color = fade_in_color(percentage);
        uint8_t alpha = fade_in_alpha(percentage);
        return RGBA32(color, color, color, alpha);
    }

    return BLACK;
}

void draw_paragraph()
{
    int total_chars = strlen(SEQEUENCE_4_PARARGAPH_01);

    // Delay the drawing of the paragraph by 5 seconds.
    if (sequence_4_time > 5.0f)
    {
        // Draw the paragraph one character at a time. Modulo is used to slow down the drawing.
        if ((sequence_4_frame % 4) == 0)
        {
            sequence_4_drawn_characters++;
            if (sequence_4_drawn_characters > total_chars)
            {
                sequence_4_drawn_characters = total_chars;
            }
        }
    }

    int x_margin = 5;
    int y_margin = 0;
    rdpq_textparms_t params = {
        // .line_spacing = -3,
        .align = ALIGN_CENTER,
        .valign = VALIGN_CENTER,
        .width = RESOLUTION_320x240.width - (2 * x_margin),
        .height = RESOLUTION_320x240.height - (2 * y_margin),
        .wrap = WRAP_WORD,
    };

    // rdpq_set_mode_standard();
    // rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_paragraph_t *par = rdpq_paragraph_build(&params, FONT_CELTICGARMONDTHESECOND, SEQEUENCE_4_PARARGAPH_01, &sequence_4_drawn_characters);
    rdpq_paragraph_render(par, x_margin, y_margin);
    rdpq_paragraph_free(par);
}

void draw_mallard_logo()
{
    // if (sequence_4_time < 1.0f)
    // {
    //     // TODO: Fade in the mallard logo.
    //     rdpq_set_mode_standard();
    //     rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    //     rdpq_sprite_blit(sequence_4_mallard_logo_black_sprite,
    //                      RESOLUTION_320x240.width / 2 - sequence_4_mallard_logo_black_sprite->width / 2,
    //                      RESOLUTION_320x240.height / 2 - sequence_4_mallard_logo_black_sprite->height / 2,
    //                      &(rdpq_blitparms_t){
    //                          .scale_x = 1.0f,
    //                          .scale_y = 1.0f,
    //                      });
    // }
    if (sequence_4_time < 3.0f)
    {
        float percentage = sequence_4_time / 3.0f;
        float scale = 0.75f + 0.10 * percentage;
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_sprite_blit(sequence_4_mallard_logo_black_sprite,
                         RESOLUTION_320x240.width / 2 - sequence_4_mallard_logo_black_sprite->width * scale / 2,   // TODO: Center the logo.
                         RESOLUTION_320x240.height / 2 - sequence_4_mallard_logo_black_sprite->height * scale / 2, // TODO: Center the logo.
                         &(rdpq_blitparms_t){
                             .scale_x = scale,
                             .scale_y = scale,
                         });
    }
    // else if (sequence_4_time > 2.0f && sequence_4_time < 3.0f)
    // {
    //     // TODO: Fade out the mallard logo.
    //     rdpq_set_mode_standard();
    //     rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    //     rdpq_sprite_blit(sequence_4_mallard_logo_black_sprite,
    //                      RESOLUTION_320x240.width / 2 - sequence_4_mallard_logo_black_sprite->width / 2,
    //                      RESOLUTION_320x240.height / 2 - sequence_4_mallard_logo_black_sprite->height / 2,
    //                      &(rdpq_blitparms_t){
    //                          .scale_x = 1.0f,
    //                          .scale_y = 1.0f,
    //                      });
    // }
}

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
    rdpq_clear(background_color());

    draw_mallard_logo();

    ///////////////////////////////////////////////////////////
    //                  Draw UI Elements                     //
    ///////////////////////////////////////////////////////////

    sequence_4_draw_press_start_to_skip();
    // sequence_4_draw_mallard_idle_sprite();

    ///////////////////////////////////////////////////////////
    //                  Draw Paragraph                       //
    ///////////////////////////////////////////////////////////

    draw_paragraph();
    rdpq_detach_show();

    ///////////////////////////////////////////////////////////
    //                  Handle Audio                         //
    ///////////////////////////////////////////////////////////

    int patidx, row;

    xm64player_tell(&xm, &patidx, &row, NULL);

    fprintf(stderr, "Patidx: %d, Row: %d\n", patidx, row);

    // If the pattern index is greater than the currently allowed pattern, loop back to the start of the currently allowed pattern.
    if (patidx > sequence_4_currentXMPattern)
        xm64player_seek(&xm, sequence_4_currentXMPattern, 0, 0);
}