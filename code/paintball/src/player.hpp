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

struct PlayerGameplayData
{
    T3DVec3 pos;
    color_t color;
};

class PlayerOtherData
{
    public:
        PlayerOtherData(T3DModel *model, const color_t &color);

        // Physics
        T3DVec3 accel;
        T3DVec3 velocity;

        // Renderer
        float direction;
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

        std::vector<PlayerOtherData> playerOtherData;
        std::vector<PlayerGameplayData> playerGameplayData;

        void simulatePhysics(PlayerGameplayData &gameplay, PlayerOtherData &other, uint32_t id, float deltaTime);
        void handleActions(PlayerGameplayData &gameplay, uint32_t id);

        void renderPlayer(PlayerGameplayData &gameplay, PlayerOtherData &other, uint32_t id, T3DViewport &viewport);

    public:
        PlayerController();
        void update(float deltaTime, T3DViewport &viewport);
        void fixed_update(float deltaTime);
};

#endif // __PLAYER_H