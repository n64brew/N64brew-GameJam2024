#ifndef GAMEJAM2024_LANDGRAB_FONT_H
#define GAMEJAM2024_LANDGRAB_FONT_H

#include "global.h"

extern rdpq_font_t *font_squarewave;
extern rdpq_font_t *font_anita;
extern rdpq_font_t *font_bbrick;

typedef enum
{
  FONT_SQUAREWAVE = 1,
  FONT_ANITA = 2,
  FONT_BBRICK = 3
} Font;

typedef enum
{
  FONT_STYLE_P1 = 0x00,
  FONT_STYLE_P2 = 0x01,
  FONT_STYLE_P3 = 0x02,
  FONT_STYLE_P4 = 0x03,
  FONT_STYLE_WHITE = 0x04,
} FontStyle;

#define FMT_STYLE_P1 "^00"
#define FMT_STYLE_P2 "^01"
#define FMT_STYLE_P3 "^02"
#define FMT_STYLE_P4 "^03"
#define FMT_STYLE_WHITE "^04"

void font_init (void);

void font_cleanup (void);

#endif
