/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include "base.h"
#include "../../collision/shapes.h"

namespace Actor
{
  class Coin final : public Base
  {
    private:
      T3DVec3 floorNorm{0,1,0};
      float floorPosY{0.0f};
      uint16_t param{0};
      uint8_t timer{0};

      void onCollision(Coll::Sphere &sphere);

      bool isDynamic() const { return param != 0; }
      uint32_t getTimeSliceIdx() const { return param & 0x0F; }
      bool isSpecial() const { return param & 0xF0; }

      void makeDynamic();
      void makeStatic();

    public:
      Coin(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Coin() final;

      void update(float deltaTime) final;
  };
}