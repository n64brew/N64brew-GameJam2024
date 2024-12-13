#include "player.h"
#include "ai.h"
#include "board.h"
#include "color.h"
#include "minigame.h"
#include "sfx.h"

#define PLAYER_MOVE_DELAY 0.15f

#define BONUS_USED_ALL_PIECES 15
#define BONUS_MONOMINO_FINAL_PIECE 20

#define SPRITE_CURSOR_P1 "rom:/landgrab/cursor_p1.rgba16.sprite"
#define SPRITE_CURSOR_P2 "rom:/landgrab/cursor_p2.rgba16.sprite"
#define SPRITE_CURSOR_P3 "rom:/landgrab/cursor_p3.rgba16.sprite"
#define SPRITE_CURSOR_P4 "rom:/landgrab/cursor_p4.rgba16.sprite"

static void
player_clear_piece (Player *player)
{
  memset (player->piece_buffer, CELL_EMPTY, sizeof (player->piece_buffer));
}

static void
player_reconstrain_cursor (Player *player)
{
  player_set_cursor (player, player->cursor_col, player->cursor_row);
}

static void
player_move_cursor (Player *player, int col, int row, bool active)
{
  player_set_cursor (player, col, row);
  player->move_delay = PLAYER_MOVE_DELAY;
  if (active)
    {
      sfx_play (SFX_CLICK);
    }
}

static int
player_score (Player *player)
{
  int score = 0;
  // Each unused square costs you a point
  for (size_t i = 0; i < PIECE_COUNT; i++)
    {
      if (!player->pieces_used[i])
        {
          score -= PIECES[i].value;
        }
    }

  // Bonuses are applied when the player uses all their pieces
  if (player->pieces_left == 0)
    {
      score += BONUS_USED_ALL_PIECES;
      if (player->monomino_final_piece)
        {
          score += BONUS_MONOMINO_FINAL_PIECE;
        }
    }

  return score;
}

static void
player_incr_piece (Player *player, int incr)
{
  // Bail if there are no pieces left
  if (player->pieces_left == 0)
    {
      player_clear_piece (player);
      return;
    }

  int desired_index = player->piece_index + incr;
  // Wrap-around negative values
  while (desired_index < 0)
    {
      desired_index += PIECE_COUNT;
    }
  // Wrap-around overflowing positive values
  while (desired_index >= PIECE_COUNT)
    {
      desired_index -= PIECE_COUNT;
    }
  bool result = player_change_piece (player, desired_index);
  // The requested piece is unavailable, try the next one
  if (!result)
    {
      player_incr_piece (player, incr + (incr > 0 ? 1 : -1));
    }
}

static void
player_shuffle_piece (Player *player)
{
  if (player->pieces_left == 0)
    {
      player_clear_piece (player);
      return;
    }

  int value = PIECES[player->piece_index].value;
  size_t shuffle_indexes[PIECE_COUNT];
  size_t shuffle_count = 0;
  for (size_t i = 0; i < PIECE_COUNT; i++)
    {
      if (PIECES[i].value == value && !player->pieces_used[i])
        {
          shuffle_indexes[shuffle_count++] = i;
        }
    }

  if (shuffle_count == 0)
    {
      player_incr_piece (player, +1);
    }
  else
    {
      size_t random_index = shuffle_indexes[rand () % shuffle_count];
      player_change_piece (player, random_index);
    }

  if (rand () % 2)
    {
      player_flip_piece (player);
    }

  if (rand () % 2)
    {
      player_mirror_piece (player);
    }
}

static void
player_maximize_value (Player *player)
{
  if (player->pieces_left == 0)
    {
      player_clear_piece (player);
      return;
    }

  int highest_value = 0;
  size_t highest_value_index = 0;
  for (size_t i = 0; i < PIECE_COUNT; i++)
    {
      if (PIECES[i].value > highest_value && !player->pieces_used[i])
        {
          highest_value = PIECES[i].value;
          highest_value_index = i;
        }
    }

  player_change_piece (player, highest_value_index);
}

static void
player_incr_value (Player *player, int incr)
{
  // Bail if there are no pieces left
  if (player->pieces_left == 0)
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
          player_shuffle_piece (player);
          return;
        }
    }
  // No piece of the requested value was found, try the next value
  player_incr_value (player, incr + (incr > 0 ? 1 : -1));
}

