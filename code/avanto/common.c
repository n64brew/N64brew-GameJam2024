#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#include "common.h"

extern T3DViewport viewport;

static int next_sfx_channel = FIRST_SFX_CHANNEL;

float get_ground_height(float z, struct ground *ground) {
  float height = 0;
  for (size_t i = 0; i < ground->num_changes; i++) {
    if (ground->changes[i].start_z > z) {
      break;
    }
    height = ground->changes[i].height;
  }
  return height;
}

int get_next_sfx_channel() {
  int r = next_sfx_channel++;
  if (next_sfx_channel >= FIRST_SFX_CHANNEL + NUM_SFX_CHANNELS) {
    next_sfx_channel = FIRST_SFX_CHANNEL;
  }
  return r;
}

void skeleton_init(struct skeleton *s,
    const T3DModel *model,
    size_t num_anims) {
  s->skeleton = t3d_skeleton_create(model);
  s->num_anims = num_anims;
  s->anims = malloc(sizeof(T3DAnim) * num_anims);
}

void skeleton_free(struct skeleton *s) {
  for (size_t i = 0; i < s->num_anims; i++) {
    t3d_anim_destroy(&s->anims[i]);
  }
  free(s->anims);
  t3d_skeleton_destroy(&s->skeleton);
}

void entity_init(struct entity *e,
    const T3DModel *model,
    const T3DVec3 *scale,
    const T3DVec3 *rotation,
    const T3DVec3 *pos,
    T3DSkeleton *skeleton,
    T3DModelDrawConf draw_conf) {

  e->model = model;
  e->transform = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_from_srt_euler(e->transform, scale->v, rotation->v, pos->v);
  e->skeleton = skeleton;

  rspq_block_begin();
  t3d_matrix_push(e->transform);

  if (e->skeleton) {
    draw_conf.matrices = skeleton->bufferCount == 1? skeleton->boneMatricesFP
      : (const T3DMat4FP*) t3d_segment_placeholder(T3D_SEGMENT_SKELETON);
  }
  t3d_model_draw_custom(e->model, draw_conf);

  t3d_matrix_pop(1);
  e->display_block = rspq_block_end();
}

void entity_free(struct entity *e) {
  free_uncached(e->transform);
  rspq_block_free(e->display_block);
}

