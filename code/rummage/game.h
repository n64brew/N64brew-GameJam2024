#ifndef GAMEJAM2024_RUMMAGE_GAME_H
#define GAMEJAM2024_RUMMAGE_GAME_H

#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

void game_init();
void game_logic(float deltatime);
void game_render(float deltatime);
void game_cleanup();

#endif
