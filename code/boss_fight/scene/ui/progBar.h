/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

class ProgBar
{
  public:
    ProgBar();
    ~ProgBar();

    void draw(float prog, float deltaTime);
};