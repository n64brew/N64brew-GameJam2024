/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "camera.h"

namespace {
  constexpr float aspect = 240.0f / 320.0f;
  constexpr float vpNear = 3.0f;
  constexpr float vpFar = 280.0f;
  constexpr float zoomDepth = -86.0f;
}

void Camera::update(T3DViewport &viewport)
{
  t3d_viewport_set_projection(viewport, T3D_DEG_TO_RAD(45.0f), 5.0f, 150.0f);
  float zoom = 172.0f;
  auto eyeDir = T3DVec3{0,
    sinf(viewAngleVert),
    cosf(viewAngleVert)
  } * zoom;

  t3d_vec3_lerp(target, target, targetDest, 0.5f);
  pos = target + eyeDir;
  t3d_viewport_look_at(viewport, pos, target, T3DVec3{{0, 1, 0}});
}

void Camera::reset() {
  targetDest = target;
  viewAngleVert = T3D_DEG_TO_RAD(30.0f);
  viewAngleHor = T3D_DEG_TO_RAD(0.0f);
  //viewAngleVert = T3D_DEG_TO_RAD(90.0f);
}

float Camera::getRotY() const {
  return viewAngleHor;
}
