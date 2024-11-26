#ifndef SEQUENCE_GAME_GRAPHICS_H
#define SEQUENCE_GAME_GRAPHICS_H

#include <libdragon.h>

#define SEQUENCE_GAME_MALLARD_IDLE_FRAMES 4

extern int sequence_game_frame;
extern bool sequence_game_started;
extern bool sequence_game_paused;

extern sprite_t *sequence_game_mallard_one_idle_sprite;
extern sprite_t *sequence_game_mallard_two_idle_sprite;
extern sprite_t *sequence_game_mallard_three_idle_sprite;
extern sprite_t *sequence_game_mallard_four_idle_sprite;

extern sprite_t *sequence_game_background_lakeview_terrace_sprite;
extern sprite_t *sequence_game_start_button_sprite;
extern sprite_t *sequence_game_paused_text_sprite;

extern struct Character *characters;

void sequence_game_render_ducks();
void sequence_game_draw_background_lakeview_terrace();
void sequence_game_draw_paused();
void sequence_game_render(float deltatime);

#endif // SEQUENCE_GAME_GRAPHICS_H
