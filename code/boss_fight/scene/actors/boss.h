/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include "base.h"
#include "../../collision/shapes.h"
#include "../healthMeter.h"
#include "../../main.h"

namespace Actor
{
  class Boss final : public Base
  {
    private:
      constexpr static int PART_COUNT = 5;

      Coll::Sphere collider[PART_COUNT]{};
      T3DVec3 pos2D{};

      float faceDir{0.0f};
      bool isMoving{false};
      float sprintTimer{0.0f};

      std::array<T3DMat4FP, PART_COUNT> matFP{};
      std::array<T3DMat4FP, PART_COUNT> matFPSpikes{};
      HealthMeter health{100};
      float hurtTimer{0.0f};
      float dieTimer{0.0f};
      bool isVisible{false};

      float time{0.0f};
      float randTimeEnd{5.0f};

      void onCollision(Coll::Sphere &sphere, int index);
      bool hurt(int damage);
      void bossDie();

      InputState updateAI(float deltaTime);

    public:
      Boss(Scene &scene, const T3DVec3 &pos, uint16_t param);
      ~Boss() final;

      bool isHurt() const { return hurtTimer > 0.0f; }
      const Coll::Sphere& getColl(int index) const { return collider[index]; }

      void update(float deltaTime) final;
      void draw2D(float deltaTime) final;
      void draw3D(float deltaTime) final;
  };
}