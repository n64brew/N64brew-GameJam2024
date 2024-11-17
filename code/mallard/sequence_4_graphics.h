#ifndef SEQUENCE_4_GRAPHICS_H
#define SEQUENCE_4_GRAPHICS_H

#include <libdragon.h>

extern float sequence_4_time;
extern int sequence_4_frame;
extern int sequence_4_drawn_characters;
extern int sequence_4_current_paragraph;
extern int sequence_4_paragraph_speed;
extern bool sequence_4_all_paragraphs_finished;
extern bool sequence_4_current_paragraph_finished;
extern bool sequence_4_paragraph_fade_out_started;
extern float sequence_4_paragraph_fade_out_duration;
extern bool sequence_4_paragraph_fade_out_finished;
extern char *sequence_4_current_paragraph_string;

extern sprite_t *sequence_4_mallard_idle_sprite;
extern sprite_t *sequence_4_a_button_sprite;
extern sprite_t *sequence_4_start_button_sprite;
extern sprite_t *sequence_4_mallard_logo_black_sprite;

void sequence_4_draw_press_start_to_skip();
void sequence_4_draw_press_a_for_next();
void sequence_4_draw_mallard_idle_sprite();
void sequence_4_draw_mallard_logo();
void sequence_4_draw_paragraph(float deltatime);
void sequence_4_menu();

#endif // SEQUENCE_4_GRAPHICS_H
