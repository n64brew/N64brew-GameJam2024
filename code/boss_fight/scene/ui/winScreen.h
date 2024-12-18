/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

class WinScreen
{
  public:
    WinScreen();
    ~WinScreen();

    void setWinner(uint8_t winner[4]);
    void draw(float deltaTime);
};