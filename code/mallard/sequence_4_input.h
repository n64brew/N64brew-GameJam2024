#ifndef SEQUENCE_4_INPUT_H
#define SEQUENCE_4_INPUT_H

#include <libdragon.h>

extern float sequence_4_time;
extern bool sequence_4_finished;
extern bool sequence_4_current_paragraph_finished;
extern bool sequence_4_all_paragraphs_finished;
extern int sequence_4_current_paragraph;
extern int sequence_4_drawn_characters;
extern int sequence_4_paragraph_speed;
extern char *sequence_4_current_paragraph_string;
extern int sequence_4_currentXMPattern;

void sequence_4_process_controller(float deltatime);

#endif // SEQUENCE_4_INPUT_H
