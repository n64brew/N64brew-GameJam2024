/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3dmath.h>
#include <vector>
#include <functional>

class Scene;

namespace Debug
{
  class Overlay
  {
    private:
      struct MenuItem {
        const char *text{};
        int value{};
        bool isBool{};
        std::function<void(MenuItem&)> onChange{};
      };

      struct Menu {
        std::vector<MenuItem> items{};
        int currIndex;
      };

      Menu menu{};

    public:
      void draw(Scene &scene, int triCount, float deltaTime);
  };
}