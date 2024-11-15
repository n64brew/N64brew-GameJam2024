#ifndef __GAMESTATE_H
#define __GAMESTATE_H

struct GameState
{
    float gameTime;
    bool isCountdown;
    bool gameFinished;

    T3DVec3 avPos;
};

#endif // __GAMESTATE_H