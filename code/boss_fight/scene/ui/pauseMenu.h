/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

class Scene;

class PauseMenu
{
  public:
    bool isPaused{false};

    PauseMenu();
    ~PauseMenu();

    void update(Scene &scene, float deltaTime);
    void draw();
};