/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3dmath.h>
#include <vector>

namespace Coll
{
  struct NavPointsRes {
    const T3DVec3 *point{nullptr};
    float dist2{0.0f};
  };

  struct NavPoints
  {
    // points, internally sorted by X-coord
    std::vector<T3DVec3> points{};

    void addPoint(const T3DVec3 &point);
    NavPointsRes getClosest(const T3DVec3 &pos, float deltaX) const;
  };
}