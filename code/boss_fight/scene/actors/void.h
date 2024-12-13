/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3dskeleton.h>
#include "base.h"
#include "../../collision/shapes.h"
#include "../../render/ptSystem.h"

namespace Actor
{
  class Void final : public Base
  {
    private:
      PTSystem ptSystem{64};
      T3DVec3 basePos{};
      T3DMat4FP *matFP{};
      float rotAngle{0};
      float timer{0};
      float growTimer{0};
      float voidStrength{0};
      uint16_t param{0};
      uint8_t isActive{false};

      void onCollision(Coll::Sphere &sphere);

    public:
      Void(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Void() final;

      void update(float deltaTime) final;
      void draw3D(float deltaTime) final;
      void drawPtx(float deltaTime) final;
  };
}