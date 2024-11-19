#ifndef SEQUENCE_GAME_H
#define SEQUENCE_GAME_H

#include <libdragon.h>

#define BLACK RGBA32(0x00, 0x00, 0x00, 0xFF)

#define GAME_FADE_IN_DURATION 1.0f

extern bool sequence_game_finished;

void sequence_game(float deltatime);

#endif // SEQUENCE_GAME_H
