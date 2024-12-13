#ifndef GAMEJAM2024_TOHUBOHU_GAME_H
#define GAMEJAM2024_TOHUBOHU_GAME_H

#include <libdragon.h>
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

void game_init();
void game_logic(float deltatime);
void game_render(float deltatime, T3DViewport viewport);
void game_render_gl(float deltatime);
void game_cleanup();
int game_key();
int game_vault();
void start_game();
void stop_game();
bool is_playing();
bool is_paused();
void toggle_pause();
bool has_winner();
PlyNum winner();

#endif
