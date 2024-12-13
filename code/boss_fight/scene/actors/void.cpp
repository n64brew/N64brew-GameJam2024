/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "void.h"
#include "../scene.h"
#include "../shadows.h"

namespace
{
  constexpr float SCALE = 0.15f;
  constexpr float GROW_TIMER_MAX = 1.0f;
  constexpr float MAX_VOID_TIME = 5.0f;
  constexpr float MOVE_RADIUS = 3.25f;

  T3DModel *model;
  T3DMaterial *matOrb;
  uint32_t refCount = 0;
}

Actor::Void::Void(Scene &scene, const T3DVec3 &pos, uint16_t param)
  : Base(scene), param{param}
{
  if(refCount++ == 0) {
    model = t3d_model_load(FS_BASE_PATH "obj/void.t3dm");

    matOrb = t3d_model_get_material(model, "Orb");

    auto it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
    while(t3d_model_iter_next(&it)) {
      rspq_block_begin();
        t3d_model_draw_object(it.object, nullptr);
      it.object->userBlock = rspq_block_end();
    }
  }

  rotAngle = Math::rand01() * 2.0f;
  float radius = 2.65f * SCALE;
  auto res = scene.getCollScene().raycastFloor(pos + T3DVec3{0, radius, 0});

  drawMask = 0;
  coll = {
    .center = res.hitPos + T3DVec3{0, radius, 0},
    .radius = radius,
    .callback = [this](Coll::Sphere &sphere) {
      onCollision(sphere);
    },
    .mask = 0xFF,
    .interactType = Coll::InteractType::SPHERES,
    .type = Coll::CollType::DESTRUCTABLE,
  };

  basePos = coll.center;
  scene.getCollScene().registerSphere(&coll);
  matFP = static_cast<T3DMat4FP *>(malloc_uncached(sizeof(T3DMat4FP)));

  // particles
  for(uint32_t i=0; i<ptSystem.countMax; ++i) {
    auto *pos = tpx_buffer_get_pos(ptSystem.particles, i);
    pos[0] = (rand() % 128) - 64;
    pos[1] = (rand() % 255) - 128;
    pos[2] = (rand() % 128) - 64;
    tpx_buffer_get_size(ptSystem.particles, i)[0] = 64;
    uint8_t *color = tpx_buffer_get_rgba(ptSystem.particles, i);
    color[0] = 200 + (rand() % 22);
    color[1] = 30 + (rand() % 22);
    color[2] = 200 + (rand() % 22);
    color[3] = 255;
  }
  ptSystem.count = ptSystem.countMax;
}

Actor::Void::~Void() {
  scene.getCollScene().setVoidSphere(param, {}, 0);
  scene.getCollScene().unregisterSphere(&coll);
  free_uncached(matFP);
  if(--refCount == 0) {
    t3d_model_free(model);
  }
}

void Actor::Void::onCollision(Coll::Sphere &sphere) {
  if(isActive)return;

  bool doActivate = sphere.type == Coll::CollType::SWORD;
  if(!doActivate && sphere.type == Coll::CollType::DESTRUCTABLE) {
    doActivate = t3d_vec3_len2(sphere.velocity) > 1.0f;
  }

  if(doActivate) {
    auto midPoint = (coll.center + sphere.center) * (COLL_WORLD_SCALE * 0.5f);
    for(int i=0; i<2; ++i) {
      scene.getPTSwirl().add(midPoint + Math::randDir3D()*4.0f, 32, 0.7f);
    }

    scene.getCollScene().unregisterSphere(&coll);
    isActive = true;
    growTimer = 0.0f;

    scene.getAudio().playSFX("SwordHit"_u64, {.volume = 0.8f});
    scene.getAudio().playSFX("VoidOn"_u64, {.volume = 1.0f, .variation = 64});
  }
}

