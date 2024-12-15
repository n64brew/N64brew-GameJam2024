#include "board.h"
#include "color.h"

#define TILE_UNCLAIMED 0
#define TILE_UNCLAIMED_COLOR RGBA32 (160, 160, 160, 64)

static int board[BOARD_SIZE];
static sprite_t *x_sprite = NULL;

void
board_init (void)
{
  memset (board, TILE_UNCLAIMED, sizeof (board));
  x_sprite = sprite_load ("rom:/landgrab/x.ia8.sprite");
}

void
board_cleanup (void)
{
  sprite_free (x_sprite);
  x_sprite = NULL;
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
          int tile = board[row * BOARD_COLS + col];
          color_t tile_color = TILE_UNCLAIMED_COLOR;
          if (tile != TILE_UNCLAIMED)
            {
              tile_color = PLAYER_COLORS[tile - 1];
            }
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

void
board_render_bad_tile_marker (int col, int row)
{
  Rect rect = board_get_tile_rect (col, row);

  rdpq_mode_push ();
  {
    rdpq_set_mode_standard ();
    rdpq_mode_filter (FILTER_BILINEAR);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner (
        RDPQ_COMBINER1 ((PRIM, ENV, TEX0, ENV), (0, 0, 0, TEX0)));
    rdpq_set_prim_color (COLOR_DARK_GRAY); // fill color
    rdpq_set_env_color (COLOR_DARK_GRAY);  // outline color
    rdpq_sprite_blit (x_sprite, rect.x0, rect.y0, &(rdpq_blitparms_t){});
  }
  rdpq_mode_pop ();
}

bool
board_is_tile_valid (int col, int row)
{
  // Just a simple bounds check
  return col >= 0 && col < BOARD_COLS && row >= 0 && row < BOARD_ROWS;
}

bool
board_is_tile_unclaimed (int col, int row)
{
  // If it's not in bounds, it's technically not unclaimed.
  // It is perfectly acceptable to test bogus coordinates.
  // (AI players will try a ton of bogus moves)
  if (!board_is_tile_valid (col, row))
    {
      return false;
    }
  return board[row * BOARD_COLS + col] == TILE_UNCLAIMED;
}

bool
board_is_tile_claimed (int col, int row, PlyNum player)
{
  // If it's not in bounds, it's technically not claimed either.
  // It is perfectly acceptable to test bogus coordinates.
  // (AI players will try a ton of bogus moves)
  if (!board_is_tile_valid (col, row))
    {
      return false;
    }
  return board[row * BOARD_COLS + col] == player + 1;
}

Rect
board_get_tile_rect (int col, int row)
{
  int x0 = BOARD_MARGIN_LEFT + col * (TILE_SIZE + TILE_SPACING);
  int y0 = BOARD_MARGIN_TOP + row * (TILE_SIZE + TILE_SPACING);
  return (Rect){ x0, y0, x0 + TILE_SIZE, y0 + TILE_SIZE };
}

void
board_blit_piece (Player *player)
{
  PlyNum p = player->plynum;
  // Actually place the player's piece on the board
  // Assumes the placement has already been validated.
  // This will overwrite tiles if the placement is not valid!
  for (int piece_row = 0; piece_row < PIECE_ROWS; piece_row++)
    {
      for (int piece_col = 0; piece_col < PIECE_COLS; piece_col++)
        {
          int index = piece_row * PIECE_COLS + piece_col;
          if (player->piece_buffer[index] == CELL_FILLED)
            {
              int board_col = player->cursor_col + piece_col;
              int board_row = player->cursor_row + piece_row;
              board[board_row * BOARD_COLS + board_col] = p + 1;
            }
        }
    }
}

CheckPieceResult
board_check_piece (Player *player)
{
  PlyNum p = player->plynum;
  // Assume the placement is valid until proven otherwise
  CheckPieceResult result = { .is_available = true };

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
              // Ensure the cell is unclaimed
              if (!board_is_tile_unclaimed (board_col, board_row))
                {
                  result.is_available = false;
                  goto board_check_piece_bail;
                }
              // Convenience check for the first piece placed
              result.is_in_corner
                  = result.is_in_corner
                    || ((board_col == 0 || board_col == BOARD_COLS - 1)
                        && (board_row == 0 || board_row == BOARD_ROWS - 1));
              // Check left face
              if (board_is_tile_claimed (board_col - 1, board_row, p))
                {
                  result.is_touching_faces = true;
                  goto board_check_piece_bail;
                }
              // Check right face
              if (board_is_tile_claimed (board_col + 1, board_row, p))
                {
                  result.is_touching_faces = true;
                  goto board_check_piece_bail;
                }
              // Check top face
              if (board_is_tile_claimed (board_col, board_row - 1, p))
                {
                  result.is_touching_faces = true;
                  goto board_check_piece_bail;
                }
              // Check bottom face
              if (board_is_tile_claimed (board_col, board_row + 1, p))
                {
                  result.is_touching_faces = true;
                  goto board_check_piece_bail;
                }
              // Check top-left corner
              if (board_is_tile_claimed (board_col - 1, board_row - 1, p))
                {
                  result.is_touching_corners = true;
                }
              // Check top-right corner
              if (board_is_tile_claimed (board_col + 1, board_row - 1, p))
                {
                  result.is_touching_corners = true;
                }
              // Check bottom-left corner
              if (board_is_tile_claimed (board_col - 1, board_row + 1, p))
                {
                  result.is_touching_corners = true;
                }
              // Check bottom-right corner
              if (board_is_tile_claimed (board_col + 1, board_row + 1, p))
                {
                  result.is_touching_corners = true;
                }
            }
        }
    }

board_check_piece_bail:

  if (player_is_first_turn (player))
    {
      // SPECIAL CASE: On the player's first turn, there are no pieces to
      // touch, so we instead check to make sure the piece is in a corner of
      // the board.
      result.is_valid = result.is_available && result.is_in_corner;
    }
  else
    {
      // Standard rule: can only touch corners, not faces
      result.is_valid = result.is_available && result.is_touching_corners
                        && !result.is_touching_faces;
    }

  return result;
}
