#ifndef GAMEJAM2024_LANDGRAB_PLAYER_H
#define GAMEJAM2024_LANDGRAB_PLAYER_H

#include "board.h"
#include "global.h"
#include "piece.h"

typedef struct Player
{
  PlyNum plynum;
  int score;
  color_t color;
  sprite_t *cursor_sprite;
  int cursor_col;
  int cursor_row;
  int piece_index;
  Cell piece_buffer[PIECE_SIZE];
  bool pieces_used[PIECE_COUNT];
  float pulse_sine_x;
  float pulse_sine_y;
} Player;

void player_init (Player *player, PlyNum plynum);

void player_cleanup (Player *player);

bool player_loop (Player *player, bool active);

bool player_loop_ai (Player *player, bool active);

void player_render (Player *player, bool active);

bool player_set_cursor (Player *player, int col, int row);

int player_remaining_pieces (Player *player);

bool player_change_piece (Player *player, int piece_index);

void player_incr_piece (Player *player, int incr);

void player_incr_value (Player *player, int incr);

void player_flip_piece_right (Player *player);

void player_flip_piece_left (Player *player);

#endif
