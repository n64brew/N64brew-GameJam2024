/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "grass.h"
#include "../scene.h"
#include "../../debug/debugDraw.h"

namespace
{
  constexpr float GRASS_FX_TIME = 0.4f;
  sprite_t *sprite = nullptr;
  uint32_t refCount = 0;
  uint8_t grassFxTCooldown = 0;

  color_t colorMul(color_t color, uint8_t mul)
  {
    return (color_t){
      (uint8_t)(color.r * mul / 255),
      (uint8_t)(color.g * mul / 255),
      (uint8_t)(color.b * mul / 255),
      color.a
    };
  }

  int generateGrass(TPXParticle *particles, int partCount, int seed, uint16_t arg)
  {
    char path[] = FS_BASE_PATH "grass/0.rgba32.sprite\0";
    path[sizeof(path)-17] = '0' + arg;
    sprite_t *grassTex = sprite_load(path);

    color_t *heightMap = static_cast<color_t *>(sprite_get_pixels(grassTex).buffer);

    int p = 0;
    for(uint32_t z=0; z<grassTex->height; ++z)
    {
      for(uint32_t x=0; x<grassTex->width; ++x) {
        if(p >= partCount) {
          z = grassTex->height;
          break;
        }

        auto color = heightMap[z * grassTex->width + x];
        if(color.a  < 64)continue;
        color.a >>= 1;

        int8_t *ptPosA = tpx_buffer_get_pos(particles, p);
        auto *ptColorA = (color_t*)tpx_buffer_get_rgba(particles, p);
        *ptColorA = colorMul(color, 230);

        int rndA = (Math::noise2d(x+seed, z+seed) % 3) - 1;
        int rndB = (Math::noise2d(z+seed, x-seed) % 3) - 1;

        ptPosA[0] = (x - grassTex->width/2) * 3 + rndA;
        ptPosA[1] = 1;
        ptPosA[2] = (z - grassTex->height/2) * -3 + rndB;

        *tpx_buffer_get_size(particles, p) = 120;
        ++p;
      }
    }

    sprite_free(grassTex);
    return p & ~1;
  }
}

Actor::Grass::Grass(Scene &scene, const T3DVec3 &pos, uint16_t param)
  : Base(scene), ptSystem{64*10}
{
  if(refCount++ == 0) {
    sprite = sprite_load(FS_BASE_PATH "grass/blade.i8.sprite");
  }
  drawMask = 0;
  coll.center = pos;

  ptSystem.count = generateGrass(ptSystem.particles, ptSystem.countMax, rand(), param);
  //debugf("Generated %ld particles\n", ptSystem.count);
  spawnThreshold = ptSystem.count - 100;

  for(auto &fx : ptFX) {
    fx.timer = 0.0f;
    for(uint32_t i=0; i<fx.countMax; i++) {
      auto p = tpx_buffer_get_pos(fx.particles, i);
      p[0] = (rand() % 32) - 16;
      p[1] = (rand() % 32) - 16;
      p[2] = (rand() % 32) - 16;

      auto cDst = (uint32_t*)tpx_buffer_get_rgba(fx.particles, i);
      auto cSrc = (uint32_t*)tpx_buffer_get_rgba(ptSystem.particles, rand() % ptSystem.count);
      *cDst = *cSrc;

      *tpx_buffer_get_size(fx.particles, i) = 55;
    }
    fx.count = fx.countMax;
  }

  t3d_mat4fp_from_srt_euler(ptSystem.mat,
    T3DVec3{{1.0f, 2.0f, 1.0f}},
    T3DVec3{{0, 0, 0}},
    coll.center * COLL_WORLD_SCALE + T3DVec3{{0, 0.2f, 0}}
  );
}

Actor::Grass::~Grass()
{
  if(--refCount == 0) {
    sprite_free(sprite);
  }
}

void Actor::Grass::updateFX(float deltaTime) {
  for(auto &fx : ptFX) {
    if(fx.timer > 0.0f) {
      fx.timer = fmaxf(fx.timer - deltaTime, 0.0f);

      float timeRel = fx.timer / GRASS_FX_TIME;
      float scale = (1.0f - timeRel) * 1.2f;

      t3d_mat4fp_from_srt_euler(fx.mat,
          T3DVec3{{scale, scale, scale}},
          T3DVec3{{fx.timer, fx.timer*0.8f, fx.timer*0.4f}},
          fx.pos
      );
    }
  }
}

