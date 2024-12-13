/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include "boss.h"
#include "../../utils/mesh.h"
#include "../scene.h"
#include "../shadows.h"
#include "../../render/colorHelper.h"
#include "../../debug/debugDraw.h"

namespace {
  constexpr float HURT_TIMEOUT = 0.75f;
  constexpr float MOVE_SPEED = 3.6f;

  T3DModel *model;
  float rotTimer = 0.0f;
  T3DObject *objBall;
  T3DObject *objSpikes;
  T3DObject *objEyes;

  int targetedPlayer = 0;

  uint32_t hurtIndex = 0;
  float hurtIndexTimer = 0;
  float partTimer[5] = {0.0f};
  bool isDead[5] = {false};
}

Actor::Boss::Boss(Scene &scene, const T3DVec3 &pos, uint16_t param)
  : Base(scene)
{
  drawMask = DRAW_MASK_3D | DRAW_MASK_2D;

  assert(model == nullptr); // single instance
  model = t3d_model_load(FS_BASE_PATH "obj/boss.t3dm");
  objBall   = Mesh::recordObject(model, "Ball");
  objSpikes = Mesh::recordObject(model, "Spikes");
  objEyes   = Mesh::recordObject(model, "Eyes");

  float scale = 1.0f;
  int idx = 0;
  for(auto &coll : collider) {
    coll = Coll::Sphere{pos + T3DVec3{idx * 5.0f, 1.0f, 0}, 0.8f * scale};
    coll.type = idx == 0 ? Coll::CollType::BOSS_HEAD : Coll::CollType::BOSS_BODY;
    coll.mask = 1 << idx;
    coll.interactType = Coll::InteractType::TRI_MESH | Coll::InteractType::SPHERES;
    coll.callback = [this, idx](Coll::Sphere &sphere) {onCollision(sphere, idx);};
    scene.getCollScene().registerSphere(&coll);
    scale *= 0.9f;
    partTimer[idx] = 1.0f;
    isDead[idx] = false;
    ++idx;
  }
}

Actor::Boss::~Boss() {
  scene.requestGameEnd();

  t3d_model_free(model);
  model = nullptr;

  for(auto &coll : collider) {
    scene.getCollScene().unregisterSphere(&coll);
  }
}

void Actor::Boss::onCollision(Coll::Sphere &sphere, int index) {
  if(hurtTimer > 0.0f)return;
  if(collider[index].type != Coll::CollType::BOSS_HEAD)return;

  auto &cl = collider[index];

  if(sphere.type == Coll::CollType::SWORD) {
    if(hurt(4)) {
      auto midPoint = (cl.center + sphere.center) * 0.5f;
      scene.requestSpawnActor("Part"_u32, midPoint, 0);

      scene.getAudio().playSFX("BoxHit"_u64, cl.center, {.volume = 0.8f, .variation = 64});
      scene.getAudio().playSFX("SwordHit"_u64, cl.center, {.volume = 0.9f, .variation = 64});

      int coinCount = 2 + (rand() % 2);
      for(int i=0; i<coinCount; ++i) {
        scene.requestSpawnActor("Coin"_u32, cl.center, 1);
      }
    }

    if(index == 0)return;
    auto diff = cl.center - sphere.center;
    float indexScale = 2.0f + index * 0.5f;
    cl.velocity = diff * indexScale;
  }
}

bool Actor::Boss::hurt(int damage) {
 if(hurtTimer > 0.0f)return false;

  hurtTimer = HURT_TIMEOUT;
  health.reduce(damage);
  return true;
}

