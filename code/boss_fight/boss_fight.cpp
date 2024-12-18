/**
* @copyright 2024 - Max Bebök
* @license MIT
*/

#include <t3d/tpx.h>
#include "main.h"
#include "scene/scene.h"
#include "utils/memory.h"

namespace {
  Scene *scene{};
}

extern "C" {
  #include "../../core.h"
  #include "../../minigame.h"

  MinigameDef minigame_def = {
    .gamename      = "Coin Rush",
    .developername = "HailToDodongo",
    .description   = "Collect as many coins as possible!",
    .instructions  = "Stick: Move, B: Attack, A: Jump",
  };

  void minigame_init()
  {
    Memory::dumpHeap("minigame_init");
    constexpr resolution_t res{SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_HEIGHT > 240 ? INTERLACE_HALF : INTERLACE_OFF};
    display_init(res, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    t3d_init((T3DInitParams){});
    tpx_init((TPXInitParams){});

    #if RSPQ_PROFILE
      rspq_profile_start();
      debugf("Profiling enabled\n");
    #endif

    scene = new Scene();
  }

  void minigame_fixedloop(float deltatime)
  {
    //debugf("Fixed loop\n");
  }

  void minigame_loop(float deltatime)
  {
    scene->update(deltatime);
    scene->draw(deltatime);
  }

  void minigame_cleanup()
  {
    rspq_wait();
    delete scene;
    tpx_destroy();
    t3d_destroy();
    display_close();
    Memory::dumpHeap("minigame_cleanup");
  }
}