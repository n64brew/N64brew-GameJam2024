/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "coin.h"
#include "../scene.h"
#include "../shadows.h"

namespace {
  constexpr float SPAWN_FRAMES = 20;

  constexpr uint32_t TIME_SLICES = 3;
  uint32_t sliceIdx = 0;
  uint8_t hitFxTimeout = 0;
  uint8_t specialSpawn = 0;
}

Actor::Coin::Coin(Scene &scene, const T3DVec3 &pos, uint16_t param)
  : Base(scene), param{param}
{
  drawMask = 0;
  floorPosY = pos.y * COLL_WORLD_SCALE;

  coll = {
    .center = pos + T3DVec3{0,0.4f,0},
    .radius = 0.25f,
    .callback = [this](Coll::Sphere &sphere) {
      onCollision(sphere);
    },
    .mask = 0xFF,
    .interactType = 0,
    .type = Coll::CollType::COIN,
  };

  if(isDynamic()) {
    makeDynamic();
    // dynamic coins may spawn from within the player, add a little delay before collectable
    coll.mask = 0;
    timer = SPAWN_FRAMES;

    // every 20 coins spawn a special one worth more
    if(++specialSpawn > 20) {
      this->param |= 0xF0;
      coll.type = Coll::CollType::COIN_MULTI;
      specialSpawn = 0;
    }

    coll.velocity = Math::randDir2D() * 2.5f;
    coll.velocity.y = 1.5f + Math::rand01();
  }
  scene.getCollScene().registerSphere(&coll);
}

void Actor::Coin::onCollision(Coll::Sphere &sphere) {
  if(sphere.type != Coll::CollType::PLAYER)return;
  requestDelete();

  auto midPoint = (coll.center + sphere.center) * 0.5f;
  scene.requestSpawnActor("Part"_u32, midPoint, isSpecial() ? 1 : 0);
  scene.getAudio().playSFX("CoinGet"_u64, {.volume = 0.4f, .variation = 32});
  //scene.getAudio().playSFX("CoinGet"_u64, coll.center, {.volume = 0.6f, .variation = 32});
}


void Actor::Coin::makeDynamic() {
  ++sliceIdx;
  this->param = 1 + (sliceIdx % TIME_SLICES);
  coll.interactType = (uint8_t)(Coll::InteractType::TRI_MESH | Coll::InteractType::BOUNCY);
  coll.velocity.y = -1;
}

void Actor::Coin::makeStatic() {
  param &= ~0xF;
  coll.interactType = 0; // stop collision if still long enough...
  coll.velocity = {0,0,0};
}

void Actor::Coin::update(float deltaTime)
{
  if(!checkCulling(120.0f)) {
    // sometimes dynamic coins spawn way OOB, @TODO: check why
    if(isDynamic() && !checkCulling(200.0f)) {
      requestDelete();
    }
    return;
  }
  if(isDynamic()) {
    auto vel = t3d_vec3_len2(&coll.velocity);
    if(vel < 0.05f) {
      makeStatic();
    }

    if(coll.interactType != 0) {
      coll.velocity.y -= 6.0f * deltaTime;

      if(!(coll.hitTriTypes & Coll::TriType::FLOOR))
      {
        uint32_t timeSliceIdx = (scene.frameIdx % TIME_SLICES) + 1;
        if(timeSliceIdx == (getTimeSliceIdx())) {
          auto floor = scene.getCollScene().raycastFloor(coll.center);
          if(floor.collCount) {
            floorPosY = floor.hitPos.y * COLL_WORLD_SCALE;
            //floorPosY += 0.5f;
            floorNorm = floor.normal;
          } else {
            floorPosY = -9999.0f;
          }
        }
      }
    }

    if(hitFxTimeout > 0)hitFxTimeout = 0;
    if(hitFxTimeout == 0 && coll.hitTriTypes & Coll::TriType::FLOOR && fabsf(coll.velocity.y) > 0.5f) {
      hitFxTimeout = 4;
      scene.getAudio().playSFX("CoinHit"_u64, coll.center, {.volume = 0.1f, .variation = 64});
    }

    if(timer > 0) {
      --timer;
      coll.mask = timer == 0 ? 0xFF : 0;
    }
  } else {
    // make static coin dynamic to make it fall into a void
    if(scene.getCollScene().isInVoid(coll.center)) {
      makeDynamic();
    }
  }

  auto posWorld = coll.center * COLL_WORLD_SCALE;
  if(isSpecial()) {
    scene.getPTCoins().add(posWorld, (uint32_t)this, {0x60,0xA0,0xFF,0xFF});
  } else {
    scene.getPTCoins().add(posWorld, (uint32_t)this);
  }

  if(floorPosY > -100.f) {
    T3DVec3 shadowPos{posWorld.x, floorPosY, posWorld.z};
    Shadows::addShadow(shadowPos, floorNorm, 0.7f, 0.7f);
  }

  if(coll.center.y < -2.0f) {
    scene.requestSpawnActor("Part"_u32, coll.center, 0);
    requestDelete();
  }
}

Actor::Coin::~Coin() {
  scene.getCollScene().unregisterSphere(&coll);
}
