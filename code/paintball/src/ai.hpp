#ifndef __AI_H
#define __AI_H

#include <t3d/t3dmath.h>

#include "../../../core.h"
#include "common.hpp"
#include "player.hpp"

class AI
{
    private:

    public:
        Direction calculateFireDirection(Player&, float deltaTime, std::vector<Player> &players);
        void calculateMovement(Player&, float deltaTime, std::vector<Player> &players, T3DVec3 &inputDirection);
};

#endif // __AI_H

