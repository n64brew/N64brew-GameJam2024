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
  class Box final : public Base
  {
    private:
      T3DMat4FP *matFP{};
      float rotAngle{0};
      uint16_t param{0};
      uint8_t isHit{false};

      void onCollision(Coll::Sphere &sphere);
      void breakBox();
      void makeDynamic();

    public:
      Box(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Box() final;

      void update(float deltaTime) final;
      void draw3D(float deltaTime) final;
  };
}