/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3dmath.h>

namespace Math {
  inline float easeOutCubic(float x) {
    x = 1.0f - x;
    return 1.0f - (x*x*x);
  }

  inline float easeOutSin(float x) {
     return fm_sinf((x * T3D_PI) * 0.5f);
  }

  inline int noise2d(int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return (n * (n * n * 60493 + 19990303) + 89);
  }

  inline float rand01() {
    return (rand() % 4096) / 4096.0f;
  }

  template<typename T>
  inline auto min(T a, T b) { return a < b ? a : b; }

  template<typename T>
  inline auto max(T a, T b) { return a > b ? a : b; }

  inline T3DVec3 randDir3D() {
    T3DVec3 res{{rand01()-0.5f, rand01()-0.5f, rand01()-0.5f}};
    t3d_vec3_norm(&res);
    return res;
  }

  inline T3DVec3 randDir2D() {
    T3DVec3 res{{rand01()-0.5f, 0.0f, rand01()-0.5f}};
    t3d_vec3_norm(&res);
    return res;
  }

  struct Vec2 {
    float v[2];

    Vec2() = default;
    Vec2(float x, float y) : v{x, y} {}

    float &operator[](int i) {
      return v[i];
    }
    const float &operator[](int i) const {
      return v[i];
    }

    float dot(const Vec2 &other) const {
      return v[0] * other.v[0] + v[1] * other.v[1];
    }
  };

  struct Timer
  {
    float value{};
    float target{};

    void update(float deltaTime) {
      if(target < value) {
        value = fmaxf(value - deltaTime, target);
      } else {
        value = fminf(value + deltaTime, target);
      }
    }
  };
}