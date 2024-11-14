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

sprite_t *sequence_4_start_button_sprite;

sprite_t *sequence_4_mallard_idle_sprite;

rdpq_font_t *sequence_4_font_pacifico;
rdpq_font_t *sequence_4_font_celtic_garamond_the_second;
rdpq_font_t *sequence_4_font_halo_dek;

int sequence_4_frame;
bool sequence_4_initialized = false;
bool sequence_4_finished = false;
int sequence_4_drawn_characters = 0;
float sequence_4_paragraph_delay_duration = 0.0f;

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
    //                  Set up UI Elements                   //
    ///////////////////////////////////////////////////////////

    sequence_4_start_button_sprite = sprite_load("rom:/core/StartButton.sprite");

    ///////////////////////////////////////////////////////////
    //                  Set up Game Elements                 //
    ///////////////////////////////////////////////////////////

    sequence_4_mallard_idle_sprite = sprite_load("rom:/mallard/mallard_idle.rgba32.sprite");

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

    // Close the display and free the allocated memory.
    rspq_wait();
    display_close();

    // Reset the state.
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

void sequence_4(float deltatime)
{
    sequence_4_frame++;
    sequence_4_paragraph_delay_duration += deltatime;

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

    surface_t *disp = display_get();

    rdpq_attach(disp, NULL);

    float percentage = sequence_4_paragraph_delay_duration > 2.0f ? 1.0f : sequence_4_paragraph_delay_duration / 2.0f;
    rdpq_clear(
        RGBA32(
            fade_in_color(percentage),
            fade_in_color(percentage),
            fade_in_color(percentage),
            fade_in_alpha(percentage)));

    ///////////////////////////////////////////////////////////
    //                  Draw UI Elements                     //
    ///////////////////////////////////////////////////////////

    sequence_4_draw_press_start_to_skip();
    sequence_4_draw_mallard_idle_sprite();

    ///////////////////////////////////////////////////////////
    //                  Draw Paragraph                       //
    ///////////////////////////////////////////////////////////

    int total_chars = strlen(SEQEUENCE_4_PARARGAPH_01);

    // Delay the drawing of the paragraph by 2 seconds.
    if (sequence_4_paragraph_delay_duration > 2.0f)
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
        .width = disp->width - (2 * x_margin),
        .height = disp->height - (2 * y_margin),
        .wrap = WRAP_WORD,
    };

    // rdpq_set_mode_standard();
    // rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_paragraph_t *par = rdpq_paragraph_build(&params, FONT_CELTICGARMONDTHESECOND, SEQEUENCE_4_PARARGAPH_01, &sequence_4_drawn_characters);
    rdpq_paragraph_render(par, x_margin, y_margin);
    rdpq_paragraph_free(par);

    rdpq_detach_show();
}