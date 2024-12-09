#include "player.h"
#include "board.h"
#include "color.h"

void
player_init (Player *player, PlyNum plynum)
{
  player->plynum = plynum;
  player->score = 0;
  player->color = PLAYER_COLORS[plynum];
  player->pulse_sine_x = 0.0f;
  player->pulse_sine_y = 0.0f;
  memset (player->pieces_used, 0, sizeof (player->pieces_used));
  player_change_piece (player, 0);
  switch (plynum)
    {
    case PLAYER_1:
      player->cursor_sprite
          = sprite_load ("rom:/landgrab/cursor_p1.rgba16.sprite");
      player_set_cursor (player, -PIECE_COLS, -PIECE_ROWS);
      break;
    case PLAYER_2:
      player->cursor_sprite
          = sprite_load ("rom:/landgrab/cursor_p2.rgba16.sprite");
      player_set_cursor (player, BOARD_COLS + PIECE_COLS, -PIECE_ROWS);
      break;
    case PLAYER_3:
      player->cursor_sprite
          = sprite_load ("rom:/landgrab/cursor_p3.rgba16.sprite");
      player_set_cursor (player, -PIECE_COLS, BOARD_ROWS + PIECE_ROWS);
      break;
    case PLAYER_4:
      player->cursor_sprite
          = sprite_load ("rom:/landgrab/cursor_p4.rgba16.sprite");
      player_set_cursor (player, BOARD_COLS + PIECE_COLS,
                         BOARD_ROWS + PIECE_COLS);
      break;
    }
}

void
player_cleanup (Player *player)
{
  sprite_free (player->cursor_sprite);
  player->cursor_sprite = NULL;
}

bool
player_loop (Player *player, bool active)
{
  joypad_port_t port = core_get_playercontroller (player->plynum);
  joypad_buttons_t pressed = joypad_get_buttons_pressed (port);

  if (active && pressed.a)
    {
      bool placed = board_place_piece (player);
      if (placed)
        {
          player->pieces_used[player->piece_index] = true;
          player_incr_piece (player, 1);
          // This is the end of the player's turn
          return true;
        }
      // TODO Play a sound if placement failed
      // TODO Provide feedback to the player
    }

  if (pressed.d_up)
    {
      player_set_cursor (player, player->cursor_col, player->cursor_row - 1);
    }
  else if (pressed.d_down)
    {
      player_set_cursor (player, player->cursor_col, player->cursor_row + 1);
    }

  if (pressed.d_left)
    {
      player_set_cursor (player, player->cursor_col - 1, player->cursor_row);
    }
  else if (pressed.d_right)
    {
      player_set_cursor (player, player->cursor_col + 1, player->cursor_row);
    }

  if (pressed.l)
    {
      player_flip_piece_left (player);
    }
  else if (pressed.r)
    {
      player_flip_piece_right (player);
    }

  if (pressed.c_left)
    {
      player_incr_piece (player, -1);
    }
  else if (pressed.c_right)
    {
      player_incr_piece (player, +1);
    }

  // This is not the end of the player's turn
  return false;
}

bool
player_loop_ai (Player *player, bool active)
{
  // TODO: Implement AI
  return active;
}

void
player_render_piece (Player *player)
{
  player->pulse_sine_x += 0.1f;
  player->pulse_sine_y = fabs (sinf (player->pulse_sine_x));
  color_t draw_color
      = color_between (player->color, COLOR_WHITE, player->pulse_sine_y);

  for (int i = 0; i < PIECE_SIZE; i++)
    {
      if (player->piece_buffer[i] == 1)
        {
          int col = player->cursor_col + (i % PIECE_COLS);
          int row = player->cursor_row + (i / PIECE_COLS);
          board_render_tile (col, row, draw_color);
        }
    }
}

void
player_render (Player *player, bool active)
{
  // if (active)
  //   {
  //     player_render_piece (player);
  //   }
  player_render_piece (player);

  int icon_col = player->cursor_col + 2;
  int icon_row = player->cursor_row + 2;
  Rect icon_rect = board_get_tile_rect (icon_col, icon_row);

  rdpq_set_mode_standard ();
  rdpq_mode_filter (FILTER_BILINEAR);
  rdpq_mode_alphacompare (1);
  rdpq_sprite_blit (player->cursor_sprite, icon_rect.x0, icon_rect.y0,
                    &(rdpq_blitparms_t){});
}