void Actor::Void::update(float deltaTime)
{
  if(!checkCulling(120.0f))return;
  if(coll.center.y < -5.0f)return requestDelete();
  drawMask = DRAW_MASK_3D | (isActive ? DRAW_MASK_PTX : 0);

  rotAngle += deltaTime * 1.5f;

  if(param == 0) {
    coll.center = basePos;
  } else {
    float distX = fm_sinf(rotAngle*0.4f) * MOVE_RADIUS;
    float distZ = fm_cosf(rotAngle*0.4f) * MOVE_RADIUS;
    coll.center = basePos + T3DVec3{distX, 0, distZ};
  }

  auto bottomPos = coll.center;
  bottomPos *= COLL_WORLD_SCALE;

  float scale = SCALE;
  voidStrength = 0.08f + fm_sinf(timer*2.5f) * 0.005f;

  if(isActive) {
    timer += deltaTime;

    if(timer < MAX_VOID_TIME) {
      growTimer = fminf(growTimer + deltaTime, GROW_TIMER_MAX);
      voidStrength *= Math::easeOutCubic(growTimer);
    } else {
      growTimer = fmaxf(growTimer - deltaTime, 0.0f);
      voidStrength *= Math::easeOutCubic(growTimer);
      if(growTimer == 0.0f) {
        isActive = false;

        uint32_t coinCount = 4 + (rand() % 4);
        for(uint32_t i=0; i<coinCount; ++i) {
          scene.requestSpawnActor("Coin"_u32, coll.center, 1);
        }

        scene.getAudio().playSFX("VoidOff"_u64, {.volume = 1.0f, .variation = 32});
        coll.velocity = {};
        coll.center = basePos;
        scene.getCollScene().registerSphere(&coll);
        timer = 0.0f;

        return;
      }
    }

    scale *= (1.0f - Math::easeOutCubic(growTimer));

    scene.getCollScene().setVoidSphere(param, coll.center, voidStrength * 31.0f);
    scene.addPointLight(bottomPos + T3DVec3{0,2,0}, -voidStrength, {0xFF,0x00,0x80,0xFF});

    t3d_mat4fp_from_srt_euler(
      ptSystem.mat,
      T3DVec3{{voidStrength*6, voidStrength*5.2f, voidStrength*6}},
      T3DVec3{{0, 0, 0}},
      coll.center * COLL_WORLD_SCALE - T3DVec3{0, 10.0f, 0}
    );

  } else {
    scene.getCollScene().setVoidSphere(param, coll.center, 0.0f);
    auto shadowPos = bottomPos;
    shadowPos.y -= coll.radius * COLL_WORLD_SCALE;
    Shadows::addShadow(shadowPos, {0,1,0}, 1.8f, 1.0f);
  }

  t3d_mat4fp_from_srt_euler(
    matFP,
    T3DVec3{{scale, scale, scale}},
    T3DVec3{{0, rotAngle, 0}},
    coll.center * COLL_WORLD_SCALE + T3DVec3{0, 3.0f + fm_sinf(rotAngle*5.0f)*1.1f,0}
  );

  // particles
  if(isActive) {
    for(uint32_t i=0; i<ptSystem.countMax; ++i) {
      auto *pos = tpx_buffer_get_pos(ptSystem.particles, i);
      pos[1] += 2;
      auto *size = tpx_buffer_get_size(ptSystem.particles, i);
      if(pos[1] < 0) {
        *size = 0;
      } else if(pos[1] < 40) {
        *size += 4;
      } else {
        *size -= 1;
      }
    }
  }

  // texture animation
  matOrb->textureA.t.low += deltaTime * 8.0f;
  if(matOrb->textureA.t.low > 64.0f)matOrb->textureA.t.low -= 64.0f;
  matOrb->textureA.t.low += deltaTime * 8.0f;
  if(matOrb->textureA.t.low > 64.0f)matOrb->textureA.t.low -= 64.0f;
  matOrb->textureB.s.low += deltaTime * 4.0f;
  if(matOrb->textureB.s.low > 64.0f)matOrb->textureB.s.low -= 64.0f;
  matOrb->textureB.t.low += deltaTime * 4.0f;
  if(matOrb->textureB.t.low > 64.0f)matOrb->textureB.t.low -= 64.0f;
}

void Actor::Void::draw3D(float deltaTime) {
  float scale = 1.0f - growTimer;
  if(scale > 0.001f) {
    t3d_matrix_set(matFP, true);

    auto it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
    while(t3d_model_iter_next(&it)) {
      t3d_model_draw_material(it.object->material, &scene.t3dState);
      rspq_block_run(it.object->userBlock);
    }
  }
}

void Actor::Void::drawPtx(float deltaTime) {
  tpx_state_set_scale(0.15f, 10.8f * voidStrength);
  ptSystem.draw();
}
