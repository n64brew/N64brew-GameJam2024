#include <libdragon.h>
#include "sequence_4.h"
#include "sequence_4_graphics.h"
#include "../../core.h"
#include "../../minigame.h"

#define SEQUENCE_4_MALLARD_IDLE_FRAMES 4

void sequence_4_draw_press_start_to_skip()
{
    if (sequence_4_time > DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION + DRAW_FADE_WHITE_TO_BLACK_DURATION)
    {
        // Draw "Start" button
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_sprite_blit(sequence_4_start_button_sprite,
                         RESOLUTION_320x240.width - sequence_4_start_button_sprite->width - 46,
                         RESOLUTION_320x240.height - sequence_4_start_button_sprite->height - 1,
                         NULL);

        // Draw "Skip" text
        float x = RESOLUTION_320x240.width - 44;
        float y = RESOLUTION_320x240.height - 5;
        rdpq_text_printf(NULL, FONT_HALODEK, x, y, "Skip");
        rdpq_mode_pop();
    }
}

void sequence_4_draw_mallard_idle_sprite()
{
    int sequence_4_mallard_idle_frame = (sequence_4_frame >> 3) % SEQUENCE_4_MALLARD_IDLE_FRAMES;
    rdpq_blitparms_t blitparms = {
        .s0 = sequence_4_mallard_idle_frame * 32,
        .t0 = 0,
        .width = 32,
        .height = 32,
        .flip_x = true,
    };
    rdpq_mode_push();
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_4_mallard_idle_sprite, 0, 0, &blitparms);
    rdpq_mode_pop();
}

void sequence_4_draw_mallard_logo()
{
    if (sequence_4_time < DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION)
    {
        float scale = 0.75f + 0.10 * sequence_4_time / (DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION);
        rdpq_blitparms_t blitparms = {
            .scale_x = scale,
            .scale_y = scale,
        };
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_alphacompare(1);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
        if (sequence_4_time < DRAW_MALLARD_LOGO_FADE_IN_DURATION)
            rdpq_set_fog_color(RGBA32(0, 0, 0, sequence_4_time / DRAW_MALLARD_LOGO_FADE_IN_DURATION * 255));
        if (sequence_4_time >= DRAW_MALLARD_LOGO_FADE_IN_DURATION && sequence_4_time < (DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION))
            rdpq_set_fog_color(RGBA32(0, 0, 0, 255));
        if (sequence_4_time >= (DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION))
            rdpq_set_fog_color(RGBA32(0, 0, 0, (1 - ((sequence_4_time - DRAW_MALLARD_LOGO_FADE_IN_DURATION - DRAW_MALLARD_LOGO_DURATION) / DRAW_MALLARD_LOGO_FADE_OUT_DURATION) * 255)));
        rdpq_sprite_blit(sequence_4_mallard_logo_black_sprite,
                         RESOLUTION_320x240.width / 2 - sequence_4_mallard_logo_black_sprite->width * scale / 2,
                         RESOLUTION_320x240.height / 2 - sequence_4_mallard_logo_black_sprite->height * scale / 2,
                         &blitparms);
        rdpq_mode_pop();
    }
}

void sequence_4_draw_paragraph()
{
    if (sequence_4_time > DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION + DRAW_FADE_WHITE_TO_BLACK_DURATION && sequence_4_all_paragraphs_finished == false)
    {
        int total_chars = 0;
        switch (sequence_4_current_paragraph)
        {
        case 0:
            sequence_4_current_paragraph_string = SEQEUENCE_4_GAMEJAM_PARARGAPH_01;
            total_chars = strlen(SEQEUENCE_4_GAMEJAM_PARARGAPH_01);
            break;
        case 1:
            sequence_4_current_paragraph_string = SEQEUENCE_4_GAMEJAM_PARARGAPH_02;
            total_chars = strlen(SEQEUENCE_4_GAMEJAM_PARARGAPH_02);
            break;
        case 2:
            sequence_4_current_paragraph_string = SEQEUENCE_4_GAMEJAM_PARARGAPH_03;
            total_chars = strlen(SEQEUENCE_4_GAMEJAM_PARARGAPH_03);
            break;
        default:
            sequence_4_all_paragraphs_finished = true;
            break;
        }

        if ((sequence_4_frame % sequence_4_paragraph_speed) == 0)
        {
            sequence_4_drawn_characters++;
            if (sequence_4_drawn_characters > total_chars)
            {
                sequence_4_drawn_characters = total_chars;
                sequence_4_current_paragraph_finished = true;
            }
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

        rdpq_paragraph_t *par = rdpq_paragraph_build(&params, FONT_CELTICGARMONDTHESECOND, sequence_4_current_paragraph_string, &sequence_4_drawn_characters);
        rdpq_paragraph_render(par, x_margin, y_margin);
        rdpq_paragraph_free(par);
    }
}