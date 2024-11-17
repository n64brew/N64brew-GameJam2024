#ifndef SEQUENCE_4_H
#define SEQUENCE_4_H

#include <libdragon.h>

#define FONT_PACIFICO 1
#define FONT_CELTICGARMONDTHESECOND 2
#define FONT_HALODEK 3

#define DRAW_MALLARD_LOGO_FADE_IN_DURATION 1.0f
#define DRAW_MALLARD_LOGO_DURATION 1.0f
#define DRAW_MALLARD_LOGO_FADE_OUT_DURATION 1.0f
#define DRAW_FADE_WHITE_TO_BLACK_DURATION 1.0f

#define DEFAULT_PARAGRAPH_SPEED 4
#define SEQEUENCE_4_GAMEJAM_PARARGAPH_01 "$02Hark, ye gamer folk, to the tale of Mallard. That noble band who didst rise like the morning sun to lay waste upon the Winter..."
#define SEQEUENCE_4_GAMEJAM_PARARGAPH_02 "$02With spirit ablaze, they took siege, keen as the falcon and fierce as the storm..."
#define SEQEUENCE_4_GAMEJAM_PARARGAPH_03 "$02Leaving banners fallen and pride humbled, this is the tale of that very day..."

extern bool sequence_4_story;
extern bool sequence_5_BLANK;

void sequence_4(float deltatime);

#endif // SEQUENCE_4_H
