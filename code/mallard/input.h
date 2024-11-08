#ifndef INPUT_H
#define INPUT_H

#include <libdragon.h>

extern float b_btn_held_duration;
extern bool b_btn_held;
extern bool paused;

void handle_input(float deltatime);

#endif // INPUT_H
