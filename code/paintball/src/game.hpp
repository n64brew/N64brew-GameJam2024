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

#include "./player.hpp"
#include "./damage.hpp"
#include "./wrappers.hpp"

#include <functional>
#include <memory>

class Game
{
    private:
        Display display;
        T3D t3d;
        PlayerController playerManager;
        DamageController damageController;

        void timer_callback();
        T3DViewport viewport;
        RDPQFont font;
        U::Timer timer;

        // Map
        U::T3DMat4FP mapMatFP;
        U::RSPQBlock dplMap;
        U::T3DModel modelMap;

        void setupMap(U::T3DMat4FP &mapMatFP);
        void renderMap();

    public:
        Game();
        ~Game();
        void update(float deltatime);
        void fixed_update(float deltatime);
};

#endif /* __GAME_H */

