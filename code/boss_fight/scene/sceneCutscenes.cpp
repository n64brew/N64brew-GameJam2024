/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "scene.h"

namespace {
  constexpr float FADE_TIME_MAX = 2.0f;

  void spawnRandomBox(Scene &scene) {
    T3DVec3 spawnPos{Math::rand01() * 3.0f, 5.2f, Math::rand01() * 3.0f};
    spawnPos.z = fminf(spawnPos.z, 3.0f);
    spawnPos.z = fmaxf(spawnPos.z, -3.0f);
    spawnPos = scene.getPlayer(rand() % 4).getPos() + spawnPos;
    scene.requestSpawnActor("WBox"_u32, spawnPos, 1);
  }
  void spawnRandomBoxMulti(Scene &scene, int count) {
    for(int i=0; i<count; ++i) {
      spawnRandomBox(scene);
    }
  }
}

void Scene::initCutscenes()
{
  // Intro, moves players into place and shows breaking a box,
  // enables movement and BGM
  cutsceneIntro
    .event([this]{
      overrideInput = true;
      fadeTimer = {.value = FADE_TIME_MAX, .target = FADE_TIME_MAX};
      titleGoTimer = {.value = 0.0f, .target = 0.0f};
     })
    .wait(0.05f)
    .event([this]{
      followPlayer = true;
      fadeTimer = {.value = FADE_TIME_MAX, .target = 0.0f};
      getAudio().playSFX("FadeIn"_u64, {.volume = 0.8f});
    })
    .wait(0.9f)
    .event([this]{ uiBarTimer.target = 1.0f; })
    .wait(0.8f)
    .event([this]{ overrideInputs({.move = {0.2f, 0, 0}}); })
    .wait(0.09f).event([this]{ input[0] = {.move = {0.55f, 0, 0}}; })
    .wait(0.09f).event([this]{ input[1] = {.move = {0.55f, 0, 0}}; })
    .wait(0.08f).event([this]{ input[2] = {.move = {0.55f, 0, 0}}; })
    .wait(0.09f).event([this]{ input[3] = {.move = {0.55f, 0, 0}}; })
    .wait(0.3f)
    .event([this]{ overrideInputs({.move = {0.6f, 0, 0}, .jump = true}); })
    .wait(0.3f)
    .wait(0.15f).event([this]{ input[0] = {.move = {0.6f, 0, 0}, .attack = true}; })
    .wait(0.15f).event([this]{ input[1] = {.move = {0.6f, 0, 0}, .attack = true}; })
    .wait(0.15f).event([this]{ input[2] = {.move = {0.6f, 0, 0}, .attack = true}; })
    .wait(0.15f).event([this]{ input[3] = {.move = {0.6f, 0, 0}, .attack = true}; })
    .wait(0.1f)
    .wait(0.1f).event([this]{ input[0] = {}; })
    .wait(0.1f).event([this]{ input[1] = {}; })
    .wait(0.1f).event([this]{ input[2] = {}; })
    .wait(0.1f).event([this]{ input[3] = {}; })
    .wait(0.2f)
    .event([this]{
      auto spawnPos = getClosesRespawn(getPlayer(0).getPos()) + T3DVec3{0,5,0};
      requestSpawnActor("WBox"_u32, spawnPos, 1);
    })
    .wait(0.2f)
    .event([this]{
      getAudio().playSFX("Notice"_u64, {.volume = 0.5f});
      for(auto &p : players)p.setAlertIcon(true);
    })
    .wait(0.5f).event([this]{ input[0].jump = true; })
    .wait(0.1f).event([this]{ input[2].jump = true; })
    .wait(0.3f).event([this]{ input[0].jump = false; })
    .wait(0.1f).event([this]{ input[2].jump = false; })
    .wait(0.1f)
    .event([this]{ for(auto &p : players)p.setAlertIcon(false); })
    .wait(0.2f)
    .event([this]{ titleGoTimer.target = 0.6f; })
    .wait(0.5f)
    .event([this]{ getAudio().playInfoSFX("Start"_u64); })
    .wait(0.3f)
    .event([this]{
      overrideInput = false;
      followPlayer = false;
      uiBarTimer.target = 0.0f;
      titleGoTimer.target = 0.0f;
      getAudio().playBGM("Main"_u64);
      changeState(State::GAME);
    });

    //################################################################################//
    //################################################################################//

    // Outro, counts total coins and determines the winner
    // plays winning animation and stop the game
    cutsceneOutro
      .event([this]{
        overrideInputs({});
        followPlayer = true;
        uiBarTimer.target = 1.0f;
        getAudio().playInfoSFX("Start"_u64);
        getAudio().stopBGM();
      })
      .task(2.2f, [this]{
        for(auto &p : players) {p.showCoinCount();}
      })
      .event([this]{
        uint8_t winners[4] = {0};
        for(int i=0; i<4; ++i) {
          debugf("Player %d: %ld / %ld coins\n", i, players[i].getCoinCount(), currMostCoins);
          players[i].showCoinCount();
          winners[i] = players[i].getCoinCount() >= currMostCoins;
          if(winners[i]) {
            core_set_winner((PlyNum)i);
            input[i].jump = true;
            input[i].attack = true;
          }
        }
        winScreen.setWinner(winners);
      })
      .wait(0.5f)
      .event([this]{
        getAudio().playInfoSFX("Winner"_u64);
      })
      .wait(3.5f)
      .event([this]{
        fadeTimer = {.value = 0.0f, .target = FADE_TIME_MAX};
        getAudio().playSFX("FadeOut"_u64, {.volume = 0.8f});
      })
      .wait(2.3f)
      .event([this]{ wantsExit = true; });

      //################################################################################//
      //################################################################################//

      // some time based gameplay events
      cutsceneGame
        .wait(9.0f)
        // intro section
        .event([this]{spawnRandomBox(*this);})
        .wait(1.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(2.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(0.8f)
        .event([this]{spawnRandomBox(*this);})
        .wait(7.0f) // near first bridge
        .event([this]{spawnRandomBox(*this);})
        .wait(7.0f) // over grass
        .event([this]{spawnRandomBox(*this);})
        .wait(16.0f) // half-way to checkerboard
        .event([this]{spawnRandomBox(*this);})
        .wait(12.0f) // over checkerboard
        .event([this]{spawnRandomBox(*this);})
        .wait(1.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(5.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);})
        .wait(3.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);})
        .wait(3.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);})
        .wait(4.0f) // bridge to crystal section
        .event([this]{spawnRandomBox(*this);})
        .wait(4.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(10.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(2.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(8.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(45.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);})
        .wait(7.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);})
        .wait(1.0f)
        .event([this]{spawnRandomBox(*this);})
        .wait(38.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);})
        .wait(2.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);})
        .wait(8.0f)
        .event([this]{spawnRandomBoxMulti(*this, 2);});

        /*.wait(0.01f)
        .task(1000.0f, [this]{ // debug timings
          debugf("T: %f\n", 1000 - cutsceneGame.getLocalTime());
        });*/
}