void Actor::Grass::update(float deltaTime)
{
  if(!checkCulling(coll.center.z > 0 ? 130.0f : 150.0f))return;
  drawMask = DRAW_MASK_PTX;

  bool hadCut = false;
  updateFX(deltaTime);
  for(int p=0; p<4; ++p)
  {
    const auto &player = scene.getPlayer(p);
    if(!player.isAttacking(0.2f))continue;

    auto &playerPos = player.getPos();
    if(fabsf(playerPos.y - coll.center.y) > 0.5f)continue;

    float diffX = playerPos.x - coll.center.x;
    float diffZ = playerPos.z - coll.center.z;
    float dist2XZ = diffX*diffX + diffZ*diffZ;

    if(dist2XZ < 8.2f)
    {
      // nwo erase all particles within a certain radius of the position by setting their size to 0
      int localPosX = (playerPos.x - coll.center.x) * COLL_WORLD_SCALE;
      int localPosZ = (playerPos.z - coll.center.z) * COLL_WORLD_SCALE;

      uint32_t oldCount = ptSystem.count;
      for(uint32_t i=0; i<ptSystem.count; ++i)
      {
        auto ptPos = tpx_buffer_get_pos(ptSystem.particles, i);
        int diffX = ptPos[0] - localPosX;
        int diffZ = ptPos[2] - localPosZ;
        if((diffX*diffX + diffZ*diffZ) < 56) {
          ptSystem.removeParticle(i);
        }
      }

      fxCooldown -= deltaTime;
      if(grassFxTCooldown > 0)--grassFxTCooldown;

      if(oldCount != ptSystem.count && fxCooldown <= 0.0f) {
        hadCut = true;
        if(grassFxTCooldown == 0) {
          scene.getAudio().playSFX("GrassCut"_u64, playerPos, {.volume = 0.5f, .variation = 64});
          grassFxTCooldown = 10;
        }

        for(auto &fx : ptFX) {
          if(fx.timer > 0.0f)continue;
          fx.timer = GRASS_FX_TIME;
          fx.pos = (playerPos * COLL_WORLD_SCALE) - T3DVec3{{0, 2.5f, 0}};
          fxCooldown = 0.04f;
          updateFX(deltaTime);
          break;
        }
      }
    }

    if(hadCut && (int)ptSystem.count < spawnThreshold) {
      uint32_t randCount = 2 + (rand() % 2);
      for(uint32_t i=0; i<randCount; ++i) {
        scene.requestSpawnActor("Coin"_u32, playerPos, 1);
      }
      spawnThreshold = (int32_t)ptSystem.count - 75;
    }

    if(hadCut && ptSystem.count < 5) {
      for(uint32_t i=0; i<25; ++i) {
        scene.requestSpawnActor("Coin"_u32, playerPos, 1);
      }
      requestDelete();
    }
  }
}

void Actor::Grass::drawPtx(float deltaTime) {
  rdpq_sync_pipe();
  rdpq_mode_filter(FILTER_POINT);
  rdpq_mode_alphacompare(10);
  rdpq_mode_zbuf(true, true);
  rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,0,TEX0,0), (0,0,0,TEX0)));

  rdpq_texparms_t p = {
    .s = {.scale_log = -1, .repeats = REPEAT_INFINITE, .mirror = false},
    .t = {.scale_log = -3, .repeats = REPEAT_INFINITE, .mirror = false}
  };

  rdpq_sync_tile();
  rdpq_sync_load();
  rdpq_sprite_upload(TILE0, sprite, &p);

  tpx_state_set_tex_params(0, 0);
  tpx_state_set_scale(0.4f, 0.8f);
  ptSystem.drawTextured();

  bool hadMatUpdate = false;
  for(auto &fx : ptFX) {
    if(fx.timer > 0.0f) {
      if(!hadMatUpdate) {
        rdpq_sync_pipe();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        hadMatUpdate = true;
      }
      tpx_state_set_scale(fx.timer+0.125f, fx.timer+0.125f);
      fx.drawTextured();
    }
  }
}

void Actor::Grass::drawDebug() {
  auto &playerPos = scene.getPlayer(0).getPos();

  color_t col{0xFF, 0xFF, 0xFF, 0xFF};
  if(playerPos.y - 0.5f > coll.center.y)col = {0xFF, 0, 0, 0xFF};

  float diffX = playerPos.x - coll.center.x;
  float diffZ = playerPos.z - coll.center.z;
  float dist2XZ = diffX*diffX + diffZ*diffZ;
  if(dist2XZ > 7.5f)return;

  Debug::drawLine(coll.center*COLL_WORLD_SCALE, scene.getPlayer(0).getPos() * COLL_WORLD_SCALE, col);
}
