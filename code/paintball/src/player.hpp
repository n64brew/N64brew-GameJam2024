#ifndef __PLAYER_H
#define __PLAYER_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <vector>
#include <array>

#include "../../../core.h"
#include "../../../minigame.h"

#include "./wrappers.hpp"

constexpr int PlayerCount = 4;
constexpr float speedLimit = 80.f;
constexpr float forceLimit = 60.f;
constexpr float invMass = 10;

class Player
{
    public:
        Player(U::T3DModel &model, color_t color, T3DVec3 pos);

        T3DVec3 pos;
        T3DVec3 accel;
        T3DVec3 velocity;
        float direction;
        U::RSPQBlock block;
        U::T3DMat4FP matFP;
        // TODO: this wrapper is currently heap allocated
        U::T3DSkeleton skel;
};

class PlayerController
{
    private:
        U::T3DModel model;

        std::array<Player, PlayerCount> players;
        void processInputs(Player &player, uint32_t id, float deltaTime);

    public:
        PlayerController();
        void update(float deltaTime);
        void fixed_update(float deltaTime);
};

#endif // __PLAYER_H