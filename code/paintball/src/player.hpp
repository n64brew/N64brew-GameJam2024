#ifndef __PLAYER_H
#define __PLAYER_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <vector>

#include "../../../core.h"
#include "../../../minigame.h"

#include "./constants.hpp"
#include "./wrappers.hpp"
#include "./damage.hpp"

class Player
{
    public:
        Player(T3DModel *model, color_t color, T3DVec3 pos);

        T3DVec3 pos;
        T3DVec3 accel;
        T3DVec3 velocity;
        float direction;
        color_t color;
        U::RSPQBlock block;
        U::T3DMat4FP matFP;
        // TODO: this wrapper is currently heap allocated
        U::T3DSkeleton skel;
};

class PlayerController
{
    private:
        DamageController damageController;

        U::T3DModel model;

        std::vector<Player> players;
        void processInputs(Player &player, uint32_t id, float deltaTime);

    public:
        PlayerController();
        void update(float deltaTime);
        void fixed_update(float deltaTime);
};

#endif // __PLAYER_H