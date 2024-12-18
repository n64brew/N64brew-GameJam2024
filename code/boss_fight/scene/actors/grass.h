/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include "base.h"
#include "../../collision/shapes.h"
#include "../../render/ptSystem.h"

namespace Actor
{
  class Grass final : public Base
  {
    private:
      PTSystem ptSystem{0};
      PTSystem ptFX[4]{{16}, {16}, {16}, {16}};
      float fxCooldown{0.0f};
      int32_t spawnThreshold{};

      void updateFX(float deltaTime);
    public:
      Grass(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Grass() final;

      void update(float deltaTime) final;
      void drawPtx(float deltaTime) final;

      void drawDebug() final;
  };
}