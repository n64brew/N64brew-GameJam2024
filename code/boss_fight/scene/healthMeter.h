/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3d.h>

struct HealthMeter
{
  int value{0};
  float visibleTimer{1.0f};

  void reduce(int amount);
  void increase(int amount);

  inline void update(float deltaTime) {
    visibleTimer = fmaxf(visibleTimer - deltaTime, 0.0f);
  }

  void draw(const T3DVec3 &posScreen, float maxWidth);
};