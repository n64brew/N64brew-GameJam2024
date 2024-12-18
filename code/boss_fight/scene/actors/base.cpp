/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "base.h"
#include "../scene.h"

bool Actor::Base::checkCulling(float distance) {
  float diffX = coll.center.x*COLL_WORLD_SCALE - scene.getCamera().getTarget().x;
  if(diffX > distance)return false;
  if(diffX < -distance) {
    requestDelete();
    return false;
  }
  return true;
}
