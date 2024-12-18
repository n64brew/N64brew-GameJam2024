/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <array>

#include "../main.h"
#include "player.h"
#include "actors/boss.h"
#include "actors/coin.h"
#include "../collision/navPoints.h"

class Scene;

class PlayerAI
{
  private:
    enum class Action : uint8_t  {
      PROGRESS,
      COLLECT,
      ATTACK,
      IDLE,
      _SIZE
    };

    Player &player;
    Scene &scene;

    T3DVec3 lastPos{player.getPos()};
    float noProgressTime{0.0f};

    float time{0.0f};
    float randTimeEnd{5.0f};
    bool moveOut{false};
    Coll::NavPointsRes lastNavPoint{};
    Action action{Action::COLLECT};

    float jumpHoldTime{0.0f};
    uint8_t lastFocusPlayer{0};
    uint32_t lastRespawnCounter{0};

    const Player &getClosetPlayer();

    void changeAction();
    void changeAction(Action newAction);

  public:
    PlayerAI(Player &player, Scene &scene)
      : player{player}, scene{scene}
    {}

    InputState update(float deltaTime);
    void debugDraw();
};