#ifndef __AI_H
#define __AI_H

#include <t3d/t3dmath.h>

#include "../../../core.h"
#include "common.hpp"
#include "player.hpp"

constexpr float AITemperature = 0.06f;
constexpr float AIStability = 0.98f;
constexpr float AIActionRateSecond = 0.1;

class AI
{
    private:
        float aiActionTimer = 0;

    public:
        Direction calculateFireDirection(Player&, float deltaTime, std::vector<Player> &players);
        void calculateMovement(Player&, float deltaTime, std::vector<Player> &players, T3DVec3 &inputDirection);
};

#endif // __AI_H

