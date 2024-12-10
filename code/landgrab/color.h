#ifndef GAMEJAM2024_LANDGRAB_COLOR_H
#define GAMEJAM2024_LANDGRAB_COLOR_H

#include "global.h"

#undef RGBA32
#define RGBA32(r, g, b, a) ((color_t){ r, g, b, a })

#define COLOR_WHITE RGBA32 (255, 255, 255, 255)
#define COLOR_BLACK RGBA32 (0, 0, 0, 255)
#define COLOR_GOLD RGBA32 (242, 209, 155, 0xFF)
#define COLOR_DARK_GRAY RGBA32 (0x31, 0x39, 0x3C, 0xFF)
#define COLOR_LIGHT_GRAY RGBA32 (0x9F, 0xA2, 0xA4, 0xFF)

static const color_t PLAYER_COLORS[] = {
  PLAYERCOLOR_1,
  PLAYERCOLOR_2,
  PLAYERCOLOR_3,
  PLAYERCOLOR_4,
};

color_t color_between(color_t color_a, color_t color_b, float p);

#endif // GAMEJAM2024_LANDGRAB_COLOR_H
