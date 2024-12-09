#include "board.h"
#include "player.h"

color_t board[BOARD_SIZE];

void
board_init (void)
{
  board_fill (TILE_UNCLAIMED);
}

void
board_cleanup (void)
{
}

void
board_fill (color_t color)
{
  for (int row = 0; row < BOARD_ROWS; row++)
    {
      for (int col = 0; col < BOARD_COLS; col++)
        {
          board[row * BOARD_COLS + col] = color;
        }
    }
}

void
board_render (void)
{
  rdpq_set_mode_standard ();
  rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
  rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);

  rdpq_set_prim_color (BOARD_COLOR);
  int board_x0 = BOARD_MARGIN_LEFT - TILE_SPACING;
  int board_y0 = BOARD_MARGIN_TOP - TILE_SPACING;
  int board_x1 = BOARD_MARGIN_LEFT + BOARD_COLS * (TILE_SIZE + TILE_SPACING);
  int board_y1 = BOARD_MARGIN_TOP + BOARD_ROWS * (TILE_SIZE + TILE_SPACING);
  rdpq_fill_rectangle (board_x0, board_y0, board_x1, board_y1);

  for (int row = 0; row < BOARD_ROWS; row++)
    {
      for (int col = 0; col < BOARD_COLS; col++)
        {
          color_t tile_color = board[row * BOARD_COLS + col];
          board_render_tile (col, row, tile_color);
        }
    }
}

void
board_render_tile (int col, int row, color_t color)
{
  rdpq_set_prim_color (color);
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

bool
board_place_piece (Player *player)
{
  // First, make sure the placement is valid
  for (int piece_row = 0; piece_row < PIECE_ROWS; piece_row++)
    {
      for (int piece_col = 0; piece_col < PIECE_COLS; piece_col++)
        {
          int index = piece_row * PIECE_COLS + piece_col;
          if (player->piece_buffer[index] == CELL_FILLED)
            {
              int board_col = player->cursor_col + piece_col;
              int board_row = player->cursor_row + piece_row;
              // Ensure the cell is in-bounds of the board
              if (board_col < 0 || board_col >= BOARD_COLS || board_row < 0
                  || board_row >= BOARD_ROWS)
                {
                  return false;
                }
              // Ensure the cell is unclaimed
              color_t check_tile = board[board_row * BOARD_COLS + board_col];
              color_t unclaimed = TILE_UNCLAIMED;
              if (check_tile.r != unclaimed.r || check_tile.g != unclaimed.g
                  || check_tile.b != unclaimed.b
                  || check_tile.a != unclaimed.a)
                {
                  return false;
                }
            }
        }
    }

  // Actually place the piece on the board
  for (int piece_row = 0; piece_row < PIECE_ROWS; piece_row++)
    {
      for (int piece_col = 0; piece_col < PIECE_COLS; piece_col++)
        {
          int index = piece_row * PIECE_COLS + piece_col;
          if (player->piece_buffer[index] == CELL_FILLED)
            {
              int board_col = player->cursor_col + piece_col;
              int board_row = player->cursor_row + piece_row;
              board[board_row * BOARD_COLS + board_col] = player->color;
            }
        }
    }

  return true;
}
