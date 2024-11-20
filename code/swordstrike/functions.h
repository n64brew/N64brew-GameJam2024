#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "types.h"
#include "globals.h"
#include "libdragon.h"

void initPlayer(struct player* p, struct weapon* weapon);
void updatePlayerBoundingBox(struct player *player);
void defineFloor(struct floorPiece *floor);
void updateWeaponHitbox(struct weapon *weapon);

// PLAYER FIXED LOOP FUNCTIONS
bool isPlayerOnFloor(struct player *p, struct floorPiece *f);
void handleMovementAndGravity(struct player* p, struct player **players);
void updatePlayerPos(struct player* p, struct floorPiece** floors, int* numFloors, struct player **players);
bool detectPlayerCollision(struct player* p, struct player** players, int newX, int newY);
bool detectPlayerFeetCollision(struct player* p, struct player** players, int newX, int newY);
bool validateAndMovePlayer(struct player* p, struct player** players, int newX, int newY, int leftEdge, int rightEdge);
bool onTopOfEnemy(struct player *player1, struct player *player2);
bool checkBoundingBoxOverlap(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);
void checkPlayerWeaponCollision(struct player* player1, struct player* player2);
void initPlayerSlide(struct player* p);

// AI FUNCTIONS
float calculateDistance(struct player* player1, struct player* player2);
bool targetIsAbove(struct player* ai, struct player* target);
int targetDirection(struct player* ai, struct player* target);
void generateCompInputs(struct player* ai, struct player* target, struct floorPiece** floors, int* numFloors);
void initAiSlide(struct player* ai, int dir);

// PLAYER LOOP FUNCTIONS
void pollPlayerInput(struct player *p,joypad_buttons_t *joypad_held);
void pollAttackInput(struct player *p, joypad_buttons_t *joypad_held);
void rdpq_draw_one_rectangle(int *x, int *y, int *w, int *h, color_t color);
void rdpq_draw_one_floor_piece(int *x, int *y, int *w, int *h, color_t color);
void draw_players_and_level(struct player** players, sprite_t** player_sprites, sprite_t** player_left_attack_anim,
                            sprite_t** player_right_attack_anim, struct floorPiece** floors, int *numFloors, color_t WHITE);

#endif
