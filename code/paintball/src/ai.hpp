#ifndef __AI_H
#define __AI_H

#include <t3d/t3dmath.h>

#include "../../../core.h"
#include "common.hpp"
#include "player.hpp"
#include "gamestate.hpp"

constexpr float AITemperature = 0.06f;
constexpr float AIUnstable = 0.02f;
constexpr float AIActionRateSecond = 0.2;

class AI
{
    private:
        float aiActionTimer;
        AiDiff difficulty;

        void tryChangeState(Player& player, AIState newState);
    public:
        AI();
        Direction calculateFireDirection(Player&, float deltaTime, std::vector<Player> &players, GameState &state);
        void calculateMovement(Player&, float deltaTime, std::vector<Player> &players, GameState &state, T3DVec3 &inputDirection);
};

#endif // __AI_H

