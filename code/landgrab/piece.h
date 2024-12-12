#ifndef GAMEJAM2024_LANDGRAB_PIECE_H
#define GAMEJAM2024_LANDGRAB_PIECE_H

#include "global.h"

#define PIECE_ROWS 5
#define PIECE_COLS 5
#define PIECE_SIZE (PIECE_ROWS * PIECE_COLS)

// MAKE SURE YOU UPDATE THESE IF YOU CHANGE PIECES IN `piece.c`!
#define PIECE_MIN_VALUE 1
#define PIECE_MAX_VALUE 5
#define PIECE_COUNT 21

typedef enum
{
  CELL_EMPTY = 0,
  CELL_FILLED = 1,
} Cell;

typedef struct
{
  int value;
  Cell cells[PIECE_SIZE];
} Piece;

extern const Piece PIECES[PIECE_COUNT];

#endif
