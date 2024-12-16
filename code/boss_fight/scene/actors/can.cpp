/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "can.h"
#include "../scene.h"

namespace {
  T3DModel *model{nullptr};
  uint32_t refCount = 0;
  uint8_t particleTimeout = 0;
}

Actor::Can::Can(Scene &scene, const T3DVec3 &pos, uint16_t param)
: Base(scene), basePos{pos}
{
  if(refCount++ == 0) {
    model = t3d_model_load(FS_BASE_PATH "obj/can.t3dm");

    auto it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
    while(t3d_model_iter_next(&it)) {
      rspq_block_begin();
        t3d_model_draw_object(it.object, nullptr);
      it.object->userBlock = rspq_block_end();
    }
  }

  drawMask = 0;
  coll = {
    .center = basePos,
    .radius = 1.7f,
    .callback = [this](Coll::Sphere &sphere) {
      onCollision(sphere);
    },
    .mask = 0xFF,
    .interactType = (uint8_t)(Coll::InteractType::SPHERES),
    .type = Coll::CollType::DESTRUCTABLE,
  };

  matFP = static_cast<T3DMat4FP *>(malloc_uncached(sizeof(T3DMat4FP)));
}

Actor::Can::~Can() {
  scene.getCollScene().unregisterSphere(&coll);
  free_uncached(matFP);

  if(--refCount == 0) {
    t3d_model_free(model);
  }
}

void Actor::Can::onCollision(Coll::Sphere &sphere)
{
  if(sphere.type != Coll::CollType::SWORD)return;

  if(particleTimeout == 0) {
    scene.requestSpawnActor("Part"_u32, sphere.center, 99);
    particleTimeout = 10;
  }

  if(spinTimer > 0)return;

  scene.getAudio().playSFX("SwordHit"_u64, {.volume = 0.8f});
  int coinAmount = 5 + (rand() % 3);
  for(int i=0; i<coinAmount; ++i) {
    scene.requestSpawnActor("Coin"_u32, sphere.center, 1);
  }

  spinTimer = 2.0f;
}

void Actor::Can::update(float deltaTime) {
  if(!checkCulling(130.0f))return;
  if(drawMask == 0) {
    drawMask = DRAW_MASK_3D;
    scene.getCollScene().registerSphere(&coll);
  }

  if(particleTimeout > 0)--particleTimeout;
  spinTimer = fmaxf(spinTimer - deltaTime, 0.0f);

  timer += deltaTime * (1.0f + fminf(1.0f, spinTimer * 2.0f) * 3.0f);
  coll.center = basePos;

  auto matPos = coll.center * COLL_WORLD_SCALE;
  matPos.y += fm_sinf(timer*2.0f)*0.1f;

  t3d_mat4fp_from_srt_euler(matFP,
    T3DVec3{{0.25f, 0.25f, 0.25f}},
    T3DVec3{{fm_sinf(timer)*0.1f, timer, fm_cosf(timer)*0.1f}},
    matPos
  );
}

void Actor::Can::draw3D(float deltaTime) {
  t3d_matrix_set(matFP, true);

  auto it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
  while(t3d_model_iter_next(&it)) {
    t3d_model_draw_material(it.object->material, &scene.t3dState);
    rspq_block_run(it.object->userBlock);
  }
}
