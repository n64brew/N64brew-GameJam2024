#ifndef SEQUENCE_GAME_INPUT_H
#define SEQUENCE_GAME_INPUT_H

#include <libdragon.h>

extern bool sequence_game_finished;
extern bool sequence_game_paused;

extern float sequence_game_start_held_elapsed;

void sequence_game_process_controller(float deltatime);

#endif // SEQUENCE_GAME_INPUT_H
