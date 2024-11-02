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

        bool gameFinished;

        std::vector<PlayerOtherData> playerOtherData;
        std::vector<PlayerGameplayData> playerGameplayData;

        // Player calculations
        void simulatePhysics(PlayerGameplayData &gameplayData, PlayerOtherData &other, uint32_t id, float deltaTime);
        void handleActions(PlayerGameplayData &gameplayData, uint32_t id);
        void renderPlayer(PlayerGameplayData &gameplayData, PlayerOtherData &other, uint32_t id, T3DViewport &viewport);

    public:
        GameplayController();
        void render(float deltaTime, T3DViewport &viewport);
        void fixedUpdate(float deltaTime);
};

#endif // __GAMEPLAY_H