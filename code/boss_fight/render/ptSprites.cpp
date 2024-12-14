/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "ptSprites.h"
#include "../main.h"

namespace
{
  constexpr float SCALE_Y = 4.0f;
  constexpr float OFFSET_Y = 15.0f;
  constexpr T3DVec3 MAT_SCALE{1.0f, 1.0f / SCALE_Y, 1.0f};
}

PTSprites::PTSprites(const char* spritePath, bool isRotating)
{
  sprite = sprite_load(spritePath);
  for(auto &s : systems) {
    s.count = 0;
    s.pos = {-999,0,0}; // forces matrix creation for 0,0,0
  }

  rspq_block_begin();
  {
    rdpq_mode_begin();
      if(isRotating) {
        rdpq_mode_filter(FILTER_BILINEAR);
        rdpq_mode_alphacompare(64);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_zbuf(true, false);
      } else {
        rdpq_mode_filter(FILTER_POINT);
        rdpq_mode_alphacompare(10);
      }
      rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,0,TEX0,0), (TEX0,0,ENV,0)));
    rdpq_mode_end();

    int shift = -2;
    if(sprite->height == 16)shift = -1;
    rdpq_texparms_t p = {
      .s = {.scale_log = shift, .repeats = REPEAT_INFINITE, .mirror = isRotating},
      .t = {.scale_log = shift, .repeats = REPEAT_INFINITE, .mirror = isRotating}
    };
    rdpq_sprite_upload(TILE0, sprite, &p);

    tpx_state_set_scale(1.0f, 1.0f);
  }
  setupDPL = rspq_block_end();
  mirrorPt = 32;
  if(!isRotating) {
    mirrorPt = 0;
    //mirrorPt = sprite->width / sprite->height * 4;
  }
}

PTSprites::~PTSprites() {
  rspq_block_free(setupDPL);
  sprite_free(sprite);
}

PTSystem *PTSprites::getBySection(float sectionX) {
  // find existing that is not full...
  for(auto &s : systems) {
    if(s.pos.x == sectionX && !s.isFull())return &s;
  }
  // ...or allocate new one + matrix creation
  for(auto &s : systems) {
    if(s.count == 0) {
      s.pos = {sectionX,OFFSET_Y,0};
      t3d_mat4fp_from_srt_euler(s.mat, MAT_SCALE, {0,0,0}, s.pos);
      return &s;
    }
  }
  return nullptr;
}

void PTSprites::add(const T3DVec3 &pos, uint32_t seed, color_t col, float scale)
{
  float sectionX = fm_floorf((pos.x + 127.0f) / 256.0f);
  sectionX *= 256.0f;

  PTSystem *sys = getBySection(sectionX);
  if(!sys) {
    debugf("No space for coins! %.2f\n", sectionX);
    return;
  }

  float posY = (pos.y - OFFSET_Y) * SCALE_Y;
  if(posY < -127.0f || posY > 127.0f) {
    return;
  }

  seed = (seed * 23) >> 3;
  uint32_t offset = (seed * 23) % 7;

  auto p = tpx_buffer_get_pos(sys->particles, sys->count);
  p[0] = (int8_t)(pos.x - sectionX);
  p[1] = (int8_t)(posY);
  p[2] = (int8_t)pos.z;

  *tpx_buffer_get_size(sys->particles, sys->count) = (int8_t)(scale * 120.0f);

  auto c = tpx_buffer_get_rgba(sys->particles, sys->count);
  c[0] = col.r - (seed & 0b11111);
  c[1] = col.g - (seed & 0b11111);
  c[2] = col.b;
  c[3] = offset * 32;

  ++sys->count;
}

void PTSprites::draw(float deltaTime) {
  animTimer += deltaTime * 15.0f;
  int16_t uvOffset = (int16_t)(animTimer);
  if(uvOffset >= 8)animTimer -= 8.0f;

  rspq_block_run(setupDPL);
  tpx_state_set_tex_params(uvOffset * (1024/sprite->height), mirrorPt);

  for(auto &system : systems) {

    if(system.count % 2 != 0) {
      *tpx_buffer_get_size(system.particles, system.count) = 0;
      ++system.count;
    }
    system.drawTextured();
  }
}

void PTSprites::clear() {
  for(auto &system : systems) {
    system.count = 0;
  }
}

void PTSprites::simulateDust(float deltaTime)
{
  simTimer += deltaTime;
  bool isStep = simTimer > 0.75f;
  if(isStep)simTimer = 0;

  for(auto &system : systems)
  {
    for(uint32_t i=0; i<system.count; ++i) {
      auto pos = tpx_buffer_get_pos(system.particles, i);
      int8_t *size = tpx_buffer_get_size(system.particles, i);
      color_t *color = (color_t *)tpx_buffer_get_rgba(system.particles, i);

      if(isStep) {
        pos[1] += 1;
      }
      size[0] -= 1;
      if(color->r > 1) {
        color->r -= 1;
        color->g -= 1;
        color->b -= 1;
      }

      if(pos[1] > 126 || size[0] <= 0) {
        system.removeParticle(i--);
      }
    }
  }
}