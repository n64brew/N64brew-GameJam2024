#ifndef __GAMEPLAY_H
#define __GAMEPLAY_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <random>

#include "../../../core.h"
#include "../../../minigame.h"

#include "constants.hpp"
#include "wrappers.hpp"
#include "bullet-controller.hpp"
#include "player.hpp"
#include "map.hpp"
#include "ui.hpp"
#include "gamestate.hpp"
#include "ai.hpp"
#include "common.hpp"

constexpr float PlayerInvMass = 10;
constexpr float BulletOffset = 15.f;

class GameplayController
{
    private:
        // Resources
        BulletController bulletController;
        U::T3DModel model;
        U::T3DModel shadowModel;
        U::Sprite arrowSprite {sprite_load("rom:/paintball/arrow.ia4.sprite"), sprite_free};

        // Player data
        std::vector<Player> playerData;

        // Controllers
        std::shared_ptr<MapRenderer> map;
        AI ai;

        // Player calculations
        void simulatePhysics(
            Player &player,
            uint32_t id,
            float deltaTime,
            T3DVec3 &inputDirection
        );
        void handleFire(Player &player, uint32_t id, Direction direction);

    public:
        GameplayController(std::shared_ptr<MapRenderer> map, std::shared_ptr<UIRenderer> ui);
        void newRound();
        const std::vector<Player> &getPlayerData() const;

        void render(float deltaTime, T3DViewport &viewport, GameState &state);
        void renderUI();
        void fixedUpdate(float deltaTime, GameState &state);
};

#endif // __GAMEPLAY_H