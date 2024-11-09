#ifndef INPUT_H
#define INPUT_H

#include <libdragon.h>

extern float sequence3_b_btn_held_duration;
extern bool sequence3_paused;
extern bool sequence3_rewind;
extern bool sequence3_video_finished;

void process_controller(float deltatime);

#endif // INPUT_H
