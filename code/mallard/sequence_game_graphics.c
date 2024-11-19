#include <libdragon.h>
#include "sequence_game.h"
#include "sequence_game_graphics.h"
#include "../../core.h"
#include "../../minigame.h"

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

void sequence_game_render(float deltatime)
{
    if (sequence_game_started == true && sequence_game_finished == false)
    {
        sequence_game_draw_background_lakeview_terrace();
        sequence_game_draw_mallard_idle_sprite();

        if (sequence_game_fade_in_duration < GAME_FADE_IN_DURATION)
        {
            float percentage = sequence_game_fade_in_duration > GAME_FADE_IN_DURATION ? 1.0f : sequence_game_fade_in_duration / GAME_FADE_IN_DURATION;
            uint8_t alpha = (int)(255.0f * (1.0f - percentage));
            rdpq_mode_push();
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_set_prim_color(RGBA32(0, 0, 0, alpha));
            rdpq_fill_rectangle(0, 0, RESOLUTION_320x240.width, RESOLUTION_320x240.height);
            rdpq_mode_pop();
        }

        sequence_game_fade_in_duration += deltatime;
    }
}