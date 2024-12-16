/**
* @copyright 2023 - Max Bebök
* @license MIT
*/
#pragma once
#include <functional>
#include <t3d/t3dmath.h>
#include "../utils/math.h"

namespace Coll
{
  struct Sphere;

  namespace TriType {
    constexpr uint8_t FLOOR = 1 << 0;
    constexpr uint8_t WALL = 1 << 1;
    constexpr uint8_t CEIL = 1 << 2;
    constexpr uint8_t SPHERE = 1 << 3;
  }

  namespace InteractType {
    constexpr uint8_t TRI_MESH = 1 << 0;
    constexpr uint8_t SPHERES  = 1 << 1;
    constexpr uint8_t BOUNCY   = 1 << 2;
    constexpr uint8_t FIXED_Y  = 1 << 3;
  }

  enum class CollType : uint8_t {
    PLAYER = 0,
    BOSS_HEAD = 1,
    BOSS_BODY = 2,
    SWORD = 3,
    COIN = 4,
    COIN_MULTI = 5,
    DESTRUCTABLE = 6,
  };

  struct IVec3 {
    int16_t v[3]{};
  };

  struct AABB
  {
    IVec3 min{};
    IVec3 max{};

    bool vsAABB(const AABB &other) const;
    bool vs2DPointY(const IVec3 &pos) const;
  };
  static_assert(sizeof(AABB) == (6 * sizeof(int16_t)));

  struct Sphere
  {
    T3DVec3 center{};
    float radius{};
    T3DVec3 velocity{};
    std::function<void(Sphere&)> callback;
    uint8_t mask{0xFF};
    uint8_t interactType{0};
    uint8_t hitTriTypes{0}; // mask of triangle types the sphere last collided with
    CollType type{};

    Sphere operator*(float scale) const {
      return {
        .center = center * scale,
        .radius = radius * scale
      };
    }

    AABB toAABB() const {
      auto min = (center - T3DVec3{radius, radius, radius});
      auto max = (center + T3DVec3{radius, radius, radius});
      return {
        .min = {{ (int16_t)min.v[0], (int16_t)min.v[1], (int16_t)min.v[2] }},
        .max = {{ (int16_t)max.v[0], (int16_t)max.v[1], (int16_t)max.v[2] }}
      };
    }
  };

  struct CollInfo
  {
    T3DVec3 hitPos{};
    T3DVec3 penetration{};
    T3DVec3 normal{};
    int collCount{};
  };

  struct Triangle
  {
    T3DVec3 normal{};
    T3DVec3* v[3]{};
    AABB aabb{};
  };

  struct Triangle2D {
    Math::Vec2 v[3]{};
  };
}