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

constexpr int PLAYER_COUNT = 4;
constexpr float speedLimit = 80.f;
constexpr float forceLimit = 60.f;
constexpr float invMass = 10;

struct Player
{
    T3DVec3 pos;
    T3DVec3 accel;
    T3DVec3 velocity;
    float direction;
    std::unique_ptr<rspq_block_t, decltype(&rspq_block_free)> block;
    std::unique_ptr<T3DMat4FP, decltype(&free_uncached)> matFP;
    // TODO: this wrapper is currently heap allocated
    std::unique_ptr<T3DSkeleton, decltype(&t3d_skeleton_destroy)> skel;
};

class PlayerController
{
    private:
        std::vector<Player> players;
        void processInputs(Player &player, uint32_t id, float deltaTime);

        std::unique_ptr<T3DModel, decltype(&t3d_model_free)> model;

    public:
        PlayerController();
        void setup();
        void update(float deltaTime);
        void fixed_update(float);
};

#endif // __PLAYER_H