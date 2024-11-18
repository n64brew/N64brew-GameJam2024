#ifndef SEQUENCE_4_GRAPHICS_H
#define SEQUENCE_4_GRAPHICS_H

#include <libdragon.h>

extern bool sequence_4_libdragon_logo_started;
extern bool sequence_4_libdragon_logo_finished;
extern float sequence_4_libdragon_logo_elapsed;

extern float sequence_4_time;
extern int sequence_4_frame;
extern int sequence_4_current_paragraph_drawn_characters;
extern int sequence_4_current_paragraph;
extern int sequence_4_current_paragraph_speed;
extern bool sequence_4_paragraphs_finished;
extern bool sequence_4_current_paragraph_finished;
extern bool sequence_4_paragraphs_started;
extern bool sequence_4_paragraph_fade_out_started;
extern float sequence_4_paragraph_fade_out_elapsed;
extern bool sequence_4_paragraph_fade_out_finished;
extern char *sequence_4_current_paragraph_string;
extern float sequence_4_menu_fade_in_duration;

extern bool sequence_4_mallard_logo_started;
extern bool sequence_4_mallard_logo_finished;
extern float sequence_4_mallard_logo_elapsed;

extern sprite_t *sequence_4_mallard_idle_sprite;
extern sprite_t *sequence_4_a_button_sprite;
extern sprite_t *sequence_4_start_button_sprite;
extern sprite_t *sequence_4_mallard_libdragon_sprite;
extern sprite_t *sequence_4_mallard_logo_black_sprite;
extern sprite_t *sequence_4_mallard_logo_white_sprite;
extern sprite_t *sequence_4_mallard_menu_1_sprite;
extern sprite_t *sequence_4_mallard_menu_2_sprite;

extern xm64player_t xm;

void sequence_4_draw_libdragon_logo(float deltatime);
void sequence_4_draw_mallard_logo(float deltatime);
void sequence_4_draw_press_start_to_skip();
void sequence_4_draw_press_a_for_next();
void sequence_4_draw_paragraph(float deltatime);
void sequence_4_menu(float deltatime);
void sequence_4_draw_mallard_idle_sprite();

#endif // SEQUENCE_4_GRAPHICS_H
