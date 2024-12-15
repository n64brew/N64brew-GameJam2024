#ifndef BATTLE_H
#define BATTLE_H
#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"


#include "lucker.h"  // Include the header for the function declaration


#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

extern bool isBattle;

extern bool isDead;

void battle_init();

void battle_cleanup();

void battle_end(player *victor);

void battle_fixedLoop(float dt);

void battle_loop(float dt, rspq_syncpoint_t syncPoint);

void battle_draw();

void fighter_cleanup(player *player);

void battle_start(player *playerOne, player *playerTwo);

void battle_player_init (player *player, color_t color, T3DModel *model);

void battle_ui(int fontIndex, T3DViewport viewport);

player* get_current_player(bool left);

#endif  // BATTLE_H
//#include "lucker.h"