void
player_init (Player *player, PlyNum plynum)
{
  memset (player, 0, sizeof (Player));
  player->plynum = plynum;
  player->pieces_left = PIECE_COUNT;
  player->score = player_score (player);
  player_shuffle_piece (player);
  // Start everyone in their corners
  switch (plynum)
    {
    case PLAYER_1:
      player->cursor_sprite = sprite_load (SPRITE_CURSOR_P1);
      player_set_cursor (player, -PIECE_COLS, -PIECE_ROWS);
      break;
    case PLAYER_2:
      player->cursor_sprite = sprite_load (SPRITE_CURSOR_P2);
      player_set_cursor (player, BOARD_COLS + PIECE_COLS, -PIECE_ROWS);
      break;
    case PLAYER_3:
      player->cursor_sprite = sprite_load (SPRITE_CURSOR_P3);
      player_set_cursor (player, -PIECE_COLS, BOARD_ROWS + PIECE_ROWS);
      break;
    case PLAYER_4:
      player->cursor_sprite = sprite_load (SPRITE_CURSOR_P4);
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

PlayerTurnResult
player_loop (Player *player, bool active, float deltatime)
{
  joypad_port_t port = core_get_playercontroller (player->plynum);
  joypad_buttons_t pressed = joypad_get_buttons_pressed (port);
  joypad_8way_t d = joypad_get_direction (port, JOYPAD_2D_LH);

  if (pressed.start)
    {
      return PLAYER_TURN_PAUSE;
    }

  if (player->pieces_left == 0)
    {
      return PLAYER_TURN_PASS;
    }

  if (player->move_delay <= 0.0f)
    {

      if (JOYPAD_8WAY_ANY_UP (d))
        {
          player_move_cursor (player, player->cursor_col,
                              player->cursor_row - 1, active);
        }
      else if (JOYPAD_8WAY_ANY_DOWN (d))
        {
          player_move_cursor (player, player->cursor_col,
                              player->cursor_row + 1, active);
        }

      if (JOYPAD_8WAY_ANY_LEFT (d))
        {
          player_move_cursor (player, player->cursor_col - 1,
                              player->cursor_row, active);
        }
      else if (JOYPAD_8WAY_ANY_RIGHT (d))
        {
          player_move_cursor (player, player->cursor_col + 1,
                              player->cursor_row, active);
        }
    }
  else
    {
      player->move_delay -= deltatime;
    }

  if (pressed.l || pressed.z)
    {
      player_mirror_piece (player);
    }
  else if (pressed.r)
    {
      player_flip_piece (player);
    }

  if (pressed.c_left)
    {
      player_incr_piece (player, -1);
    }
  else if (pressed.c_right)
    {
      player_incr_piece (player, +1);
    }

  if (pressed.c_up)
    {
      player_incr_value (player, +1);
    }
  else if (pressed.c_down)
    {
      player_incr_value (player, -1);
    }

  if (active && pressed.a)
    {
      if (player_place_piece (player))
        {
          // This is the end of the player's turn
          return PLAYER_TURN_END;
        }
    }

  if (active && pressed.b)
    {
      // Player has passed their turn
      return PLAYER_TURN_PASS;
    }

  // This is not the end of the player's turn
  return PLAYER_TURN_CONTINUE;
}

PlayerTurnResult
player_loop_ai (Player *player, bool active, float deltatime)
{
  if (active)
    {
      if (player->ai_delay < 1.0f)
        {
          player->ai_delay += deltatime;
          return PLAYER_TURN_CONTINUE;
        }
      else
        {
          return ai_try (player);
        }
    }
  else
    {
      player->ai_delay = 0;
      return PLAYER_TURN_CONTINUE;
    }
}

static void
player_render_piece (Player *player, bool active)
{
  PlyNum p = player->plynum;
  Cell *piece = player->piece_buffer;
  color_t draw_color = PLAYER_COLORS[player->plynum];
  color_t hint_color = RGBA32 (0x50, 0x50, 0x50, 0x80);

  if (active)
    {
      player->pulse_sine_x += 0.1f;
      player->pulse_sine_y = fabs (sinf (player->pulse_sine_x));
      draw_color
          = color_between (draw_color, COLOR_WHITE, player->pulse_sine_y);
    }
  else
    {
      draw_color.a = 0x20;
    }

  rdpq_set_mode_standard ();
  rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
  rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);

  for (size_t i = 0; i < PIECE_SIZE; i++)
    {
      if (piece[i] == CELL_FILLED)
        {
          int col = player->cursor_col + (i % PIECE_COLS);
          int row = player->cursor_row + (i / PIECE_COLS);
          board_render_tile (col, row, draw_color);
          if (active)
            {
              if (!board_is_tile_unclaimed (col, row))
                {
                  board_render_bad_tile_marker (col, row);
                }
              else
                {
                  if (board_is_tile_valid (col - 1, row)
                      && (col == 0 || piece[i - 1] == CELL_EMPTY))
                    {
                      if (board_is_tile_unclaimed (col - 1, row))
                        {
                          board_render_tile (col - 1, row, hint_color);
                        }
                      if (board_is_tile_claimed (col - 1, row, p))
                        {
                          board_render_bad_tile_marker (col - 1, row);
                        }
                    }
                  if (board_is_tile_valid (col + 1, row)
                      && (col == PIECE_COLS - 1 || piece[i + 1] == CELL_EMPTY))
                    {
                      if (board_is_tile_unclaimed (col + 1, row))
                        {
                          board_render_tile (col + 1, row, hint_color);
                        }
                      if (board_is_tile_claimed (col + 1, row, p))
                        {
                          board_render_bad_tile_marker (col + 1, row);
                        }
                    }
                  if (board_is_tile_valid (col, row - 1)
                      && (row == 0 || piece[i - PIECE_COLS] == CELL_EMPTY))
                    {
                      if (board_is_tile_unclaimed (col, row - 1))
                        {
                          board_render_tile (col, row - 1, hint_color);
                        }
                      if (board_is_tile_claimed (col, row - 1, p))
                        {
                          board_render_bad_tile_marker (col, row - 1);
                        }
                    }
                  if (board_is_tile_valid (col, row + 1)
                      && (row == 0 || piece[i + PIECE_COLS] == CELL_EMPTY))
                    {
                      if (board_is_tile_unclaimed (col, row + 1))
                        {
                          board_render_tile (col, row + 1, hint_color);
                        }
                      if (board_is_tile_claimed (col, row + 1, p))
                        {
                          board_render_bad_tile_marker (col, row + 1);
                        }
                    }
                }
            }
        }
    }
}

static void
player_render_cursor (Player *player, bool active)
{
  const int icon_col = player->cursor_col + (PIECE_COLS / 2);
  const int icon_row = player->cursor_row + (PIECE_ROWS / 2);
  Rect icon_rect = board_get_tile_rect (icon_col, icon_row);

  rdpq_mode_push ();
  {
    rdpq_set_mode_standard ();
    rdpq_mode_filter (FILTER_BILINEAR);
    rdpq_mode_alphacompare (1);
    rdpq_sprite_blit (player->cursor_sprite, icon_rect.x0, icon_rect.y0, NULL);
  }
  rdpq_mode_pop ();

  if (active && !board_is_tile_unclaimed (icon_col, icon_row))
    {
      board_render_bad_tile_marker (icon_col, icon_row);
    }
}

void
player_render (Player *player, bool active)
{
  if (player->pieces_left > 0)
    {
      if (!plynum_is_ai (player->plynum))
        {
          player_render_piece (player, active);
        }
      player_render_cursor (player, active);
    }
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
              int check_col, check_row;
              if (player->piece_buffer[index] == CELL_FILLED)
                {
                recheck_after_bump:
                  check_col = set_col + col;
                  check_row = set_row + row;
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

bool
player_place_piece (Player *player)
{
  CheckPieceResult result = board_check_piece (player);
  if (result.is_valid)
    {
      board_blit_piece (player);
      player->pieces_used[player->piece_index] = true;
      player->pieces_left--;
      if (player->pieces_left == 0 && PIECES[player->piece_index].value == 1)
        {
          player->monomino_final_piece = true;
        }
      player->score = player_score (player);

      // Audible feedback
      sfx_play (SFX_POP);

      // Switch to the next highest-value piece available
      player_maximize_value (player);
      // Shuffle the new piece to make it more interesting
      player_shuffle_piece (player);

      // The piece was placed successfully
      return true;
    }
  else
    {
      // Audible feedback
      sfx_play (SFX_BUZZ);

      // On the player's first turn, the hint is different
      if (player_is_first_turn (player))
        {
          minigame_set_hint ("Piece must touch a corner");
        }
      else if (result.is_touching_faces)
        {
          minigame_set_hint ("Piece can only touch diagonally");
        }
      else if (!result.is_touching_corners)
        {
          minigame_set_hint ("Pieces must touch diagonally");
        }

      // The piece could not be placed
      return false;
    }
}

void
player_flip_piece (Player *player)
{
  int temp[PIECE_SIZE];
  for (int i = 0; i < PIECE_SIZE; i++)
    {
      int col = i % PIECE_COLS;
      int row = i / PIECE_COLS;
      int new_col = PIECE_COLS - 1 - row;
      int new_row = col;
      temp[new_row * PIECE_COLS + new_col] = player->piece_buffer[i];
    }

  memcpy (player->piece_buffer, temp, sizeof (player->piece_buffer));
  player_reconstrain_cursor (player);
}

void
player_mirror_piece (Player *player)
{
  int temp[PIECE_SIZE];
  for (int i = 0; i < PIECE_SIZE; i++)
    {
      int col = i % PIECE_COLS;
      int row = i / PIECE_COLS;
      int new_col = PIECE_COLS - 1 - col;
      int new_row = row;
      temp[new_row * PIECE_COLS + new_col] = player->piece_buffer[i];
    }

  memcpy (player->piece_buffer, temp, sizeof (player->piece_buffer));
  player_reconstrain_cursor (player);
}
