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
#include "map.hpp"

enum AIState {
    AI_IDLE,
    AI_DEFEND,
    AI_ATTACK,
    AI_RUN,
    AI_CHASE,
    AI_ESCAPE
};

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

    friend void tryChangeState(Player& player, AIState newState);

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

        // Who captured this player?
        int capturer;


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

        bool firstStep;

        // AI
        List<::Bullet, 4> incomingBullets;
        AIState aiState;
        float multiplier;
        float multiplier2;

    public:
        Player(T3DVec3 pos, PlyNum team, T3DModel *model, T3DModel *shadowModel);
        void render(uint32_t id, T3DViewport &viewport, float deltaTime, MapRenderer&);
        void renderUI(uint32_t id, sprite_t *arrowSprite);
        void acceptHit(const Bullet &bullet);
};


#endif // __PLAYER_H

