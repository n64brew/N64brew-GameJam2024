/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3dmath.h>

struct Skybox
{
  Skybox();
  ~Skybox();

  void update(const T3DVec3 &camPos, float deltaTime);
  void draw();
};