InputState Actor::Boss::updateAI(float deltaTime)
{
  time += deltaTime;
  if(time > randTimeEnd) {
    time = 0.0f;
    randTimeEnd = (float)(rand() % 5) + 2.5f;
    targetedPlayer = -1;
  }

  InputState res{};

  T3DVec3 dirRand{};
  T3DVec3 dirPlayer{};
  const auto &bossPos = getColl(0).center;

  // get the closest alive player position
  const T3DVec3 *closestPos;
  if(targetedPlayer >= 0) {
    closestPos = &scene.getPlayer(targetedPlayer).getPos();
  } else {
    closestPos = &scene.getPlayer(0).getPos();
    float minDist = 999999.0f;
    for(int i=0; i<4; ++i) {
      const auto &player = scene.getPlayer(i);
      const T3DVec3 &pos = player.getPos();
      float dist2 = t3d_vec3_distance2(pos, bossPos);
      if (dist2 < minDist) {
        minDist = dist2;
        closestPos = &pos;
        targetedPlayer = i;
      }
    }
  }

  // get the direction to the closest player
  //Debug::drawLine(bossPos*16.0f, *closestPos*16.0f, {0xFF, 0, 0, 0xFF});

  dirPlayer = *closestPos - bossPos;
  dirPlayer.v[1] = 0.0f;
  t3d_vec3_norm(dirPlayer);
  dirPlayer *= 0.5f + (fm_sinf(time * 1.5f) + 1.0f) * 0.25f;

  // get random direction to mix into the optimal direction
  dirRand.v[0] = fm_sinf(time * 2.0f);
  dirRand.v[2] = fm_cosf(time * 2.5f);
  dirRand *= 0.75f;

  float lerp = isHurt() ? 0.75f : 0.25f;
  t3d_vec3_lerp(res.move, dirPlayer, dirRand, lerp);
  return res;
}

void Actor::Boss::bossDie() {
  for(auto &coll : collider) {
    scene.getCollScene().unregisterSphere(&coll);
  }
}

void Actor::Boss::update(float deltaTime) {
  health.update(deltaTime);
  if(health.value <= 0) {
    if(dieTimer == 0.0) {
      bossDie();
    }
    dieTimer += deltaTime;

    for(int p=0; p<5; ++p) {
      if(dieTimer > (float)p && !isDead[p]) {
        if(p == 0)scene.getAudio().stopBGM();

        uint16_t seed = PhysicalAddr(this);
        isDead[p] = true;
        auto pt = collider[p].center * COLL_WORLD_SCALE;
        for(int i=0; i<20; ++i) {
          scene.getPTSwirl().add(pt + Math::randDir3D()*16.0f, seed+i, {0x90, 0x90, 0x90}, 1.0f);
        }
        for(int i=0; i<15; ++i) {
          scene.requestSpawnActor("Coin"_u32, collider[p].center, 1);
        }

        scene.getAudio().playSFX("BoxHit"_u64, {.volume = 0.8f});
        scene.getAudio().playSFX("PotBreak"_u64, {.volume = 0.8f});
      }
    }

    if(dieTimer > 14.0f) {
      requestDelete();
    }
    return;
  }

  InputState input = updateAI(deltaTime);
  auto &collScene = scene.getCollScene();

  float scale = 0.15f;
  rotTimer += deltaTime;
  hurtTimer = fmaxf(0.0f, hurtTimer - deltaTime);

  sprintTimer = fmaxf(0.0f, sprintTimer - deltaTime);
  if(sprintTimer == 0.0f && input.jump) {
    sprintTimer = 2.0f;
  }

  auto oldVel = collider[0].velocity;
  auto addVel = input.move * MOVE_SPEED;
  t3d_vec3_lerp(collider[0].velocity, oldVel, addVel, 0.05f);

  isMoving = t3d_vec3_len2(input.move) > 0.1f;
  if(isMoving) {
    faceDir = atan2f(-input.move.v[0], input.move.v[2]);
  }

  for(int p=1; p<5; ++p)
  {
    collider[p].velocity.v[0] *= 0.9f;
    collider[p].velocity.v[2] *= 0.9f;

    auto &pos = collider[p].center;
    auto &posLast = collider[p-1].center;

    float partDist = (collider[p].radius + collider[p-1].radius) * 1.1f;

    float dist = t3d_vec3_len2(pos - posLast);
    if(dist > partDist) {
      T3DVec3 dir = pos - posLast;
      t3d_vec3_norm(dir);
      pos = posLast + dir * partDist;
    }
  }

  hurtIndexTimer += deltaTime;
  if(hurtIndexTimer > 4.0f) {
    hurtIndexTimer = 0.0f;
    hurtIndex = (hurtIndex + 1) % 5;
  }

  uint32_t p=0;
  for(auto &coll : collider)
  {
    if(p == hurtIndex) {
      partTimer[p] = fmaxf(0.0f, partTimer[p] - deltaTime);
    } else {
      partTimer[p] = fminf(1.0f, partTimer[p] + deltaTime);
    }

    coll.type = p == hurtIndex ? Coll::CollType::BOSS_HEAD : Coll::CollType::BOSS_BODY;
    coll.velocity.v[1] -= 0.5f;
    auto posWorld = coll.center * 16.0f;

    if(p == 0) {
      t3d_viewport_calc_viewspace_pos(NULL, &pos2D, &posWorld);
    }

    float rotOffset = p * 0.25f;
    t3d_mat4fp_from_srt_euler((T3DMat4FP*)UncachedAddr(&matFP[p]),
      T3DVec3{{scale, scale, scale}},
      p == 0
        ? T3DVec3 {{0, faceDir, 0}}
        : T3DVec3{{rotOffset + rotTimer*0.2f, rotTimer, rotOffset}},
      posWorld
    );

    if(partTimer[p] > 0.2f) {
      float localScale = (0.5f + partTimer[p]*0.5f) * scale;
      t3d_mat4fp_from_srt_euler((T3DMat4FP*)UncachedAddr(&matFPSpikes[p]),
        T3DVec3{{localScale, localScale, localScale}},
        p == 0
          ? T3DVec3 {{0, faceDir, 0}}
          : T3DVec3{{rotOffset + rotTimer*0.2f, rotTimer, rotOffset}},
        posWorld
      );
      data_cache_hit_writeback(&matFPSpikes[p], sizeof(T3DMat4FP));
    }

    scale *= 0.9f;
    data_cache_hit_writeback(&matFP[p], sizeof(T3DMat4FP));

    auto rayRes = collScene.raycastFloor(coll.center);
    Shadows::addShadow(rayRes.hitPos * COLL_WORLD_SCALE + T3DVec3{0, 0.1f, 0}, {0,1,0}, 30.0f * scale, 1.0f);
    ++p;
  }
}

