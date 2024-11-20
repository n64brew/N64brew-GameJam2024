#ifndef SEQUENCE_GAME_H
#define SEQUENCE_GAME_H

#include <libdragon.h>

#define GAME_FADE_IN_DURATION 1.0f
#define GAME_EXIT_DURATION 2.0f
#define GAME_EXIT_THRESHOLD_DURATION 0.1f

extern bool sequence_game_finished;

void sequence_game(float deltatime);

#endif // SEQUENCE_GAME_H
