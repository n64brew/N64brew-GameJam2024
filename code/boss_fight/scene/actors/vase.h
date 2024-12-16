/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3dskeleton.h>
#include "base.h"
#include "../../collision/shapes.h"

namespace Actor
{
  class Vase final : public Base
  {
    private:
      T3DMat4FP *matFP{};
      T3DSkeleton skeleton{};
      float timer{0.0f};
      float startY{0.0f};
      uint16_t param{0};

      void onCollision(Coll::Sphere &sphere);

    public:
      Vase(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Vase() final;

      void update(float deltaTime) final;
      void draw3D(float deltaTime) final;
  };
}