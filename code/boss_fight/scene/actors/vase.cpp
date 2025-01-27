/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "../scene.h"
#include "vase.h"
#include "../shadows.h"

namespace
{
  T3DModel *model;
  T3DObject *obj;
  T3DObject *objBroken;
  uint32_t refCount = 0;
  T3DVec3 randomRot[5];
}

Actor::Vase::Vase(Scene &scene, const T3DVec3 &pos, uint16_t param)
  : Base(scene), param{param}
{
  if(refCount++ == 0) {
    for(auto &rot : randomRot) {
      rot = Math::randDir3D();
    }

    model = t3d_model_load(FS_BASE_PATH "obj/vase.t3dm");

    obj = t3d_model_get_object(model, "Vase");
    rspq_block_begin();
      t3d_model_draw_object(obj, nullptr);
    obj->userBlock = rspq_block_end();

    objBroken = t3d_model_get_object(model, "Broken");
    rspq_block_begin();
      t3d_model_draw_object(objBroken, (T3DMat4FP*)t3d_segment_placeholder(1));
    objBroken->userBlock = rspq_block_end();
  }

  timer = -9999;
  skeleton = t3d_skeleton_create(model);
  t3d_skeleton_update(&skeleton);

  drawMask = 0;
  coll = {
    .center = pos + T3DVec3{0,0.47f,0},
    .radius = 0.5f,
    .callback = [this](Coll::Sphere &sphere) {
      onCollision(sphere);
    },
    .mask = 0xFF,
    .interactType = (uint8_t)(Coll::InteractType::SPHERES),
    .type = Coll::CollType::DESTRUCTABLE,
  };

  scene.getCollScene().registerSphere(&coll);
  matFP = static_cast<T3DMat4FP *>(malloc_uncached(sizeof(T3DMat4FP)));
  startY = coll.center.y;
}

Actor::Vase::~Vase() {
  t3d_skeleton_destroy(&skeleton);
  scene.getCollScene().unregisterSphere(&coll);
  free_uncached(matFP);
  if(--refCount == 0) {
    t3d_model_free(model);
  }
}

void Actor::Vase::onCollision(Coll::Sphere &sphere) {
  if(sphere.type == Coll::CollType::SWORD) {
    timer = 1.5f;
    scene.getCollScene().unregisterSphere(&coll);

    scene.getAudio().playSFX("PotBreak"_u64, coll.center, {.volume = 0.9f, .variation = 32});

    int coinAmount = 10 + (rand() % 15);
    for(int i=0; i<coinAmount; ++i) {
      scene.requestSpawnActor("Coin"_u32, coll.center - T3DVec3{0,0.2f,0}, 1);
    }

    auto pt = coll.center * COLL_WORLD_SCALE;
    uint32_t seed = PhysicalAddr(this);
    for(int i=0; i<10; ++i) {
      scene.getPTSwirl().add(pt + Math::randDir3D()*15.0f, seed+i, 1.0f);
    }
  }
}

void Actor::Vase::update(float deltaTime)
{
  if(!checkCulling(120.0f))return;
  drawMask = DRAW_MASK_3D;

  if(timer > 0.0f)
  {
    timer -= deltaTime;
    if(timer < 0.0f) {
      requestDelete();
      return;
    }

    // update each bone with a random rotation and offset further away
    for(int i=0; i<skeleton.skeletonRef->boneCount; ++i) {
      auto &bone = skeleton.bones[i];

      randomRot[i] += Math::randDir3D() * 0.1f;
      t3d_vec3_norm(&randomRot[i]);

      t3d_quat_rotate_euler(&bone.rotation, randomRot[i].v, 0.25f);
      bone.position += (bone.position+randomRot[i]) * deltaTime * timer * 1.9f;
      bone.position.y *= 0.92f;

      if(timer < 1.2f) {
        bone.scale *= 0.97f;
      }
      bone.hasChanged = true;
    }
    t3d_skeleton_update(&skeleton);
  } else {
    coll.center.y = startY;
    t3d_mat4fp_from_srt_euler(matFP,
      T3DVec3{{0.2f, 0.2f, 0.2f}},
      T3DVec3{{0, 0, 0}},
      coll.center * COLL_WORLD_SCALE
    );

    auto bottomPos = coll.center;
    bottomPos.y -= coll.radius - 0.1f;
    Shadows::addShadow(bottomPos * COLL_WORLD_SCALE, {0,1,0}, 2.2f, 1.0f);
  }
}

void Actor::Vase::draw3D(float deltaTime) {
  if(timer >= 0.0f) {
    t3d_model_draw_material(objBroken->material, &scene.t3dState);
    t3d_matrix_set(matFP, true);
    t3d_segment_set(1, skeleton.boneMatricesFP);
    rspq_block_run(objBroken->userBlock);
  } else {
    t3d_model_draw_material(obj->material, &scene.t3dState);
    t3d_matrix_set(matFP, true);
    rspq_block_run(obj->userBlock);
  }
}
