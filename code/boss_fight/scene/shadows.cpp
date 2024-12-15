/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "shadows.h"

namespace Shadows
{
  constexpr int MAX_SHADOWS = 16 * 4;
  constexpr float SCALE_FACTOR = 4.0f;
  constexpr uint32_t MAX_VERTICES = 68; // t3d has 70, but we need it divisible by 4

  uint32_t shadowCount = 0;
  uint32_t vertOffset = 0;

  T3DMat4FP *matFP{};
  T3DVertPacked *vertices{};
  sprite_t *shadowTex{};
  rspq_block_t *setupDPL{};
}

void Shadows::init() {
  shadowCount = 0;
  vertOffset = 0;
  matFP = static_cast<T3DMat4FP*>(malloc_uncached(sizeof(T3DMat4FP)));
  vertices = static_cast<T3DVertPacked*>(malloc_uncached(
    sizeof(T3DVertPacked) * MAX_SHADOWS / 2 * 4
  ));

  // prefill static data (UVs)
  constexpr int16_t UV_END = 32*32;
  for(int i = 0; i < MAX_SHADOWS; i++) {
    auto *uvA = t3d_vertbuffer_get_uv(vertices, i * 4);
    auto *uvB = t3d_vertbuffer_get_uv(vertices, i * 4 + 1);
    auto *uvC = t3d_vertbuffer_get_uv(vertices, i * 4 + 2);
    auto *uvD = t3d_vertbuffer_get_uv(vertices, i * 4 + 3);
    uvA[0] = 0;      uvA[1] = 0;
    uvB[0] = UV_END; uvB[1] = 0;
    uvC[0] = UV_END; uvC[1] = UV_END;
    uvD[0] = 0;      uvD[1] = UV_END;

    *t3d_vertbuffer_get_color(vertices, i*4+0) = 0xFFFFFFFF;
    *t3d_vertbuffer_get_color(vertices, i*4+1) = 0xFFFFFFFF;
    *t3d_vertbuffer_get_color(vertices, i*4+2) = 0xFFFFFFFF;
    *t3d_vertbuffer_get_color(vertices, i*4+3) = 0xFFFFFFFF;
  }

  t3d_mat4fp_from_srt_euler(matFP,
    {1.0f/SCALE_FACTOR, 1.0f/SCALE_FACTOR, 1.0f/SCALE_FACTOR},
    {0,0,0},
    {0,0.1f,0}
  );

  shadowTex = sprite_load(FS_BASE_PATH "shadow.i8.sprite");

  rspq_block_begin();
    rdpq_mode_begin();
      rdpq_mode_zbuf(true, false);
      rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,PRIM), (TEX0,0,PRIM,0)));
      rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
      rdpq_mode_alphacompare(40);
      rdpq_mode_fog(0);
    rdpq_mode_end();

    rdpq_set_prim_color({0, 0, 0, 0x70});
    rdpq_sync_tile();
    rdpq_sync_load();
    rdpq_sprite_upload(TILE0, shadowTex, nullptr);

    t3d_matrix_set(matFP, true);
    t3d_fog_set_enabled(false);
    t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_DEPTH | T3D_FLAG_TEXTURED | T3D_FLAG_CULL_BACK));
    t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);
  setupDPL = rspq_block_end();
}

void Shadows::destroy() {
  free_uncached(vertices);
  free_uncached(matFP);
  sprite_free(shadowTex);
  rspq_block_free(setupDPL);
}

void Shadows::addShadow(const T3DVec3 &pos, const T3DVec3 &normal, float size, float strength) {
  size *= 20;

  if (shadowCount < MAX_SHADOWS) {
    auto posA = t3d_vertbuffer_get_pos(vertices, vertOffset+0);
    auto posB = t3d_vertbuffer_get_pos(vertices, vertOffset+1);
    auto posC = t3d_vertbuffer_get_pos(vertices, vertOffset+2);
    auto posD = t3d_vertbuffer_get_pos(vertices, vertOffset+3);

    // flat floors are very likely, so we can optimize this case
    if(normal.y > 0.95f) {
      T3DVec3 posScaled{
        pos.x * SCALE_FACTOR,
        (pos.y + normal.y) * SCALE_FACTOR,
        pos.z * SCALE_FACTOR
      };

      posA[0] = posScaled.x - size;
      posA[1] = posScaled.y;
      posA[2] = posScaled.z - size;

      posB[0] = posScaled.x + size;
      posB[1] = posScaled.y;
      posB[2] = posScaled.z - size;

      posC[0] = posScaled.x + size;
      posC[1] = posScaled.y;
      posC[2] = posScaled.z + size;

      posD[0] = posScaled.x - size;
      posD[1] = posScaled.y;
      posD[2] = posScaled.z + size;
    } else {
      // ...otherwise we need to calculate the vectors to make a shadow plane
      auto posScaled = (pos + normal) * SCALE_FACTOR;
      T3DVec3 right, up;
      t3d_vec3_cross(right, normal, {0,1,0});
      t3d_vec3_cross(up, right, normal);
      t3d_vec3_norm(&right);
      t3d_vec3_norm(&up);

      auto vecA = (right - up) * size;
      auto vecB = (right + up) * size;

      posA[0] = posScaled.x - vecA.x;
      posA[1] = posScaled.y - vecA.y;
      posA[2] = posScaled.z - vecA.z;

      posB[0] = posScaled.x + vecA.x;
      posB[1] = posScaled.y + vecA.y;
      posB[2] = posScaled.z + vecA.z;

      posC[0] = posScaled.x + vecB.x;
      posC[1] = posScaled.y + vecB.y;
      posC[2] = posScaled.z + vecB.z;

      posD[0] = posScaled.x - vecB.x;
      posD[1] = posScaled.y - vecB.y;
      posD[2] = posScaled.z - vecB.z;
    }

    ++shadowCount;
    vertOffset += 4;
  }
}

void Shadows::draw() {
  rspq_block_run(setupDPL);
  uint32_t count = shadowCount * 4;
  auto vert = vertices;

  while(count)
  {
    uint32_t localCount = count;
    if(localCount > MAX_VERTICES)localCount = MAX_VERTICES;

    t3d_vert_load(vert, 0, localCount);
    for (uint32_t i = 0; i < localCount; i+=4) {
      t3d_tri_draw(i+0, i+2, i+1);
      t3d_tri_draw(i+0, i+3, i+2);
    }
    t3d_tri_sync();

    vert += localCount / 2;
    count -= localCount;
  }
}

void Shadows::reset() {
  shadowCount = 0;
  vertOffset = 0;
}