bool
player_set_cursor (Player *player, int set_col, int set_row)
{
  // Assume the placement is valid until proven otherwise
  bool valid = true;
  // Ensure the player's piece is in-bounds of the board
  bool in_bounds = false;
  while (!in_bounds)
    {
      in_bounds = true;
      for (int row = 0; row < PIECE_ROWS; row++)
        {
          for (int col = 0; col < PIECE_COLS; col++)
            {
              int index = row * PIECE_COLS + col;
              if (player->piece_buffer[index] == CELL_FILLED)
                {
                recheck_after_bump:
                  int check_col = set_col + col;
                  int check_row = set_row + row;
                  if (check_col < 0)
                    {
                      set_col += 1;
                      in_bounds = valid = false;
                      goto recheck_after_bump;
                    }
                  else if (check_col >= BOARD_COLS)
                    {
                      set_col -= 1;
                      in_bounds = valid = false;
                      goto recheck_after_bump;
                    }
                  if (check_row < 0)
                    {
                      set_row += 1;
                      in_bounds = valid = false;
                      goto recheck_after_bump;
                    }
                  else if (check_row >= BOARD_ROWS)
                    {
                      set_row -= 1;
                      in_bounds = valid = false;
                      goto recheck_after_bump;
                    }
                }
            }
        }
    }
  player->cursor_col = set_col;
  player->cursor_row = set_row;
  // Return whether the requested placement was valid
  return valid;
}

int
player_remaining_pieces (Player *player)
{
  int remaining = PIECE_COUNT;
  for (int i = 0; i < PIECE_COUNT; i++)
    {
      remaining -= player->pieces_used[i];
    }
  return remaining;
}

void
player_reconstrain_cursor (Player *player)
{
  player_set_cursor (player, player->cursor_col, player->cursor_row);
}

void
player_clear_piece (Player *player)
{
  memset (player->piece_buffer, CELL_EMPTY, sizeof (player->piece_buffer));
}

bool
player_change_piece (Player *player, int piece_index)
{
  assert (piece_index >= 0 && piece_index < PIECE_COUNT);
  if (!player->pieces_used[piece_index])
    {
      player->piece_index = piece_index;
      memcpy (player->piece_buffer, PIECES[piece_index].cells,
              sizeof (player->piece_buffer));
      player_reconstrain_cursor (player);
      return true;
    }
  return false;
}

void
player_incr_piece (Player *player, int incr)
{
  if (player_remaining_pieces (player) == 0)
    {
      player_clear_piece (player);
      return;
    }

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
  if (!result)
    {
      player_incr_piece (player, incr > 0 ? 1 : -1);
    }
}

void
player_incr_value (Player *player, int incr)
{
  // Bail if there are no pieces left
  if (player_remaining_pieces (player) == 0)
    {
      player_clear_piece (player);
      return;
    }

  int value = PIECES[player->piece_index].value;
  int desired_value = value + incr;
  // Wrap-around negative values
  while (desired_value < PIECE_MIN_VALUE)
    {
      desired_value += PIECE_MAX_VALUE;
    }
  // Wrap-around overflowing positive values
  while (desired_value > PIECE_MAX_VALUE)
    {
      desired_value -= PIECE_MAX_VALUE;
    }
  // Attempt to find an unused piece of the requested value
  for (size_t i = 0; i < PIECE_COUNT; i++)
    {
      if (PIECES[i].value == desired_value && !player->pieces_used[i])
        {
          player_change_piece (player, i);
          return;
        }
    }
  // No piece of the requested value was found, try the next value
  player_incr_value (player, incr + (incr > 0 ? 1 : -1));
}

void
player_flip_piece_right (Player *player)
{
  int flip_buffer[PIECE_SIZE];
  for (int i = 0; i < PIECE_SIZE; i++)
    {
      int col = i % PIECE_COLS;
      int row = i / PIECE_COLS;
      int new_col = PIECE_COLS - 1 - row;
      int new_row = col;
      flip_buffer[new_row * PIECE_COLS + new_col]
          = player->piece_buffer[i];
    }

  memcpy (player->piece_buffer, flip_buffer,
          sizeof (player->piece_buffer));
  player_reconstrain_cursor (player);
}

void
player_flip_piece_left (Player *player)
{
  int flip_buffer[PIECE_SIZE];
  for (int i = 0; i < PIECE_SIZE; i++)
    {
      int col = i % PIECE_COLS;
      int row = i / PIECE_COLS;
      int new_col = row;
      int new_row = PIECE_ROWS - 1 - col;
      flip_buffer[new_row * PIECE_COLS + new_col]
          = player->piece_buffer[i];
    }

  memcpy (player->piece_buffer, flip_buffer,
          sizeof (player->piece_buffer));
  player_reconstrain_cursor (player);
}
