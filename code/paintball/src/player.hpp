#ifndef __PLAYER_H
#define __PLAYER_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <vector>
#include <algorithm>

#include "../../../core.h"
#include "../../../minigame.h"

#include "wrappers.hpp"
#include "constants.hpp"
#include "list.hpp"
#include "bullet.hpp"

class GameplayController;
class BulletController;
class Game;
class AI;
class Player
{
    friend class ::GameplayController;
    friend class ::BulletController;
    friend class ::Game;
    friend class ::AI;

    private:
        // GAMEPLAY DATA

        T3DVec3 pos;
        T3DVec3 prevPos;
        // A player can be in any team at any given time
        PlyNum team;
        // When hit by a team, this is set to that color, upon another hit,
        // the player moves to that team
        PlyNum firstHit;
        float temperature;
        int fragCount;

        // OTHER DATA

        // Physics
        T3DVec3 accel;
        T3DVec3 velocity;

        // Renderer
        float direction;
        U::RSPQBlock block;
        U::T3DMat4FP matFP;

        T3D::Skeleton skel;

        T3D::Anim animWalk;

        T3DVec3 screenPos;

        float displayTemperature;
        float timer;

        // AI
        List<::Bullet, 4> incomingBullets;

    public:
        Player(T3DVec3 pos, PlyNum team, T3DModel *model, T3DModel *shadowModel);
        void render(uint32_t id, T3DViewport &viewport, float deltaTime);
        void renderUI(uint32_t id, sprite_t *arrowSprite);
};


#endif // __PLAYER_H

