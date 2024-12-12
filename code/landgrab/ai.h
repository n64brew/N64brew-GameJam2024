#ifndef GAMEJAM2024_LANDGRAB_AI_H
#define GAMEJAM2024_LANDGRAB_AI_H

#include "global.h"
#include "player.h"

void ai_init (void);
void ai_cleanup (void);
bool ai_try (Player *player);

#endif // GAMEJAM2024_LANDGRAB_AI_H
