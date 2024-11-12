#ifndef SEQUENCE_3_INPUT_H
#define SEQUENCE_3_INPUT_H

#include <libdragon.h>

extern float sequence_3_b_btn_held_duration;
extern bool sequence_3_finished;
extern bool sequence_3_paused;
extern bool sequence_3_rewind;
extern bool sequence_3_finished;

void sequence_3_process_controller(float deltatime);

#endif // SEQUENCE_3_INPUT_H
