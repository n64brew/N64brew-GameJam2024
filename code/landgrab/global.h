#ifndef GAMEJAM2024_LANDGRAB_GLOBAL_H
#define GAMEJAM2024_LANDGRAB_GLOBAL_H

#include <libdragon.h>

#include "../../core.h"

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

// clang-format off
#define PLAYER_FOREACH(iterator_token) \
    for (\
        PlyNum iterator_token = PLAYER_1; \
        iterator_token < MAXPLAYERS; \
        iterator_token += 1 \
    )

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

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