void Actor::Boss::draw3D(float deltaTime) {
   if(hurtTimer > 0.0f) {
    auto col = ColorHelper::primHurtEffect(hurtTimer, 1.0f / HURT_TIMEOUT);
    rdpq_set_prim_color(col);
  } else {
    rdpq_set_prim_color({0xFF, 0xFF, 0xFF, 0xFF});
  }

  // @TODO: add light handler in scene
  t3d_light_set_ambient({0xAA, 0x7A, 0x7A, 0xFF});

  t3d_model_draw_material(objBall->material, &scene.t3dState);
  for(uint32_t i=0; i<matFP.size(); ++i) {
    if(isDead[i])continue;

    t3d_light_set_ambient({0xAA, 0x7A, 0x7A, (uint8_t)(0xFF * partTimer[i])});

    if(partTimer[i] > 0.3f) {
      t3d_matrix_set(&matFPSpikes[i], true);
      rspq_block_run(objSpikes->userBlock);
    }

    t3d_matrix_set(&matFP[i], true);
    rspq_block_run(objBall->userBlock);
  }

  t3d_light_set_ambient({0xAA, 0x7A, 0x7A, 0xFF});

  if(!isDead[0]) {
    t3d_matrix_set(&matFP[0], true);
    t3d_model_draw_material(objEyes->material, &scene.t3dState);
    rspq_block_run(objEyes->userBlock);
  }

  rdpq_set_prim_color({0xFF, 0xFF, 0xFF, 0xFF});
  t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);
}

void Actor::Boss::draw2D(float deltaTime) {
  //rdpq_mode_push();
  //rdpq_set_mode_fill({});
  rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
  health.draw(pos2D, 40.0f);
  //rdpq_mode_pop();
}
