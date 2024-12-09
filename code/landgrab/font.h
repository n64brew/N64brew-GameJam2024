#ifndef GAMEJAM2024_LANDGRAB_FONT_H
#define GAMEJAM2024_LANDGRAB_FONT_H

#include "global.h"

typedef enum
{
    FONT_SQUAREWAVE = 1
} Font;

typedef enum
{
    STYLE_SQUAREWAVE_P1 = 0x00,
    STYLE_SQUAREWAVE_P2 = 0x01,
    STYLE_SQUAREWAVE_P3 = 0x02,
    STYLE_SQUAREWAVE_P4 = 0x03,
    STYLE_SQUAREWAVE_WHITE = 0x04
} StyleSquarewave;

#define FMT_SQUAREWAVE_P1 "^00"
#define FMT_SQUAREWAVE_P2 "^01"
#define FMT_SQUAREWAVE_P3 "^02"
#define FMT_SQUAREWAVE_P4 "^03"
#define FMT_SQUAREWAVE_WHITE "^04"

extern rdpq_font_t *font_squarewave;

void font_init (void);

void font_cleanup (void);

#endif
