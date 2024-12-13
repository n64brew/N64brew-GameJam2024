/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "particles.h"
#include "../scene.h"
#include "../../main.h"
#include "../../debug/debugDraw.h"

namespace {
  constexpr float PARTICLE_TIME = 1.2f;
}

Actor::Particles::Particles(Scene &scene, const T3DVec3 &pos, uint16_t param)
: Base(scene)
{
  timer = 0;
  coll.center = pos;
  for(auto &dir: dirs) {
    dir = Math::randDir3D();
    dir.y += 0.75f;
  }

  switch(param) {
    case 0: color = scene.getPTSpark().getColor(); break;
    case 1: color = {0x60,0xA0,0xFF,0xFF}; break;
    case 2: color = {0xFF,0x40,0x40,0xFF}; break;
    default: color = {0xFF,0xFF,0xFF,0xFF}; break;
  }
}

void Actor::Particles::update(float deltaTime) {
  if(timer >= PARTICLE_TIME)requestDelete();
  if(!checkCulling(130.0f))return;

  timer += deltaTime;
  float dirScale = (timer+0.15f) * 16.0f;
  uint16_t seed = (uint32_t)(void*)(this) >> 8;

  for(auto &dir: dirs) {
    auto partPos = coll.center * COLL_WORLD_SCALE + dir * dirScale;
    float partScale = (1.0f - (timer / PARTICLE_TIME)) * 0.8f;
    scene.getPTSpark().add(partPos, seed, color, partScale);

    seed += 0x1234;
    dir.y -= 0.75f * deltaTime;
  }
}

void Actor::Particles::drawDebug() {
  Debug::drawSphere(coll.center * COLL_WORLD_SCALE, 5.0f, {0xFF, 0xFF, 0xFF, 0xFF});
}
