#ifndef SEQUENCE_GAME_GRAPHICS_H
#define SEQUENCE_GAME_GRAPHICS_H

#include <libdragon.h>

#define SEQUENCE_GAME_MALLARD_BASE_FRAMES 1
#define SEQUENCE_GAME_MALLARD_WALK_FRAMES 8
#define SEQUENCE_GAME_MALLARD_IDLE_FRAMES 4
#define SEQUENCE_GAME_MALLARD_SLAP_FRAMES 3
#define SEQUENCE_GAME_MALLARD_RUN_FRAMES 8

#define SEQUENCE_GAME_SNOWMAN_IDLE_FRAMES 4
#define SEQUENCE_GAME_SNOWMAN_JUMP_FRAMES 7

extern bool sequence_game_started;
extern bool sequence_game_paused;

extern sprite_t *sequence_game_mallard_one_base_sprite;
extern sprite_t *sequence_game_mallard_two_base_sprite;
extern sprite_t *sequence_game_mallard_three_base_sprite;
extern sprite_t *sequence_game_mallard_four_base_sprite;

extern sprite_t *sequence_game_mallard_one_walk_sprite;
extern sprite_t *sequence_game_mallard_two_walk_sprite;
extern sprite_t *sequence_game_mallard_three_walk_sprite;
extern sprite_t *sequence_game_mallard_four_walk_sprite;

extern sprite_t *sequence_game_background_lakeview_terrace_sprite;
extern sprite_t *sequence_game_start_button_sprite;
extern sprite_t *sequence_game_paused_text_sprite;

extern Duck *ducks;
extern Snowman *snowmen;

void sequence_game_render_ducks();
void sequence_game_render_snowmen();
void sequence_game_draw_background_lakeview_terrace();
void sequence_game_draw_paused();
void sequence_game_render(float deltatime);

int get_frame_from_snowman(Snowman *snowman);
int get_frame_from_duck(Duck *duck);

#endif // SEQUENCE_GAME_GRAPHICS_H
