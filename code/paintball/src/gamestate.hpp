#ifndef __GAMESTATE_H
#define __GAMESTATE_H

#include "../../../core.h"

enum State
{
    STATE_COUNTDOWN,
    STATE_GAME,
    STATE_LAST_ONE_STANDING,
    STATE_WAIT_FOR_NEW_ROUND,
    STATE_FINISHED,
    STATE_PAUSED,
    STATE_INTRO,
};

struct GameState
{
    State state;
    float timeInState;
    float gameTime;

    int currentRound;

    int scores[MAXPLAYERS];
    PlyNum winner;

    T3DVec3 avPos;
};

#endif // __GAMESTATE_H