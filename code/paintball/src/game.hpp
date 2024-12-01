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
#include "./map.hpp"
#include "./ui.hpp"
#include "./gamestate.hpp"

#include <functional>
#include <memory>

constexpr float MapShrinkTime = 60.f;

class Game
{
    private:
        // Resources
        Display display;
        T3D t3d;
        T3DViewport viewport;
        RDPQFont font;
        U::Timer timer;

        // Map
        // TODO: gameplay controller is probably a better place for this
        std::shared_ptr<MapRenderer> mapRenderer;

        // UI
        std::shared_ptr<UIRenderer> uiRenderer;

        // Controllers
        GameplayController gameplayController;

        // Camera
        T3DVec3 camTarget;
        T3DVec3 camPos;

        GameState state;

        Wav64 sfxStart;
        Wav64 sfxFinish;
        Wav64 sfxLastOne;

        void gameOver();
        void processState();
        void addScores(const std::vector<Player>&);

    public:
        Game();
        ~Game();
        void render(float deltatime);
        void fixedUpdate(float deltatime);
};

#endif /* __GAME_H */

