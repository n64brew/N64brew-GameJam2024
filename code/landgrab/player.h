#ifndef GAMEJAM2024_LANDGRAB_PLAYER_H
#define GAMEJAM2024_LANDGRAB_PLAYER_H

#include "board.h"
#include "constants.h"
#include "piece.h"

typedef struct
{
  PlyNum plynum;
  color_t color;
  int cursor_x;
  int cursor_y;
  int piece_index;
  int piece_buffer[PIECE_SIZE];
  int pieces_used[PIECE_COUNT];
} PlayerState;

void player_init (PlayerState *player, PlyNum plynum);

void player_cleanup (PlayerState *player);

void player_draw (PlayerState *player);

void player_set_cursor (PlayerState *player, int x, int y);

int player_remaining_pieces (PlayerState *player);

bool player_change_piece (PlayerState *player, int piece_index);

void player_next_piece (PlayerState *player, int incr);

void player_rotate_piece_right (PlayerState *player);

void player_rotate_piece_left (PlayerState *player);

#endif
