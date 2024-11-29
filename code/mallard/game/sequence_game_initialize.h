#ifndef SEQUENCE_GAME_INITIALIZE_H
#define SEQUENCE_GAME_INITIALIZE_H

#include "sequence_game.h"

#define MIN_X 5
#define MAX_X 152 + 152 - 15
#define MIN_Y 111 - 85
#define MAX_Y 194

#define PLAYER_1_SPAWN_X1 5
#define PLAYER_1_SPAWN_Y1 111
#define PLAYER_1_SPAWN_X2 152
#define PLAYER_1_SPAWN_Y2 194
#define PLAYER_2_SPAWN_X1 5 + 152
#define PLAYER_2_SPAWN_Y1 111
#define PLAYER_2_SPAWN_X2 152 + 152 - 15
#define PLAYER_2_SPAWN_Y2 194
#define PLAYER_3_SPAWN_X1 5
#define PLAYER_3_SPAWN_Y1 111 - 85
#define PLAYER_3_SPAWN_X2 152
#define PLAYER_3_SPAWN_Y2 194 - 85 - 1
#define PLAYER_4_SPAWN_X1 5 + 152
#define PLAYER_4_SPAWN_Y1 111 - 85
#define PLAYER_4_SPAWN_X2 152 + 152 - 15
#define PLAYER_4_SPAWN_Y2 194 - 85 - 1

extern Duck *ducks;
extern struct Snowman *snowmen;
extern struct Controller *controllers;

extern sprite_t *sequence_game_mallard_one_base_sprite;
extern sprite_t *sequence_game_mallard_two_base_sprite;
extern sprite_t *sequence_game_mallard_three_base_sprite;
extern sprite_t *sequence_game_mallard_four_base_sprite;

extern sprite_t *sequence_game_mallard_one_walk_sprite;
extern sprite_t *sequence_game_mallard_two_walk_sprite;
extern sprite_t *sequence_game_mallard_three_walk_sprite;
extern sprite_t *sequence_game_mallard_four_walk_sprite;

extern sprite_t *sequence_game_mallard_one_slap_sprite;
extern sprite_t *sequence_game_mallard_two_slap_sprite;
extern sprite_t *sequence_game_mallard_three_slap_sprite;
extern sprite_t *sequence_game_mallard_four_slap_sprite;

extern sprite_t *sequence_game_mallard_one_run_sprite;
extern sprite_t *sequence_game_mallard_two_run_sprite;
extern sprite_t *sequence_game_mallard_three_run_sprite;
extern sprite_t *sequence_game_mallard_four_run_sprite;

void initialize_ducks();
void free_ducks();
void initialize_controllers();
void free_controllers();

int random_between(int min, int max);

#endif // SEQUENCE_GAME_INITIALIZE_H