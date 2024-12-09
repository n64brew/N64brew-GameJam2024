#ifndef GAMEJAM2024_LANDGRAB_GLOBAL_H
#define GAMEJAM2024_LANDGRAB_GLOBAL_H

#include <libdragon.h>

#include "../../core.h"

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define DISPLAY_BPP 16
#define DISPLAY_BUFFERS 3
#define DISPLAY_RESOLUTION RESOLUTION_320x240
#define DISPLAY_BITDEPTH DEPTH_16_BPP
#define DISPLAY_GAMMA GAMMA_NONE
#define DISPLAY_FILTERS FILTERS_RESAMPLE_ANTIALIAS

#undef RGBA32
#define RGBA32(r, g, b, a) ((color_t){ r, g, b, a })

#define COLOR_WHITE RGBA32 (255, 255, 255, 255)
#define COLOR_BLACK RGBA32 (0, 0, 0, 255)

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

#endif //GAMEJAM2024_LANDGRAB_GLOBAL_H
