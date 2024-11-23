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

bool script_update(struct script_state *state, float delta_time) {
  while (delta_time > EPS) {
    if (state->action->type == ACTION_END) {
      return true;
    }
    else if (state->action->type == ACTION_WARP_TO) {
      state->character->pos = state->action->v;
    }
    else if (state->action->type == ACTION_WAIT) {
      if (state->time + delta_time >= state->action->f) {
        delta_time -= state->action->f - state->time;
      } else {
        state->time += delta_time;
        break;
      }
    }
    else if (state->action->type == ACTION_SET_VISIBILITY) {
      state->character->visible = state->action->b;
    }
    else if (state->action->type == ACTION_START_ANIM) {
      struct character *c = state->character;
      c->current_anim = state->action->i;
      t3d_anim_attach(&c->s.anims[state->action->i], &c->s.skeleton);
    }
    else if (state->action->type == ACTION_WALK_TO
        || state->action->type == ACTION_CLIMB_TO) {
      struct character *c = state->character;
      if (!state->time) {
        size_t anim = state->action->type == ACTION_WALK_TO? WALK : CLIMB;
        c->current_anim = anim;
        t3d_anim_attach(&c->s.anims[anim], &c->s.skeleton);
        float dz = state->action->v.v[2] - c->pos.v[2];
        float dx = state->action->v.v[0] - c->pos.v[0];
        c->rotation = -fm_atan2f(dx, dz);
      }

      T3DVec3 diff;
      t3d_vec3_diff(&diff,
          &(T3DVec3) {{state->action->v.v[0], 0.f, state->action->v.v[2]}},
          &(T3DVec3) {{c->pos.v[0], 0.f, c->pos.v[2]}});

      float time_to_end = state->action->type == ACTION_WALK_TO?
        t3d_vec3_len(&diff) / WALK_SPEED : CLIMB_TIME - state->time;

      if (time_to_end - delta_time < EPS) {
        delta_time -= time_to_end;
        c->pos.v[0] = state->action->v.v[0];
        c->pos.v[2] = state->action->v.v[2];
      }
      else {
        float ratio = delta_time / time_to_end;
        c->pos.v[0] += ratio * diff.v[0];
        c->pos.v[2] += ratio * diff.v[2];
        state->time += delta_time;
        break;
      }
    }
    else if (state->action->type == ACTION_ROTATE_TO) {
      struct character *c = state->character;
      float target = state->action->f;
      float speed = state->action->f2;

      if (speed > 0 && target < c->rotation) {
        target += T3D_PI*2.f;
      }
      else if (speed < 0 && target > c->rotation) {
        target -= T3D_PI*2.f;
      }

      float time_to_end = (target - c->rotation) / speed;
      if (time_to_end - delta_time < EPS) {
        delta_time -= time_to_end;
        // Original, unchanged target
        c->rotation = state->action->f;
      }
      else {
        c->rotation += speed * delta_time;
        break;
      }
    }

    state->action++;
    state->time = 0.f;
  }

  return false;
}
