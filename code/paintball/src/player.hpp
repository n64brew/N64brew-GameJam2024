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
#include "./constants.hpp"

class GameplayController;
class BulletController;
class Game;

namespace Player {
    class OtherData;

    class GameplayData
    {
        friend class ::GameplayController;
        friend class ::BulletController;
        friend class ::Game;
        friend void render(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id, T3DViewport &viewport, float deltaTime);
        friend void renderUI(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id, sprite_t *arrowSprite);

        private:
            T3DVec3 pos;
            T3DVec3 prevPos;
            // A player can be in any team at any given time
            PlyNum team;
            // When hit by a team, this is set to that color, upon another hit,
            // the player moves to that team
            PlyNum firstHit;
            float temperature;
            int fragCount;

        public:
            GameplayData(T3DVec3 pos, PlyNum team);
    };

    class OtherData
    {
        friend class ::GameplayController;
        friend void render(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id, T3DViewport &viewport, float deltaTime);
        friend void renderUI(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id, sprite_t *arrowSprite);

        private:
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

        public:
            OtherData(T3DModel *model, T3DModel *shadowModel);
    };

    void render(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id, T3DViewport &viewport, float deltaTime);
    void renderUI(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id, sprite_t *arrowSprite);
}

#endif // __PLAYER_H

