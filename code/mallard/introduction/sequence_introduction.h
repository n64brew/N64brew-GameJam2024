#ifndef SEQUENCE_INTRODUCTION_H
#define SEQUENCE_INTRODUCTION_H

#include <libdragon.h>

#define DRAW_LIBDRAGON_LOGO_DURATION 2.0f
#define DRAW_MALLARD_LOGO_FADE_IN_DURATION 1.0f
#define DRAW_MALLARD_LOGO_DURATION 1.0f
#define DRAW_MALLARD_LOGO_FADE_OUT_DURATION 1.0f
#define PARAGRAPH_FADE_OUT_DURATION 1.0f

#define DEFAULT_PARAGRAPH_SPEED 4

#define SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_01 "$04^00Hark, ye gamer folk, to the tale of Mallard. That noble band who didst rise like the morning sun to lay waste upon the Winter..."
#define SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_02 "$04^00With spirit ablaze, they took siege, keen as the falcon and fierce as the storm..."
#define SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_03 "$04^00Leaving banners fallen and pride humbled, this is the tale of that very day..."

extern bool sequence_introduction_finished;
extern bool sequence_game_started;

void sequence_introduction(float deltatime);

#endif // SEQUENCE_INTRODUCTION_H
