#ifndef __GAMEPLAY_H
#define __GAMEPLAY_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <vector>

#include "../../../core.h"
#include "../../../minigame.h"

#include "constants.hpp"
#include "wrappers.hpp"
#include "bullet.hpp"
#include "player.hpp"

class GameplayController
{
    private:
        BulletController bulletController;

        U::T3DModel model;

        std::vector<PlayerOtherData> playerOtherData;
        std::vector<PlayerGameplayData> playerGameplayData;

        void simulatePhysics(PlayerGameplayData &gameplay, PlayerOtherData &other, uint32_t id, float deltaTime);
        void handleActions(PlayerGameplayData &gameplay, uint32_t id);

        void renderPlayer(PlayerGameplayData &gameplay, PlayerOtherData &other, uint32_t id, T3DViewport &viewport);

    public:
        GameplayController();
        void update(float deltaTime, T3DViewport &viewport);
        void fixed_update(float deltaTime);
};

#endif // __GAMEPLAY_H