#include <libdragon.h>
#include "sequence_4.h"
#include "sequence_4_graphics.h"

#define SEQUENCE_4_MALLARD_IDLE_FRAMES 4

void sequence_4_draw_press_start_to_skip()
{
    // Draw "Start" button
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_4_start_button_sprite,
                     RESOLUTION_320x240.width - sequence_4_start_button_sprite->width - 46,
                     RESOLUTION_320x240.height - sequence_4_start_button_sprite->height - 1,
                     &(rdpq_blitparms_t){
                         .scale_x = 1.0f,
                         .scale_y = 1.0f,
                     });

    // Draw "to Skip" text
    float x = RESOLUTION_320x240.width - 44;
    float y = RESOLUTION_320x240.height - 5;
    rdpq_text_printf(NULL, FONT_HALODEK, x, y, "to Skip");
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
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_4_mallard_idle_sprite, 0, 0, &blitparms);
}