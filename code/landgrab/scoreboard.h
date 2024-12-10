#ifndef GAMEJAM2024_LANDGRAB_SCOREBOARD_H
#define GAMEJAM2024_LANDGRAB_SCOREBOARD_H

#include "global.h"
#include "minigame.h"

typedef struct
{
  PlyNum p;
  int score;
} PlayerScore;

extern PlayerScore scoreboard[MAXPLAYERS];
extern int winners;

void scoreboard_init (void);

void scoreboard_cleanup (void);

void scoreboard_calculate (bool game_over);

void scoreboard_scores_render (void);

void scoreboard_pieces_render (void);

#endif // GAMEJAM2024_LANDGRAB_SCOREBOARD_H
