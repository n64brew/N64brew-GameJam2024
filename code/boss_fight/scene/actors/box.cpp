/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "box.h"
#include "../scene.h"
#include "../shadows.h"
#include "../../utils/mesh.h"

namespace
{
  T3DModel *model;
  T3DObject *obj;
  uint32_t refCount = 0;
  constexpr float SCALE = 0.35f;
}

Actor::Box::Box(Scene &scene, const T3DVec3 &pos, uint16_t param)
  : Base(scene), param{param}
{
  if(refCount++ == 0) {
    model = t3d_model_load(FS_BASE_PATH "obj/box.t3dm");
    obj = Mesh::recordFirstObject(model);
  }

  float radius = SCALE;
  auto basePos =  param
    ? pos
    : scene.getCollScene().raycastFloor(pos + T3DVec3{0, radius, 0}).hitPos;

  drawMask = 0;
  coll = {
    .center = basePos + T3DVec3{0, radius, 0},
    .radius = radius,
    .callback = [this](Coll::Sphere &sphere) {
      onCollision(sphere);
    },
    .mask = 0xFF,
    .interactType = Coll::InteractType::SPHERES | Coll::InteractType::FIXED_Y,
    .type = Coll::CollType::DESTRUCTABLE,
  };

  scene.getCollScene().registerSphere(&coll);
  matFP = static_cast<T3DMat4FP *>(malloc_uncached(sizeof(T3DMat4FP)));

  t3d_mat4fp_from_srt_euler(matFP,
    T3DVec3{{SCALE, SCALE, SCALE}},
    T3DVec3{{0,0,0}},
    coll.center * COLL_WORLD_SCALE
  );

  if(param) {
    isHit = true;
    makeDynamic();
  }
}

Actor::Box::~Box() {
  scene.getCollScene().unregisterSphere(&coll);
  free_uncached(matFP);
  if(--refCount == 0) {
    t3d_model_free(model);
  }
}

void Actor::Box::onCollision(Coll::Sphere &sphere) {
  if(isHit && sphere.type == Coll::CollType::DESTRUCTABLE) {
    rotAngle = 9999.0f;
    return;
  }

  if(!isHit && sphere.type == Coll::CollType::SWORD) {
    makeDynamic();
    coll.velocity = coll.center - sphere.center;
    t3d_vec3_norm(&coll.velocity);
    coll.velocity *= 6.5f;
    coll.velocity.y = 4.9f + Math::rand01();

    auto midPoint = (coll.center + sphere.center) * (COLL_WORLD_SCALE * 0.5f);
    for(int i=0; i<2; ++i) {
      scene.getPTSwirl().add(midPoint + Math::randDir3D()*4.0f, 32, 0.7f);
    }

    scene.getAudio().playSFX("BoxHit"_u64, coll.center, {.volume = 0.9f, .variation = 64});
  }
}

void Actor::Box::makeDynamic() {
  coll.interactType = Coll::InteractType::SPHERES | Coll::InteractType::TRI_MESH | Coll::InteractType::BOUNCY;
  isHit = true;
  param = rand() % 4;
}

void Actor::Box::breakBox() {
  if(deleteFlag)return;
  scene.getAudio().playSFX("BoxBreak"_u64, coll.center, {.volume = 0.8f});
  scene.getCollScene().unregisterSphere(&coll);

  int coinAmount = 3 + (rand() % 3);
  for(int i=0; i<coinAmount; ++i) {
    scene.requestSpawnActor("Coin"_u32, coll.center, 1);
  }

  auto pt = coll.center * COLL_WORLD_SCALE;
  uint32_t seed = PhysicalAddr(this);
  for(int i=0; i<14; ++i) {
    scene.getPTSwirl().add(pt + Math::randDir3D()*8.0f, seed+i, {0x90, 0x90, 0x90}, 1.0f);
  }

  requestDelete();
}

void Actor::Box::update(float deltaTime)
{
  if(!isHit && !checkCulling(120.0f))return;
  if(coll.center.y < -5.0f)return requestDelete();
  drawMask = DRAW_MASK_3D;

  if(isHit || coll.hitTriTypes == Coll::TriType::SPHERE)
  {
    float angleOffset = param * (T3D_PI * 0.5f);
    t3d_mat4fp_from_srt_euler(matFP,
      T3DVec3{{SCALE, SCALE, SCALE}},
      T3DVec3{{0, angleOffset + rotAngle, rotAngle*0.95f}},
      coll.center * COLL_WORLD_SCALE
    );
  }

  auto res = scene.getCollScene().raycastFloor(coll.center);
  if(isHit) {
    coll.velocity.y -= 17.0f * deltaTime;
    rotAngle += deltaTime * 4.0f;

    if(res.collCount) {
      Shadows::addShadow(res.hitPos * COLL_WORLD_SCALE, {0,1,0}, 1.5f, 1.0f);
    }
    if(coll.hitTriTypes & Coll::TriType::FLOOR || rotAngle > 16.0f) {
      breakBox();
    }

  } else {
    auto bottomPos = coll.center;
    bottomPos.y -= coll.radius;
    if(res.collCount) {
      Shadows::addShadow(bottomPos * COLL_WORLD_SCALE, {0,1,0}, 1.8f, 1.0f);
    } else {
      makeDynamic();
    }
  }
}

void Actor::Box::draw3D(float deltaTime) {
  t3d_model_draw_material(obj->material, &scene.t3dState);
  t3d_matrix_set(matFP, true);
  rspq_block_run(obj->userBlock);
}
