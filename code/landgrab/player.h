#ifndef GAMEJAM2024_LANDGRAB_PLAYER_H
#define GAMEJAM2024_LANDGRAB_PLAYER_H

#include "board.h"
#include "global.h"
#include "piece.h"

typedef struct Player
{
  PlyNum plynum;
  int pieces_left;
  bool monomino_final_piece;
  sprite_t *cursor_sprite;
  float move_delay;
  int cursor_col;
  int cursor_row;
  int piece_index;
  Cell piece_buffer[PIECE_SIZE];
  bool pieces_used[PIECE_COUNT];
  float pulse_sine_x;
  float pulse_sine_y;
  float ai_delay;
} Player;

typedef enum
{
  PLAYER_TURN_CONTINUE = 0,
  PLAYER_TURN_PASS,
  PLAYER_TURN_END,
  PLAYER_TURN_PAUSE,

} PlayerTurnResult;

void player_init (Player *player, PlyNum plynum);

void player_cleanup (Player *player);

PlayerTurnResult player_loop (Player *player, bool active, float deltatime);

PlayerTurnResult player_loop_ai (Player *player, bool active, float deltatime);

void player_render (Player *player, bool active);

int player_score (Player *player);

bool player_set_cursor (Player *player, int col, int row);

bool player_change_piece (Player *player, int piece_index);

bool player_place_piece (Player *player);

void player_incr_piece (Player *player, int incr);

void player_incr_value (Player *player, int incr);

void player_flip_piece (Player *player);

void player_mirror_piece (Player *player);

#endif
