#ifndef SEQUENCE_3_INPUT_H
#define SEQUENCE_3_INPUT_H

#include <libdragon.h>

extern float sequence3_b_btn_held_duration;
extern bool sequence3_paused;
extern bool sequence3_rewind;
extern bool sequence3_finished;

void sequence_3_process_controller(float deltatime);

#endif // SEQUENCE_3_INPUT_H
