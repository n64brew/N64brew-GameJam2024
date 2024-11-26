#ifndef SEQUENCE_GAME_INITIALIZE_H
#define SEQUENCE_GAME_INITIALIZE_H

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

extern struct Character *characters;
extern struct Controller *controllers;

void initialize_characters();
void free_characters();
void initialize_controllers();
void free_controllers();

#endif // SEQUENCE_GAME_INITIALIZE_H