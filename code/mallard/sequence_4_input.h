#ifndef SEQUENCE_4_INPUT_H
#define SEQUENCE_4_INPUT_H

#include <libdragon.h>

// Libdragon Logo
extern bool sequence_4_libdragon_logo_started;
extern bool sequence_4_libdragon_logo_finished;
extern float sequence_4_libdragon_logo_elapsed;

// Mallard Logo
extern bool sequence_4_mallard_logo_started;
extern bool sequence_4_mallard_logo_finished;
extern float sequence_4_mallard_logo_elapsed;

// Paragraphs
extern bool sequence_4_paragraphs_started;
extern bool sequence_4_paragraphs_finished;
extern bool sequence_4_paragraph_fade_out_started;
extern bool sequence_4_paragraph_fade_out_finished;
extern float sequence_4_paragraph_fade_out_elapsed;
extern bool sequence_4_current_paragraph_finished;
extern int sequence_4_current_paragraph;
extern int sequence_4_current_paragraph_drawn_characters;
extern int sequence_4_current_paragraph_speed;
extern char *sequence_4_current_paragraph_string;

extern float sequence_4_time;
extern bool sequence_4_finished;

// Music
extern int sequence_4_currentXMPattern;

void sequence_4_process_controller(float deltatime);

#endif // SEQUENCE_4_INPUT_H
