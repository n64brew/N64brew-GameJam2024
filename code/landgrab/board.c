#include "board.h"

void
board_init (BoardState *board)
{
  board_fill (board, TILE_UNCLAIMED);
}

void
board_cleanup (BoardState *board)
{
}

void
board_fill (BoardState *board, color_t color)
{
  for (int row = 0; row < BOARD_ROWS; row++)
    {
      for (int col = 0; col < BOARD_COLS; col++)
        {
          board->tiles[row * BOARD_COLS + col] = color;
        }
    }
}

void
board_draw (BoardState *board)
{
  rdpq_set_mode_fill (BOARD_COLOR);
  int board_x0 = BOARD_MARGIN_LEFT - TILE_SPACING;
  int board_y0 = BOARD_MARGIN_TOP - TILE_SPACING;
  int board_x1 = BOARD_MARGIN_LEFT + BOARD_COLS * (TILE_SIZE + TILE_SPACING);
  int board_y1 = BOARD_MARGIN_TOP + BOARD_ROWS * (TILE_SIZE + TILE_SPACING);
  rdpq_fill_rectangle (board_x0, board_y0, board_x1, board_y1);

  for (int row = 0; row < BOARD_ROWS; row++)
    {
      for (int col = 0; col < BOARD_COLS; col++)
        {
          color_t tile_color = board->tiles[row * BOARD_COLS + col];
          board_draw_tile (col, row, tile_color);
        }
    }
}

void
board_draw_tile (int col, int row, color_t color)
{
  rdpq_set_mode_fill (color);
  Rect rect = board_get_tile_rect (col, row);
  rdpq_fill_rectangle (rect.x0, rect.y0, rect.x1, rect.y1);
}

Rect
board_get_tile_rect (int col, int row)
{
  int x0 = BOARD_MARGIN_LEFT + col * (TILE_SIZE + TILE_SPACING);
  int y0 = BOARD_MARGIN_TOP + row * (TILE_SIZE + TILE_SPACING);
  return (Rect){ x0, y0, x0 + TILE_SIZE, y0 + TILE_SIZE };
}
