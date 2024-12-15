#include <libdragon.h>
#include "../mallard.h"
#include "sequence_introduction.h"
#include "sequence_introduction_graphics.h"
#include "../../../core.h"
#include "../../../minigame.h"

void sequence_introduction_draw_press_start_to_skip()
{
    if ((sequence_introduction_libdragon_logo_started == true && sequence_introduction_libdragon_logo_finished == false) ||
        (sequence_introduction_mallard_logo_started == true && sequence_introduction_mallard_logo_finished == false) ||
        (sequence_introduction_paragraphs_started == true && sequence_introduction_paragraphs_finished == false))
    {
        // Draw "Start" button
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_sprite_blit(sequence_introduction_start_button_sprite,
                         RESOLUTION_320x240.width - sequence_introduction_start_button_sprite->width - 32,
                         RESOLUTION_320x240.height - sequence_introduction_start_button_sprite->height - 1,
                         NULL);

        // Draw "Skip" text
        float x = RESOLUTION_320x240.width - 30;
        float y = RESOLUTION_320x240.height - 5;
        rdpq_text_print(NULL, FONT_HALODEK, x, y, "$01^00Skip");
        rdpq_mode_pop();
    }
}

void sequence_introduction_draw_press_a_for_next()
{
    if ((sequence_introduction_libdragon_logo_started == true && sequence_introduction_libdragon_logo_finished == false) ||
        (sequence_introduction_mallard_logo_started == true && sequence_introduction_mallard_logo_finished == false) ||
        (sequence_introduction_paragraphs_started == true && sequence_introduction_paragraphs_finished == false))
    {
        // Draw "A" button
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_sprite_blit(sequence_introduction_a_button_sprite,
                         RESOLUTION_320x240.width - sequence_introduction_a_button_sprite->width - 32,
                         RESOLUTION_320x240.height - sequence_introduction_a_button_sprite->height - sequence_introduction_start_button_sprite->height - 2,
                         NULL);

        // Draw "Next" text
        float x = RESOLUTION_320x240.width - 30;
        float y = RESOLUTION_320x240.height - sequence_introduction_start_button_sprite->height - 6;
        rdpq_text_print(NULL, FONT_HALODEK, x, y, "$01^00Next");
        rdpq_mode_pop();
    }
}

void sequence_introduction_draw_mallard_logo(float deltatime)
{
    if (sequence_introduction_mallard_logo_started == true && sequence_introduction_mallard_logo_finished == false)
    {
        float scale = 0.75f + 0.10 * sequence_introduction_mallard_logo_elapsed / (DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION);
        rdpq_blitparms_t blitparms = {
            .scale_x = scale,
            .scale_y = scale,
        };
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_alphacompare(1);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
        if (sequence_introduction_mallard_logo_elapsed < DRAW_MALLARD_LOGO_FADE_IN_DURATION)
            rdpq_set_fog_color(RGBA32(0, 0, 0, (int)((sequence_introduction_mallard_logo_elapsed / DRAW_MALLARD_LOGO_FADE_IN_DURATION) * 255.0f)));
        if (sequence_introduction_mallard_logo_elapsed >= DRAW_MALLARD_LOGO_FADE_IN_DURATION && sequence_introduction_mallard_logo_elapsed < DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION)
            rdpq_set_fog_color(RGBA32(0, 0, 0, 255));
        if (sequence_introduction_mallard_logo_elapsed >= (DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION))
            rdpq_set_fog_color(RGBA32(0, 0, 0, (int)((1.0f - ((sequence_introduction_mallard_logo_elapsed - DRAW_MALLARD_LOGO_FADE_IN_DURATION - DRAW_MALLARD_LOGO_DURATION) / DRAW_MALLARD_LOGO_FADE_OUT_DURATION)) * 255.0f)));
        rdpq_sprite_blit(sequence_introduction_mallard_logo_white_sprite,
                         RESOLUTION_320x240.width / 2 - sequence_introduction_mallard_logo_white_sprite->width * scale / 2,
                         RESOLUTION_320x240.height / 2 - sequence_introduction_mallard_logo_white_sprite->height * scale / 2,
                         &blitparms);
        rdpq_mode_pop();

        sequence_introduction_mallard_logo_elapsed += deltatime;

        if (sequence_introduction_mallard_logo_elapsed > DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION)
        {
            sequence_introduction_mallard_logo_finished = true;
            sequence_introduction_paragraphs_started = true;
        }
    }
}

