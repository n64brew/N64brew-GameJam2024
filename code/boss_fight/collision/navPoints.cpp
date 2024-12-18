/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "navPoints.h"

void Coll::NavPoints::addPoint(const T3DVec3 &point) {
  // insert point into the list sorted by X-coord
  auto it = points.begin();
  while(it != points.end() && it->v[0] < point.v[0]) {
    ++it;
  }
  points.insert(it, point);
}

Coll::NavPointsRes Coll::NavPoints::getClosest(const T3DVec3 &pos, float deltaX) const {

  float closestDist2 = 999999999.0f;
  const T3DVec3 *closestPoint = nullptr;
  for(const auto &point : points) {
    if(point.x < (pos.x+deltaX))continue; // ignore points behind the player

    float dist2 = t3d_vec3_len2(point - pos);
    if(dist2 < closestDist2) {
      closestDist2 = dist2;
      closestPoint = &point;
    }
  }

  return {closestPoint, closestDist2};
}
