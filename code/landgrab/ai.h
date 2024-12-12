#ifndef GAMEJAM2024_LANDGRAB_AI_H
#define GAMEJAM2024_LANDGRAB_AI_H

#include "player.h"

void ai_reset (Player *player);

PlayerTurnResult ai_try (Player *player);

#endif // GAMEJAM2024_LANDGRAB_AI_H
