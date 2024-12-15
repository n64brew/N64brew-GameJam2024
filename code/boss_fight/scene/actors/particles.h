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
  class Particles final : public Base
  {
    private:
      T3DVec3 dirs[5]{};
      float timer{0.0f};
      color_t color{};

    public:
      Particles(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Particles() final = default;

      void update(float deltaTime) final;
      void drawDebug() final;
  };
}