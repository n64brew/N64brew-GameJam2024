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

#include "./wrappers.hpp"

struct PlayerGameplayData
{
    PlayerGameplayData(T3DVec3 pos, PlyNum team, std::array<int, MAXPLAYERS> health);
    T3DVec3 pos;
    T3DVec3 prevPos;
    // A player can be in any team at any given time
    PlyNum team;
    std::array<int, MAXPLAYERS> health;
};

class PlayerOtherData
{
    public:
        PlayerOtherData(T3DModel *model);

        // Physics
        T3DVec3 accel;
        T3DVec3 velocity;

        // Renderer
        float direction;
        U::RSPQBlock block;
        U::T3DMat4FP matFP;
        // TODO: this wrapper is currently heap allocated
        U::T3DSkeleton skel;

        U::T3DAnim animWalk;


        T3DVec3 screenPos;
};

#endif // __PLAYER_H

