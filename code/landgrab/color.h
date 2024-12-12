#ifndef GAMEJAM2024_LANDGRAB_COLOR_H
#define GAMEJAM2024_LANDGRAB_COLOR_H

#include "global.h"

#undef RGBA32
#define RGBA32(r, g, b, a) ((color_t){ r, g, b, a })

#define COLOR_WHITE RGBA32 (255, 255, 255, 255)
#define COLOR_BLACK RGBA32 (0, 0, 0, 255)
#define COLOR_DARK_GRAY RGBA32 (0x31, 0x39, 0x3C, 0xFF)

static const color_t PLAYER_COLORS[] = {
  PLAYERCOLOR_1,
  PLAYERCOLOR_2,
  PLAYERCOLOR_3,
  PLAYERCOLOR_4,
};

color_t color_between (color_t color_a, color_t color_b, float p);

#endif // GAMEJAM2024_LANDGRAB_COLOR_H
