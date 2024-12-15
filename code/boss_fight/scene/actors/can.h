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
  class Can final : public Base
  {
    private:
      T3DVec3 basePos{};
      T3DMat4FP *matFP{};
      float timer{0.0f};
      float spinTimer{0.0f};

      void onCollision(Coll::Sphere &sphere);

    public:
      Can(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Can() final;

      void update(float deltaTime) final;
      void draw3D(float deltaTime) final;
  };
}