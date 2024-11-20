#include <libdragon.h>
#include <stdlib.h>
#include "mallard.h"
#include "sequence_game.h"
#include "sequence_game_graphics.h"
#include "../../core.h"
#include "../../minigame.h"

float sequence_game_fade_in_elapsed = 0.0f;
float sequence_game_start_held_elapsed = 0.0f;

void sequence_game_draw_mallard_idle_sprite()
{
    int sequence_game_mallard_idle_frame = (sequence_game_frame >> 3) % SEQUENCE_GAME_MALLARD_IDLE_FRAMES;
    rdpq_blitparms_t blitparms = {
        .s0 = sequence_game_mallard_idle_frame * 32,
        .t0 = 0,
        .width = 32,
        .height = 32,
        .flip_x = true,
    };
    rdpq_mode_push();
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_game_mallard_idle_sprite,
                     RESOLUTION_320x240.width / 2 - sequence_game_mallard_idle_sprite->width / 2,
                     RESOLUTION_320x240.height - sequence_game_mallard_idle_sprite->height - 30,
                     &blitparms);
    rdpq_mode_pop();
}

void sequence_game_draw_background_lakeview_terrace()
{
    rdpq_mode_push();
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_game_background_lakeview_terrace_sprite,
                     0,
                     0,
                     NULL);
    rdpq_mode_pop();
}

void sequence_game_draw_press_start_to_pause()
{
    if (sequence_game_started == true && sequence_game_finished == false)
    {
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_alphacompare(1);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
        rdpq_set_fog_color(RGBA32(0, 0, 0, (int)(255.0f * 0.5f)));
        // Draw "Start" button
        rdpq_sprite_blit(sequence_game_start_button_sprite,
                         RESOLUTION_320x240.width - sequence_game_start_button_sprite->width - 36,
                         RESOLUTION_320x240.height - sequence_game_start_button_sprite->height - 1,
                         NULL);
        rdpq_mode_pop();

        // Draw "Pause" text
        rdpq_text_print(NULL, FONT_HALODEK, RESOLUTION_320x240.width - 34, RESOLUTION_320x240.height - 5, "$01^01Pause");
    }
}

void sequence_game_draw_paused()
{
    if (sequence_game_paused == true)
    {
        float x = powf(sequence_game_start_held_elapsed,3) * ((((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f);
        float y = powf(sequence_game_start_held_elapsed,3) * ((((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f);

        float percentage = sequence_game_start_held_elapsed / GAME_EXIT_DURATION > 1.0 ? 1.0 : sequence_game_start_held_elapsed / GAME_EXIT_DURATION;
        int width_red = sequence_game_paused_text_sprite->width * percentage;
        int width_white = sequence_game_paused_text_sprite->width - width_red;

        if (width_red > 0)
        {
            // RED
            rdpq_set_mode_standard();
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_mode_combiner(RDPQ_COMBINER1((PRIM, ENV, TEX0, ENV), (0, 0, 0, TEX0)));
            rdpq_set_prim_color(RED);  // fill color
            rdpq_set_env_color(BLACK); // outline color
            rdpq_sprite_blit(sequence_game_paused_text_sprite,
                             160 - sequence_game_paused_text_sprite->width / 2 + x,
                             120 - sequence_game_paused_text_sprite->height / 2 + y,
                             &(rdpq_blitparms_t){
                                 .width = width_red,
                             });
        }

        // WHITE
        if (width_white > 0)
        {
            rdpq_set_mode_standard();
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_sprite_blit(sequence_game_paused_text_sprite,
                             160 - sequence_game_paused_text_sprite->width / 2 + x + width_red,
                             120 - sequence_game_paused_text_sprite->height / 2 + y,
                             &(rdpq_blitparms_t){
                                 .s0 = width_red,
                                 .width = width_white,
                             });
        }
    }
}

void sequence_game_render(float deltatime)
{
    if (sequence_game_started == true && sequence_game_finished == false)
    {
        sequence_game_draw_background_lakeview_terrace();
        sequence_game_draw_mallard_idle_sprite();
        sequence_game_draw_press_start_to_pause();
        sequence_game_draw_paused();

        if (sequence_game_fade_in_elapsed < GAME_FADE_IN_DURATION)
        {
            float percentage = sequence_game_fade_in_elapsed > GAME_FADE_IN_DURATION ? 1.0f : sequence_game_fade_in_elapsed / GAME_FADE_IN_DURATION;
            uint8_t alpha = (int)(255.0f * (1.0f - percentage));
            rdpq_mode_push();
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_set_prim_color(RGBA32(0, 0, 0, alpha));
            rdpq_fill_rectangle(0, 0, RESOLUTION_320x240.width, RESOLUTION_320x240.height);
            rdpq_mode_pop();
        }

        sequence_game_fade_in_elapsed += deltatime;
    }
}