/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

struct Camera
{
  T3DVec3 pos{};
  T3DVec3 target{};
  T3DVec3 targetDest{};

  float viewAngleVert{};
  float viewAngleHor{};

  void reset();
  void update(T3DViewport &viewport);

  void setTarget(T3DVec3 newPos) {
    targetDest = newPos;
    target = newPos;
  }

  void move(T3DVec3 dir) {
    targetDest += dir;
  }

  [[nodiscard]] const T3DVec3 &getTarget() const { return target; }
  [[nodiscard]] const T3DVec3 &getPos() const { return pos; }
  [[nodiscard]] float getRotY() const;
};