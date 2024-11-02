#ifndef __GAME_H
#define __GAME_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#include "../../../core.h"
#include "../../../minigame.h"

#include "./wrappers.hpp"
#include "./constants.hpp"
#include "./gameplay.hpp"

#include <functional>
#include <memory>

class Game
{
    private:
        Display display;
        T3D t3d;
        T3DViewport viewport;

        // Controllers
        GameplayController gameplayController;

        RDPQFont font;

        // Map
        void setupMap(U::T3DMat4FP &mapMatFP);
        void renderMap();
        U::T3DMat4FP mapMatFP;
        U::RSPQBlock dplMap;
        U::T3DModel modelMap;

    public:
        Game();
        ~Game();
        void render(float deltatime);
        void fixedUpdate(float deltatime);
};

#endif /* __GAME_H */

