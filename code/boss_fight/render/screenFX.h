/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3dmath.h>
#include <libdragon.h>

namespace FX
{
  void drawNGonOverlay(int sides, const T3DVec3 &pos, float radiusInner, float radiusOuter, float angle, color_t color);
  void drawBars(float height);
}