/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "../../collision/shapes.h"
#include "../../main.h"

class Scene;

namespace Actor
{
  constexpr uint8_t DRAW_MASK_3D  = 1 << 0;
  constexpr uint8_t DRAW_MASK_2D  = 1 << 1;
  constexpr uint8_t DRAW_MASK_PTX = 1 << 2;

  class Base
  {
    protected:
      Coll::Sphere coll{};
      Scene& scene;

    public:
      uint8_t deleteFlag{false};
      uint8_t drawMask{};

      void requestDelete() { deleteFlag = true; }
      [[nodiscard]] const T3DVec3 &getPos() const { return coll.center; }

      bool checkCulling(float distance);

      explicit Base(Scene& scene) : scene(scene) {}
      virtual ~Base() = default;

      virtual void update(float deltaTime) = 0;

      virtual void draw2D(float deltaTime) {}
      virtual void draw3D(float deltaTime) {}
      virtual void drawPtx(float deltaTime) {}

      virtual void drawDebug() {}
  };
}