void sequence_introduction_draw_libdragon_logo(float deltatime)
{
    if (sequence_introduction_libdragon_logo_started == true && sequence_introduction_libdragon_logo_finished == false)
    {
        // Draw "Made with Libdragon" text
        float x = RESOLUTION_320x240.width / 2 - 37;
        float y = RESOLUTION_320x240.height / 2 - 72;
        rdpq_set_mode_standard();
        rdpq_text_print(&(rdpq_textparms_t){.char_spacing = 2}, FONT_HALODEK, x, y, "$01^00Made with");

        // Draw the Libdragon logo
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_sprite_blit(sequence_introduction_mallard_libdragon_sprite, RESOLUTION_320x240.width / 2 - sequence_introduction_mallard_libdragon_sprite->width / 2, RESOLUTION_320x240.height / 2 - sequence_introduction_mallard_libdragon_sprite->height / 2, NULL);
        rdpq_mode_pop();

        sequence_introduction_libdragon_logo_elapsed += deltatime;

        if (sequence_introduction_libdragon_logo_elapsed > DRAW_LIBDRAGON_LOGO_DURATION)
        {
            sequence_introduction_libdragon_logo_finished = true;
            sequence_introduction_mallard_logo_started = true;
        }
    }
}

void sequence_introduction_draw_paragraph(float deltatime)
{
    if (sequence_introduction_paragraphs_started == true && sequence_introduction_paragraphs_finished == false)
    {
        // BYPASS THE PARAGRAPHS
        sequence_introduction_paragraphs_finished = true;
        sequence_introduction_should_cleanup = true;
        return;

        int total_chars = 0;
        switch (sequence_introduction_current_paragraph)
        {
        case 0:
            sequence_introduction_current_paragraph_string = SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_01;
            total_chars = strlen(SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_01);
            break;
        case 1:
            sequence_introduction_current_paragraph_string = SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_02;
            total_chars = strlen(SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_02);
            break;
        case 2:
            sequence_introduction_current_paragraph_string = SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_03;
            total_chars = strlen(SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_03);
            break;
        default:
            sequence_introduction_current_paragraph_string = SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_03;
            total_chars = strlen(SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_03);
            sequence_introduction_paragraph_fade_out_started = true;
            break;
        }

        if (sequence_introduction_paragraph_fade_out_started == false)
        {
            if ((sequence_introduction_frame % sequence_introduction_current_paragraph_speed) == 0)
            {
                sequence_introduction_current_paragraph_drawn_characters++;
                if (sequence_introduction_current_paragraph_drawn_characters > total_chars)
                {
                    sequence_introduction_current_paragraph_drawn_characters = total_chars;
                    sequence_introduction_current_paragraph_finished = true;
                }
            }
        }
        else
        {
            sequence_introduction_current_paragraph_drawn_characters = total_chars;
        }

        int x_margin = 5;
        int y_margin = 0;
        rdpq_textparms_t params = {
            .align = ALIGN_CENTER,
            .valign = VALIGN_CENTER,
            .width = RESOLUTION_320x240.width - (2 * x_margin),
            .height = RESOLUTION_320x240.height - (2 * y_margin),
            .wrap = WRAP_WORD,
        };

        rdpq_paragraph_t *par = rdpq_paragraph_build(&params, FONT_CELTICGARMONDTHESECOND, sequence_introduction_current_paragraph_string, &sequence_introduction_current_paragraph_drawn_characters);
        rdpq_paragraph_render(par, x_margin, y_margin);
        rdpq_paragraph_free(par);

        if (sequence_introduction_paragraph_fade_out_started == true && sequence_introduction_paragraph_fade_out_finished == false)
        {
            xm64player_set_vol(&sequence_introduction_xm, 1.0f - (sequence_introduction_paragraph_fade_out_elapsed / PARAGRAPH_FADE_OUT_DURATION));
            if (sequence_introduction_paragraph_fade_out_elapsed > PARAGRAPH_FADE_OUT_DURATION)
            {
                sequence_introduction_paragraph_fade_out_finished = true;
                sequence_introduction_paragraphs_finished = true;
                sequence_introduction_should_cleanup = true;
            }

            rdpq_mode_push();
            uint8_t alpha = (int)(255.0f * (sequence_introduction_paragraph_fade_out_elapsed / PARAGRAPH_FADE_OUT_DURATION));
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_set_prim_color(RGBA32(0, 0, 0, alpha));
            rdpq_fill_rectangle(0, 0, RESOLUTION_320x240.width, RESOLUTION_320x240.height);
            rdpq_mode_pop();
            sequence_introduction_paragraph_fade_out_elapsed += deltatime;
        }
    }
}