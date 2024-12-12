#ifndef GAMEJAM2024_LANDGRAB_BOARD_H
#define GAMEJAM2024_LANDGRAB_BOARD_H

#include "global.h"
#include "player.h"

// Tile sizing constants
#define TILE_SIZE 8
#define TILE_SPACING 1

// Board sizing constants
#define BOARD_ROWS 20
#define BOARD_COLS 20
#define BOARD_SIZE (BOARD_ROWS * BOARD_COLS)
#define BOARD_COLOR RGBA32 (0, 0, 0, 64)

// Positioning constants
#define BOARD_MARGIN_TOP 28
#define BOARD_MARGIN_LEFT 72
#define BOARD_TOP (BOARD_MARGIN_TOP - TILE_SPACING)
#define BOARD_LEFT (BOARD_MARGIN_LEFT - TILE_SPACING)
#define BOARD_RIGHT                                                           \
  (BOARD_MARGIN_LEFT + BOARD_COLS * (TILE_SIZE + TILE_SPACING))
#define BOARD_BOTTOM                                                          \
  (BOARD_MARGIN_TOP + BOARD_ROWS * (TILE_SIZE + TILE_SPACING))

typedef struct
{
  bool is_valid;
  bool is_available;
  bool is_in_corner;
  bool is_touching_corners;
  bool is_touching_faces;
} CheckPieceResult;

void board_init (void);

void board_cleanup (void);

void board_render (void);

bool board_is_tile_valid (int col, int row);

bool board_is_tile_unclaimed (int col, int row);

bool board_is_tile_claimed (int col, int row, PlyNum player);

void board_render_tile (int col, int row, color_t color);

void board_render_bad_tile_marker (int col, int row);

Rect board_get_tile_rect (int x, int y);

CheckPieceResult board_check_piece (Player *player);

void board_blit_piece (Player *player);

bool board_place_piece (Player *player);

#endif // GAMEJAM2024_LANDGRAB_BOARD_H
