#ifndef GAMEJAM2024_LANDGRAB_PIECE_H
#define GAMEJAM2024_LANDGRAB_PIECE_H

#include "constants.h"

#define PIECE_ROWS 5
#define PIECE_COLS 5
#define PIECE_SIZE (PIECE_ROWS * PIECE_COLS)

// MAKE SURE YOU UPDATE THIS IF YOU CHANGE PIECES IN `piece.c`!
#define PIECE_COUNT 21

extern const int PIECES[PIECE_COUNT][PIECE_SIZE];

int piece_value (int piece[PIECE_SIZE]);

#endif
