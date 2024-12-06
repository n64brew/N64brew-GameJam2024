#include "player.h"
#include "board.h"

void
player_init (PlayerState *player, PlyNum plynum)
{
  player->plynum = plynum;
  player->color = PLAYER_COLORS[plynum];
  memset (player->pieces_used, 0, sizeof (player->pieces_used));
  player_change_piece (player, 0);
  switch (plynum)
    {
    case PLAYER_1:
      player_set_cursor (player, 0, 0);
      break;
    case PLAYER_2:
      player_set_cursor (player, BOARD_COLS - 1, 0);
      break;
    case PLAYER_3:
      player_set_cursor (player, 0, BOARD_ROWS - 1);
      break;
    case PLAYER_4:
      player_set_cursor (player, BOARD_COLS - 1, BOARD_ROWS - 1);
      break;
    }
}

void
player_cleanup (PlayerState *player)
{
}

void
player_draw (PlayerState *player)
{
  rdpq_set_mode_fill (player->color);
  for (int i = 0; i < PIECE_SIZE; i++)
    {
      if (player->piece_buffer[i] == 1)
        {
          int col = player->cursor_x + (i % PIECE_COLS);
          int row = player->cursor_y + (i / PIECE_COLS);
          board_draw_tile (col, row, player->color);
        }
    }
}

void
player_set_cursor (PlayerState *player, int x, int y)
{
  bool in_bounds = false;
  while (!in_bounds)
    {
      in_bounds = true;
      for (int row = 0; row < PIECE_ROWS; row++)
        {
          for (int col = 0; col < PIECE_COLS; col++)
            {
              int index = row * PIECE_COLS + col;
              if (player->piece_buffer[index] == 1)
                {
                retry_bound_check:
                  int new_x = x + col;
                  int new_y = y + row;
                  if (new_x < 0)
                    {
                      x += 1;
                      in_bounds = false;
                      goto retry_bound_check;
                    }
                  else if (new_x >= BOARD_COLS)
                    {
                      x -= 1;
                      in_bounds = false;
                      goto retry_bound_check;
                    }
                  if (new_y < 0)
                    {
                      y += 1;
                      in_bounds = false;
                      goto retry_bound_check;
                    }
                  else if (new_y >= BOARD_ROWS)
                    {
                      y -= 1;
                      in_bounds = false;
                      goto retry_bound_check;
                    }
                }
            }
        }
    }
  player->cursor_x = x;
  player->cursor_y = y;
}

int
player_remaining_pieces (PlayerState *player)
{
  int remaining = PIECE_COUNT;
  for (int i = 0; i < PIECE_COUNT; i++)
    {
      remaining -= player->pieces_used[i];
    }
  return remaining;
}

bool
player_change_piece (PlayerState *player, int piece_index)
{
  assert (piece_index >= 0 && piece_index < PIECE_COUNT);
  if (player->pieces_used[piece_index] == 0)
    {
      player->piece_index = piece_index;
      memcpy (player->piece_buffer, PIECES[piece_index],
              sizeof (player->piece_buffer));
      return true;
    }
  return false;
}

void
player_next_piece (PlayerState *player, int incr)
{
  int next_piece = player->piece_index + incr;
  // Wrap-around negative values
  while (next_piece < 0)
    {
      next_piece += PIECE_COUNT;
    }
  // Wrap-around overflowing positive values
  while (next_piece >= PIECE_COUNT)
    {
      next_piece -= PIECE_COUNT;
    }
  bool result = player_change_piece (player, next_piece);
  // The requested piece is unavailable, try the next one
  if (!result && player_remaining_pieces (player) > 0)
    {
      player_next_piece (player, incr > 0 ? 1 : -1);
    }
}

void
player_rotate_piece_right (PlayerState *player)
{
  int new_piece_buffer[PIECE_SIZE];
  for (int i = 0; i < PIECE_SIZE; i++)
    {
      int x = i % PIECE_COLS;
      int y = i / PIECE_COLS;
      int new_x = PIECE_COLS - 1 - y;
      int new_y = x;
      new_piece_buffer[new_y * PIECE_COLS + new_x] = player->piece_buffer[i];
    }
  memcpy (player->piece_buffer, new_piece_buffer,
          sizeof (player->piece_buffer));
}

void
player_rotate_piece_left (PlayerState *player)
{
  int new_piece_buffer[PIECE_SIZE];
  for (int i = 0; i < PIECE_SIZE; i++)
    {
      int x = i % PIECE_COLS;
      int y = i / PIECE_COLS;
      int new_x = y;
      int new_y = PIECE_ROWS - 1 - x;
      new_piece_buffer[new_y * PIECE_COLS + new_x] = player->piece_buffer[i];
    }
  memcpy (player->piece_buffer, new_piece_buffer,
          sizeof (player->piece_buffer));
}
