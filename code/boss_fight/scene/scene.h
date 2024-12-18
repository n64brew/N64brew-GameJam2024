/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include "camera.h"
#include "player.h"
#include "../collision/scene.h"
#include "../collision/navPoints.h"
#include "playerAI.h"
#include "actors/base.h"
#include "../render/skybox.h"
#include "../render/ptSystem.h"
#include "../render/ptSprites.h"
#include "../render/culledModel.h"
#include "ui/playerMarker.h"
#include "ui/progBar.h"
#include "ui/pauseMenu.h"
#include "../audio/audioManager.h"
#include "cutscene.h"
#include "ui/winScreen.h"
#include "../debug/overlay.h"

class Scene
{
  private:
    struct PointLight {
      T3DVec3 pos{};
      float strength{0.0f};
      color_t color{};
    };

    struct ActorSpawnReq {
      T3DVec3 pos;
      uint32_t type;
      uint16_t param;
    };

    enum class State : uint8_t {
      INTRO = 0, GAME, GAME_OVER
    };

    CulledModel mapModel;
    Coll::Mesh *collMesh{};
    Coll::MeshInstance collMeshInstance{.mesh = collMesh};
    Coll::Scene collScene{};
    Coll::NavPoints navPoints{};

    PTSprites ptCoins;
    PTSprites ptSpark;
    PTSprites ptSwirl;

    Cutscene cutsceneIntro{};
    Cutscene cutsceneGame{};
    Cutscene cutsceneOutro{};

    Skybox skybox{};
    ProgBar progBar{};
    PlayerMarker playerMarker{};
    WinScreen winScreen{};
    PauseMenu pauseMenu{};
    AudioManager audioManager{};
    Debug::Overlay debugOvl{};

    T3DViewport viewport{};
    Camera cam{};
    float camStartPosX{};
    float camEndPosX{};

    InputState input[4];

    Player players[4]{
      Player(0, *this), Player(1, *this),
      Player(2, *this), Player(3, *this)
    };
    PlayerAI playerAI[4]{
      {players[0], *this},
      {players[1], *this},
      {players[2], *this},
      {players[3], *this},
    };
    uint32_t currMostCoins{0};

    std::vector<Actor::Base*> actors{};

    std::vector<T3DVec3> respawnPoints{};
    std::vector<ActorSpawnReq> actorSpawnReqs{};

    PointLight pointLights[4];
    uint32_t pointLightCount{0};

    Math::Timer fadeTimer{};
    Math::Timer uiBarTimer{};
    Math::Timer titleGoTimer{};

    // Map
    State state{State::INTRO};

    surface_t *currentFB{};
    surface_t *lastFB{};
    uint8_t wantsExit{false};
    uint8_t overrideInput{false};
    uint8_t hadBossSpawn{false};
    uint8_t hasBgmStopped{false};

    void updateGame(float deltaTime);
    void updateVisibility();

    void initCutscenes();
    void changeState(State newState);
    void loadScene(const char* path);

    void overrideInputs(const InputState &inputState) {
      overrideInput = true;
      for(auto & i : input)i = inputState;
    }

    void spawnActor(uint32_t type, const T3DVec3 &pos, uint16_t param);
  public:
    T3DModelState t3dState{};
    uint32_t frameIdx{0};

    long ticksActorUpdate{0};
    long ticksCull{0};
    uint32_t activeActorCount{0};
    uint32_t drawActorCount{0};
    bool forceAI{false};
    bool followPlayer{false};
    bool autoScroll{true};

    void update(float deltaTime);
    void draw(float deltaTime);

    void requestGameEnd() {
      changeState(State::GAME_OVER);
    }

    void requestSpawnActor(uint32_t type, const T3DVec3 &pos, uint16_t param = 0) {
      actorSpawnReqs.push_back({pos, type, param});
    }

    void addPointLight(const T3DVec3 &pos, float strength, color_t color) {
      if(pointLightCount < 4) {
        pointLights[pointLightCount++] = {pos, strength, color};
      }
    }

    Coll::Scene& getCollScene() { return collScene; }
    Coll::NavPoints& getNavPoints() { return navPoints; }

    PTSprites& getPTCoins() { return ptCoins; }
    PTSprites& getPTSpark() { return ptSpark; }
    PTSprites& getPTSwirl() { return ptSwirl; }

    AudioManager& getAudio() { return audioManager; }

    const Player &getPlayer(int index) const { return players[index]; }
    const std::vector<Actor::Base*>& getActors() const { return actors; }
    Camera& getCamera() { return cam; }

    const T3DVec3& getClosesRespawn(const T3DVec3 &pos) const;

    surface_t *getCurrentFB() const { return currentFB; }
    surface_t *getLastFB() const { return lastFB; }

    void requestExit() { wantsExit = true; }

    Scene();
    ~Scene();
};