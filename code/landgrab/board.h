#ifndef GAMEJAM2024_LANDGRAB_BOARD_H
#define GAMEJAM2024_LANDGRAB_BOARD_H

#include "global.h"

typedef struct Player Player;

#define BOARD_ROWS 20
#define BOARD_COLS 20
#define BOARD_SIZE (BOARD_ROWS * BOARD_COLS)

#define BOARD_COLOR RGBA32 (0, 0, 0, 64)

#define BOARD_MARGIN_TOP 28
#define BOARD_MARGIN_LEFT 72

#define TILE_SIZE 8
#define TILE_SPACING 1
#define TILE_UNCLAIMED RGBA32 (160, 160, 160, 64)

#define BOARD_TOP (BOARD_MARGIN_TOP - TILE_SPACING)
#define BOARD_LEFT (BOARD_MARGIN_LEFT - TILE_SPACING)
#define BOARD_RIGHT (BOARD_MARGIN_LEFT + BOARD_COLS * (TILE_SIZE + TILE_SPACING))
#define BOARD_BOTTOM (BOARD_MARGIN_TOP + BOARD_ROWS * (TILE_SIZE + TILE_SPACING))

void board_init (void);

void board_cleanup (void);

void board_fill (color_t color);

void board_render (void);

void board_render_tile (int col, int row, color_t color);

Rect board_get_tile_rect (int x, int y);

bool board_place_piece (Player *player);

#endif // GAMEJAM2024_LANDGRAB_BOARD_H
