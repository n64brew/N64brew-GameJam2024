#ifndef GAMEJAM2024_LANDGRAB_GLOBAL_H
#define GAMEJAM2024_LANDGRAB_GLOBAL_H

#include <libdragon.h>

#include "../../core.h"

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

#undef RGBA32
#define RGBA32(r, g, b, a) ((color_t){ r, g, b, a })

#define COLOR_WHITE RGBA32 (255, 255, 255, 255)
#define COLOR_BLACK RGBA32 (0, 0, 0, 255)
#define COLOR_DARK_GRAY RGBA32 (0x31, 0x39, 0x3C, 0xFF)
#define COLOR_LIGHT_GRAY RGBA32 (0x9F, 0xA2, 0xA4, 0xFF)

static const color_t PLAYER_COLORS[] = {
  PLAYERCOLOR_1,
  PLAYERCOLOR_2,
  PLAYERCOLOR_3,
  PLAYERCOLOR_4,
};

// clang-format off
#define PLAYER_FOREACH(iterator_token) \
    for (\
        PlyNum iterator_token = PLAYER_1; \
        iterator_token < MAXPLAYERS; \
        iterator_token += 1 \
    )

typedef struct { int x0; int y0; int x1; int y1; } Rect;
// clang-format on

#define JOYPAD_8WAY_IS_UP(d)                                                  \
  (d == JOYPAD_8WAY_UP || d == JOYPAD_8WAY_UP_LEFT                            \
   || d == JOYPAD_8WAY_UP_RIGHT)
#define JOYPAD_8WAY_IS_DOWN(d)                                                \
  (d == JOYPAD_8WAY_DOWN || d == JOYPAD_8WAY_DOWN_LEFT                        \
   || d == JOYPAD_8WAY_DOWN_RIGHT)
#define JOYPAD_8WAY_IS_LEFT(d)                                                \
  (d == JOYPAD_8WAY_LEFT || d == JOYPAD_8WAY_UP_LEFT                          \
   || d == JOYPAD_8WAY_DOWN_LEFT)
#define JOYPAD_8WAY_IS_RIGHT(d)                                               \
  (d == JOYPAD_8WAY_RIGHT || d == JOYPAD_8WAY_UP_RIGHT                        \
   || d == JOYPAD_8WAY_DOWN_RIGHT)

#endif // GAMEJAM2024_LANDGRAB_GLOBAL_H
