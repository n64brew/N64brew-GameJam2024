/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include "../main.h"

namespace Shadows
{
  void init();
  void destroy();

  void addShadow(const T3DVec3 &pos, const T3DVec3 &normal, float size, float strength = 1.0f);
  void draw();
  void reset();
}