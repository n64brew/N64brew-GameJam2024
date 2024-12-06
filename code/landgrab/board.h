#ifndef GAMEJAM2024_LANDGRAB_BOARD_H
#define GAMEJAM2024_LANDGRAB_BOARD_H

#include "constants.h"

#define BOARD_ROWS 20
#define BOARD_COLS 20
#define BOARD_SIZE (BOARD_ROWS * BOARD_COLS)

#define BOARD_COLOR RGBA32 (0, 0, 0, 255)

#define BOARD_MARGIN_TOP 24
#define BOARD_MARGIN_LEFT 72

#define TILE_SIZE 8
#define TILE_SPACING 1
#define TILE_UNCLAIMED RGBA32 (50, 50, 50, 255)

typedef struct
{
  color_t tiles[BOARD_SIZE];
} BoardState;

typedef struct
{
  int x0;
  int y0;
  int x1;
  int y1;
} Rect;

void board_init (BoardState *board);

void board_cleanup (BoardState *board);

void board_fill (BoardState *board, color_t color);

void board_draw (BoardState *board);

void board_draw_tile (int col, int row, color_t color);

Rect board_get_tile_rect (int x, int y);

#endif
