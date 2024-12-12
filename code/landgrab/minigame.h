#ifndef GAMEJAM2024_LANDGRAB_MINIGAME_H
#define GAMEJAM2024_LANDGRAB_MINIGAME_H

#include "player.h"

typedef enum
{
  MINIGAME_STATE_INIT = 0,
  MINIGAME_STATE_PLAY,
  MINIGAME_STATE_PAUSE,
  MINIGAME_STATE_END
} MinigameState;

extern MinigameState minigame_state;
extern Player players[MAXPLAYERS];

void minigame_set_hint (const char *msg);

#endif
