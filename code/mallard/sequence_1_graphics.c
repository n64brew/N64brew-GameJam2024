#include <libdragon.h>
#include "sequence_1_graphics.h"

void sequence_1_draw_press_start_to_skip()
{
    // Draw "to Skip" text
    float x = RESOLUTION_320x240.width - 36;
    float y = RESOLUTION_320x240.height - 5;
    rdpq_set_mode_standard();
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, x, y, "to Skip");

    // Draw the Start button
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_1_sprite_start_button,
                     RESOLUTION_320x240.width - sequence_1_sprite_start_button->width - 38,
                     RESOLUTION_320x240.height - sequence_1_sprite_start_button->height - 1,
                     &(rdpq_blitparms_t){
                         .scale_x = 1.0f,
                         .scale_y = 1.0f,
                     });
}