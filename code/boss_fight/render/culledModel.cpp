/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "../main.h"
#include "culledModel.h"
#include <array>

namespace {
  constexpr float MAP_SCALE = 0.25f;

  constexpr uint32_t LAYER_COUNT = 6;
  constexpr uint32_t LAYER_OBJ_COUNT = 16;

  float scrollOffset = 0.0f;

  std::array<std::array<T3DObject*, LAYER_OBJ_COUNT>, LAYER_COUNT> layerObj;
  std::array<uint8_t, LAYER_COUNT> layerCount;

  std::array<rspq_block_t*, 4> currBlock{};
  uint32_t currBlockIdx = 0;
  uint64_t lastHash = 0;
  bool needsUpdate = false;
  bool needsLiveMaterial = false;
}

CulledModel::CulledModel(const char *modelPath)
{
  layerObj.fill({});
  layerCount.fill(0);
  currBlock.fill(nullptr);
  currBlockIdx = 0;
  needsUpdate = false;

  model = t3d_model_load(modelPath);
  auto it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
  while(t3d_model_iter_next(&it)) {
    auto mat = it.object->material;
    // nothing should use fog, and i may have it enabled to preview some t3d specific effects in fast64
    mat->fogMode = T3D_FOG_MODE_DISABLED;
    // depth is not supported in materials yet, so set it manually based on names
    // since the camera angle is fixed, we can have a fixed draw order and avoid depth reads
    mat->otherModeMask |= SOM_Z_COMPARE | SOM_Z_WRITE;
    mat->otherModeValue |= SOM_Z_WRITE;

    bool noDepth = it.object->name[0] == '#';
    if(noDepth) {
      mat->otherModeValue &= ~SOM_Z_COMPARE;
    } else {
      mat->otherModeValue |= SOM_Z_COMPARE;
    }

    #if RSPQ_PROFILE
      // t3d disables some code in profiling, uvgen just happens to break geometry so disable it here
      mat->vertexFxFunc = T3D_VERTEX_FX_NONE;
    #endif

    // each object with depth disabled needs to be sorted, the prio. is stored in the name
    // as a one-digit number after the '#' char
    uint8_t layerId = LAYER_COUNT - 1;
    if(noDepth) {
      layerId = 0;
      char layerChar = it.object->name[1];
      if(layerChar >= '0' && layerChar <= '9') {
        layerId = it.object->name[1] - '0';
      }
    }
    it.object->_padding[0] = layerId; // @TODO: add generic user-defined IDs to t3d struct?
  }

  mapMatFP = (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_from_srt_euler(mapMatFP,
    T3DVec3{{MAP_SCALE, MAP_SCALE, MAP_SCALE}},
    T3DVec3{{0, 0, 0}},
    T3DVec3{{0, 0, 0}}
  );
}

CulledModel::~CulledModel() {
  t3d_model_free(model);
  free_uncached(mapMatFP);

  rspq_wait();
  for(auto &block : currBlock) {
    if(block)rspq_block_free(block);
  }
}

void CulledModel::update(const T3DVec3 &camPos) {
  auto bvh = t3d_model_bvh_get(model);
  auto vp = t3d_viewport_get();
  auto frustomLocal = vp->viewFrustum;
  t3d_frustum_scale(&frustomLocal, MAP_SCALE);
  t3d_model_bvh_query_frustum(bvh, &frustomLocal);

  // put all visible objects into layers, and set counters...

  uint64_t newHash = 0;
  layerCount.fill(0);
  auto it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
  while(t3d_model_iter_next(&it)) {
    if(it.object->isVisible) {
      uint8_t layerId = it.object->_padding[0];
      layerObj[layerId][layerCount[layerId]++] = it.object;
      it.object->isVisible = false;

      if(it.object->material->name[0] == '#') {
        needsLiveMaterial = true;
      }

      newHash ^= ((uint64_t)(void*)(it.object)) | ((uint64_t)it.object->triCount << 32);
      newHash = std::rotl(newHash, 10);
    }
  }

  needsUpdate = (newHash != lastHash) || needsLiveMaterial;
  lastHash = newHash;

  scrollOffset = camPos.x*2;
  scrollOffset = fm_fmodf(scrollOffset, 128.0f);
}

uint32_t CulledModel::draw(T3DModelState &t3dState) {
  uint32_t triCount = 0;

  auto ticks = get_ticks();

  if(needsUpdate)
  {
    if(!needsLiveMaterial) {
      currBlockIdx = (currBlockIdx + 1) & 3;
      if(currBlock[currBlockIdx])rspq_block_free(currBlock[currBlockIdx]);
      rspq_block_begin();
    }

    t3d_matrix_set(mapMatFP, true);

    // ...then draw in one go in order
    //debugf("==== Draw Layer:\n");
    for(uint32_t i = 0; i < LAYER_COUNT; ++i) {
      //debugf("  - Layer %ld\n", i);
      for(uint32_t j = 0; j < layerCount[i]; ++j) {
        auto obj = layerObj[i][j];
        //debugf("    - Obj %s\n", obj->name);
        if(obj->material->name[0] == '#') {
          obj->material->textureB.s.low = scrollOffset;
          obj->material->textureB.t.low = scrollOffset;
        }
        t3d_model_draw_material(obj->material, &t3dState);
        t3d_model_draw_object(obj, nullptr);
        triCount += obj->triCount;
      }
    }

    t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);

    if(!needsLiveMaterial) {
      currBlock[currBlockIdx] = rspq_block_end();
    }
  }

  if(!needsLiveMaterial) {
    rspq_block_run(currBlock[currBlockIdx]);
  }

  needsUpdate = false;
  needsLiveMaterial = false;
  ticks = get_ticks() - ticks;
  //debugf(" - Draw time: %lld\n", TICKS_TO_US(ticks));
  return triCount;
}
