#ifndef SEQUENCE_4_GRAPHICS_H
#define SEQUENCE_4_GRAPHICS_H

#include <libdragon.h>

extern int sequence_4_frame;
extern sprite_t *sequence_4_mallard_idle_sprite;
extern sprite_t *sequence_4_start_button_sprite;

void sequence_4_draw_press_start_to_skip();
void sequence_4_draw_mallard_idle_sprite();

#endif // SEQUENCE_4_GRAPHICS_H
