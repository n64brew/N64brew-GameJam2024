/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>

class Scene;

class PlayerMarker
{
  public:
    PlayerMarker();
    ~PlayerMarker();

    void update(Scene &scene, float deltaTime);
    void draw();
};