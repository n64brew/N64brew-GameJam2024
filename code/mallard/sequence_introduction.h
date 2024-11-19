#ifndef SEQUENCE_INTRODUCTION_H
#define SEQUENCE_INTRODUCTION_H

#include <libdragon.h>

#define BLACK RGBA32(0x00, 0x00, 0x00, 0xFF)
#define WHITE RGBA32(0xFF, 0xFF, 0xFF, 0xFF)
#define ASH_GRAY RGBA32(0xAD, 0xBA, 0xBD, 0xFF)
#define MAYA_BLUE RGBA32(0x6C, 0xBE, 0xED, 0xFF)
#define GUN_METAL RGBA32(0x31, 0x39, 0x3C, 0xFF)
#define REDWOOD RGBA32(0xB2, 0x3A, 0x7A, 0xFF)
#define BREWFONT RGBA32(242, 209, 155, 0xFF)

#define DRAW_LIBDRAGON_LOGO_DURATION 2.0f
#define DRAW_MALLARD_LOGO_FADE_IN_DURATION 1.0f
#define DRAW_MALLARD_LOGO_DURATION 1.0f
#define DRAW_MALLARD_LOGO_FADE_OUT_DURATION 1.0f
#define PARAGRAPH_FADE_OUT_DURATION 1.0f

#define DEFAULT_PARAGRAPH_SPEED 4

#define SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_01 "$03^00Hark, ye gamer folk, to the tale of Mallard. That noble band who didst rise like the morning sun to lay waste upon the Winter..."
#define SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_02 "$03^00With spirit ablaze, they took siege, keen as the falcon and fierce as the storm..."
#define SEQEUENCE_INTRODUCTION_GAMEJAM_PARARGAPH_03 "$03^00Leaving banners fallen and pride humbled, this is the tale of that very day..."

extern bool sequence_introduction_finished;
extern bool sequence_game_started;

void sequence_introduction(float deltatime);

#endif // SEQUENCE_INTRODUCTION_H
