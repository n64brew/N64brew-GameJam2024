#ifndef SEQUENCE_GAME_INITIALIZE_H
#define SEQUENCE_GAME_INITIALIZE_H

#include "sequence_game.h"

#define DUCK_TIME_SEEKING_TARGET_EASY 0.75f
#define DUCK_TIME_SEEKING_TARGET_MEDIUM 0.50f
#define DUCK_TIME_SEEKING_TARGET_HARD 0.0f

#define DUCK_MIN_X 5
#define DUCK_MAX_X 294
#define DUCK_MIN_Y 26
#define DUCK_MAX_Y 194

#define SNOWMAN_MIN_X 17
#define SNOWMAN_MAX_X 304
#define SNOWMAN_MIN_Y 32
#define SNOWMAN_MAX_Y 205

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

extern sprite_t *sequence_game_mallard_one_idle_sprite;
extern sprite_t *sequence_game_mallard_two_idle_sprite;
extern sprite_t *sequence_game_mallard_three_idle_sprite;
extern sprite_t *sequence_game_mallard_four_idle_sprite;

extern sprite_t *sequence_game_mallard_one_damage_sprite;
extern sprite_t *sequence_game_mallard_two_damage_sprite;
extern sprite_t *sequence_game_mallard_three_damage_sprite;
extern sprite_t *sequence_game_mallard_four_damage_sprite;

void initialize_controllers();
void free_controllers();
int random_between(int min, int max);

#endif // SEQUENCE_GAME_INITIALIZE_H