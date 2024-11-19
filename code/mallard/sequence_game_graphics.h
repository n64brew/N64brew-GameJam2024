#ifndef SEQUENCE_INTRODUCTION_GRAPHICS_H
#define SEQUENCE_INTRODUCTION_GRAPHICS_H

#include <libdragon.h>

#define SEQUENCE_GAME_MALLARD_IDLE_FRAMES 4

extern int sequence_game_frame;
extern bool sequence_game_started;
extern float sequence_game_fade_in_duration;

extern sprite_t *sequence_game_mallard_idle_sprite;
extern sprite_t *sequence_game_background_lakeview_terrace_sprite;

void sequence_game_draw_mallard_idle_sprite();
void sequence_game_draw_background_lakeview_terrace();
void sequence_game_render(float deltatime);

#endif // SEQUENCE_INTRODUCTION_GRAPHICS_H
