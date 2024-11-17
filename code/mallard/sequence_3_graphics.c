#include <libdragon.h>
#include "sequence_3_graphics.h"

#define SCREEN_WIDTH 288
#define SCREEN_HEIGHT 208

void sequence_3_draw_press_start_to_skip()
{
    // Draw "Skip" text
    float x = SCREEN_WIDTH - 36;
    float y = SCREEN_HEIGHT - 5;
    rdpq_set_mode_standard();
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, x, y, "Skip");

    // Draw the Start button
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_3_sprite_start_button,
                     SCREEN_WIDTH - sequence_3_sprite_start_button->width - 38,
                     SCREEN_HEIGHT - sequence_3_sprite_start_button->height - 1,
                     &(rdpq_blitparms_t){
                         .scale_x = 1.0f,
                         .scale_y = 1.0f,
                     });
}