#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/tpx.h>

#include "common.h"

extern T3DViewport viewport;
extern struct character players[];
extern rspq_block_t *empty_hud_block;
extern struct particle_source particle_sources[];
extern struct camera cam;

const char *const PLAYER_TITLES[] = {
  SW_PLAYER1_S "P1",
  SW_PLAYER2_S "P2",
  SW_PLAYER3_S "P3",
  SW_PLAYER4_S "P4",
};

static bool script_signals[SCRIPT_NUM_SIGNALS];

float get_ground_height(float z, struct ground *ground) {
  float height = 0;
  for (size_t i = 0; i < ground->num_changes; i++) {
    if (ground->changes[i].start_z > z) {
      break;
    }
    if (!ground->changes[i].ramp_to_next) {
      height = ground->changes[i].height;
    }
    else {
      float diff = ground->changes[i+1].height - ground->changes[i].height;
      float len = ground->changes[i+1].start_z - ground->changes[i].start_z;
      float prog = z - ground->changes[i].start_z;
      height = ground->changes[i].height + diff*(prog/len);
    }
  }
  return height;
}

float get_ground_angle(float z, struct ground *ground) {
  float angle = 0.f;
  for (size_t i = 0; i < ground->num_changes; i++) {
    if (ground->changes[i].start_z > z) {
      break;
    }
    if (!ground->changes[i].ramp_to_next) {
      angle = 0.f;
    }
    else {
      float h = ground->changes[i+1].height - ground->changes[i].height;
      float dz = ground->changes[i+1].start_z - ground->changes[i].start_z;
      float hip = sqrtf(h*h + dz*dz);
      angle = asinf(h*sinf(T3D_PI/2.f)/hip);
    }
  }
  return angle;
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
    T3DModelDrawConf *draw_conf) {

  e->model = model;
  e->transform = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_from_srt_euler(e->transform, scale->v, rotation->v, pos->v);
  e->skeleton = skeleton;

  rspq_block_begin();
  t3d_matrix_push(e->transform);

  T3DModelDrawConf dummy_conf;
  memset(&dummy_conf, 0, sizeof(dummy_conf));
  if (!draw_conf) {
    draw_conf = &dummy_conf;
  }
  if (e->skeleton) {
    draw_conf->matrices = skeleton->bufferCount == 1? skeleton->boneMatricesFP
      : (const T3DMat4FP*) t3d_segment_placeholder(T3D_SEGMENT_SKELETON);
  }
  else {
    draw_conf->matrices = NULL;
  }
  t3d_model_draw_custom(e->model, *draw_conf);

  t3d_matrix_pop(1);
  e->display_block = rspq_block_end();
}

void entity_free(struct entity *e) {
  free_uncached(e->transform);
  rspq_block_free(e->display_block);
}

void script_reset_signals() {
  for (size_t i = 0; i < SCRIPT_NUM_SIGNALS; i++) {
    script_signals[i] = false;
  }
}

bool script_update(struct script_state *state, float delta_time) {
  while (delta_time > EPS) {
    if (state->action->type == ACTION_END) {
      return true;
    }
    else if (state->action->type == ACTION_WARP_TO) {
      state->character->pos = state->action->pos;
    }
    else if (state->action->type == ACTION_WAIT) {
      if (state->time + delta_time >= state->action->time) {
        delta_time -= state->action->time - state->time;
      }
      else {
        state->time += delta_time;
        break;
      }
    }
    else if (state->action->type == ACTION_SET_VISIBILITY) {
      state->character->visible = state->action->visibility;
    }
    else if (state->action->type == ACTION_START_ANIM
        || state->action->type == ACTION_DO_WHOLE_ANIM) {
      struct character *c = state->character;
      T3DAnim *anim = &c->s.anims[state->action->anim];
      if (!state->time) {
        c->current_anim = state->action->anim;
        t3d_anim_attach(anim, &c->s.skeleton);
        t3d_anim_set_playing(anim, true);
      }

      if (state->action->type == ACTION_DO_WHOLE_ANIM) {
        // Anim not over
        if (state->time - anim->animRef->duration < EPS) {
          state->time += delta_time;
          break;
        }
        else {
          delta_time -= anim->animRef->duration - state->time;
        }
      }
    }
    else if (state->action->type == ACTION_WALK_TO
        || state->action->type == ACTION_CLIMB_TO) {
      struct character *c = state->character;
      if (!state->time) {
        size_t anim = state->action->type == ACTION_WALK_TO? WALK : CLIMB;
        c->current_anim = anim;
        t3d_anim_attach(&c->s.anims[anim], &c->s.skeleton);
        t3d_anim_set_playing(&c->s.anims[anim], true);
        float dz = state->action->pos.v[2] - c->pos.v[2];
        float dx = state->action->pos.v[0] - c->pos.v[0];
        c->rotation = -fm_atan2f(dx, dz);
      }

      T3DVec3 diff;
      t3d_vec3_diff(&diff,
          &(T3DVec3) {{state->action->pos.v[0], 0.f, state->action->pos.v[2]}},
          &(T3DVec3) {{c->pos.v[0], 0.f, c->pos.v[2]}});

      float time_to_end = state->action->type == ACTION_WALK_TO?
        t3d_vec3_len(&diff) / state->action->walk_speed:
        c->s.anims[CLIMB].animRef->duration - state->time;

      if (time_to_end - delta_time < EPS) {
        delta_time -= time_to_end;
        c->pos.v[0] = state->action->pos.v[0];
        c->pos.v[2] = state->action->pos.v[2];
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
      float target = state->action->rot;

      if (state->action->speed > 0 && target < c->rotation) {
        target += T3D_PI*2.f;
      }
      else if (state->action->speed < 0 && target > c->rotation) {
        target -= T3D_PI*2.f;
      }

      float time_to_end = (target - c->rotation) / state->action->speed;
      if (time_to_end - delta_time < EPS) {
        delta_time -= time_to_end;
        // Original, unchanged target
        c->rotation = state->action->rot;
      }
      else {
        c->rotation += state->action->speed * delta_time;
        break;
      }
    }
    else if (state->action->type == ACTION_PLAY_SFX) {
      wav64_play(state->action->sfx, state->action->channel);
    }
    else if (state->action->type == ACTION_START_XM64) {
      xm64player_play(state->action->xm64, state->action->first_channel);
    }
    else if (state->action->type == ACTION_MOVE_CAMERA_TO) {
      float new_time = state->time + delta_time;

      if (state->action->travel_time - new_time < EPS) {
        delta_time -= new_time - state->action->travel_time;
        cam.pos = state->action->pos;
        cam.target = state->action->target;
        t3d_viewport_look_at(&viewport,
            &cam.pos,
            &cam.target,
            &(T3DVec3) {{0, 1, 0}});
      }
      else {
        T3DVec3 end_angle;
        t3d_vec3_diff(&end_angle, &state->action->target, &state->action->pos);
        T3DVec3 cam_angle;
        t3d_vec3_diff(&cam_angle, &cam.target, &cam.pos);
        float time_left = state->action->travel_time - new_time;
        for (size_t i = 0; i < 3; i++) {
          float d = state->action->pos.v[i] - cam.pos.v[i];
          cam.pos.v[i] += d * (delta_time / time_left);

          d = end_angle.v[i] - cam_angle.v[i];
          cam_angle.v[i] += d * (delta_time / time_left);
        }

        t3d_vec3_add(&cam.target, &cam.pos, &cam_angle);
        t3d_viewport_look_at(&viewport,
            &cam.pos,
            &cam.target,
            &(T3DVec3) {{0, 1, 0}});

        state->time = new_time;
        break;
      }
    }
    else if (state->action->type == ACTION_SEND_SIGNAL) {
      script_signals[state->action->signal] = true;
    }
    else if (state->action->type == ACTION_WAIT_FOR_SIGNAL) {
      if (!script_signals[state->action->signal]) {
        break;
      }
    }
    else if (state->action->type == ACTION_ANIM_SET_PLAYING) {
      struct character *c = state->character;
      T3DAnim *anim = &c->s.anims[c->current_anim];
      t3d_anim_set_playing(anim, state->action->playing);
    }
    else if (state->action->type == ACTION_ANIM_UPDATE_TO_TS) {
      struct character *c = state->character;
      T3DAnim *anim = &c->s.anims[c->current_anim];
      t3d_anim_update(anim, state->action->time);
    }
    else if (state->action->type == ACTION_CALLBACK) {
      state->action->callback();
    }

    state->action++;
    state->time = 0.f;
  }

  return false;
}

rspq_block_t *build_empty_hud_block() {
  const color_t LINE_COLOR = RGBA32(0x00, 0x00, 0x00, 0xff);
  const color_t BAR_BG_COLOR = RGBA32(0x00, 0xc9, 0xff, 0xff);

  rspq_block_begin();
  rdpq_mode_push();
  rdpq_mode_zbuf(false, false);
  for (size_t i = 0; i < 4; i++) {
    int y = HUD_VERTICAL_BORDER;
    int x = HUD_HORIZONTAL_BORDER + i*HUD_INDIVIDUAL_H_SPACE;
    int mid_x = x + HUD_INDIVIDUAL_H_SPACE/2;

    MITIGATE_FONT_BUG;
    rdpq_text_print(NULL, FONT_NORMAL, mid_x-4, y, PLAYER_TITLES[i]);

    x += HUD_BAR_X_OFFSET;
    y += HUD_BAR_Y_OFFSET;
    int w = HUD_INDIVIDUAL_H_SPACE - HUD_BAR_X_OFFSET*2;
    int h = HUD_BAR_HEIGHT;
    rdpq_set_mode_fill(LINE_COLOR);
    rdpq_fill_rectangle(x, y, x+w, y+1);
    rdpq_fill_rectangle(x, y, x+1, y+h);
    rdpq_fill_rectangle(x, y+h-1, x+w, y+h);
    rdpq_fill_rectangle(x+w-1, y, x+w, y+h);
    rdpq_set_mode_fill(BAR_BG_COLOR);
    rdpq_fill_rectangle(x+1, y+1, x+w-1, y+h-1);
  }
  rdpq_mode_pop();

  return rspq_block_end();
}

void draw_hud() {
  const color_t BAR_COLOR = RGBA32(0xff, 0x45, 0x00, 0xff);

  rspq_block_run(empty_hud_block);

  rdpq_mode_push();
  rdpq_mode_zbuf(false, false);
  for (size_t i = 0; i < 4; i++) {
    int y = HUD_VERTICAL_BORDER;
    int x = HUD_HORIZONTAL_BORDER + i*HUD_INDIVIDUAL_H_SPACE;
    int mid_x = x + HUD_INDIVIDUAL_H_SPACE/2;

    x += HUD_BAR_X_OFFSET + 1;
    y += HUD_BAR_Y_OFFSET + 1;
    int max_w = HUD_INDIVIDUAL_H_SPACE - HUD_BAR_X_OFFSET*2 - 2;
    int h = HUD_BAR_HEIGHT - 2;;
    int w = (int) roundf((float) max_w * players[i].temperature);
    if (w > max_w) {
      w = max_w;
    }
    rdpq_set_mode_fill(BAR_COLOR);
    rdpq_fill_rectangle(x, y, x+w, y+h);

    if (players[i].out) {
      MITIGATE_FONT_BUG;
      rdpq_text_print(NULL, FONT_NORMAL, mid_x-8, y+10, SW_OUT_S "OUT");
    }
  }
  rdpq_mode_pop();
}

static void particle_source_init_steam(struct particle_source *source) {
  particle_source_reset_steam(source);
  for (size_t i = 0; i < source->_num_allocated_particles/2; i++) {
      source->_particles[i].colorA[0] = 0xff;
      source->_particles[i].colorA[1] = 0xff;
      source->_particles[i].colorA[2] = 0xff;
      source->_particles[i].colorA[3] = 0x80;

      source->_particles[i].colorB[0] = 0xff;
      source->_particles[i].colorB[1] = 0xff;
      source->_particles[i].colorB[2] = 0xff;
      source->_particles[i].colorB[3] = 0x80;
  }
  source->max_particles = source->_num_allocated_particles;
}

static void particle_source_spawn_snow(struct particle_source *source,
    int8_t *pos) {
  int8_t cx = (rand() % (source->x_range*2+1)) - source->x_range;
  int8_t cz = (rand() % (source->z_range*2+1)) - source->z_range;
  int8_t cy = rand() & 0xff;

  pos[0] = cx;
  pos[1] = cy;
  pos[2] = cz;
}

static void particle_source_init_snow(struct particle_source *source) {
  source->_y_move_error = 0.f;
  for (size_t i = 0; i < source->_num_allocated_particles/2; i++) {
      source->_particles[i].colorA[0] = 0xff;
      source->_particles[i].colorA[1] = 0xff;
      source->_particles[i].colorA[2] = 0xff;
      source->_particles[i].colorA[3] = 0xff;
      source->_particles[i].sizeA = 1;
      particle_source_spawn_snow(source, source->_particles[i].posA);

      source->_particles[i].colorB[0] = 0xff;
      source->_particles[i].colorB[1] = 0xff;
      source->_particles[i].colorB[2] = 0xff;
      source->_particles[i].colorB[3] = 0xff;
      source->_particles[i].sizeB = 1;
      particle_source_spawn_snow(source, source->_particles[i].posB);
  }
}

static void particle_source_init_splash(struct particle_source *source) {
  source->_time = 0.f;

  for (size_t i = 0; i < source->_num_allocated_particles/2; i++) {
      source->_particles[i].colorA[0] = 0x0a;
      source->_particles[i].colorA[1] = 0xa3;
      source->_particles[i].colorA[2] = 0xcf;
      source->_particles[i].colorA[3] = 0xff;
      source->_particles[i].sizeA = 0;

      source->_particles[i].colorB[0] = 0x0a;
      source->_particles[i].colorB[1] = 0xa3;
      source->_particles[i].colorB[2] = 0xcf;
      source->_particles[i].colorB[3] = 0xff;
      source->_particles[i].sizeB = 0;
  }
}

void particle_source_init(struct particle_source *source,
    size_t num_particles,
    int type) {
  source->_type = type;

  source->_num_allocated_particles = num_particles & 1?
    num_particles + 1 : num_particles;
  source->_meta = NULL;
  source->_particles = malloc_uncached(
      sizeof(TPXParticle) * (source->_num_allocated_particles/2));
  source->_transform = malloc_uncached(sizeof(T3DMat4FP));

  if (type != SNOW) {
    source->_meta = malloc(
        sizeof(struct particle_meta) * source->_num_allocated_particles);
  }

  source->_type = type;
  switch (type) {
    case STEAM:
      particle_source_init_steam(source);
      break;

    case SNOW:
      particle_source_init_snow(source);
      break;

    case SPLASH:
      particle_source_init_splash(source);
      break;
  }
}

void particle_source_reset_steam(struct particle_source *source) {
  for (size_t i = 0; i < source->_num_allocated_particles/2; i++) {
    source->_particles[i].sizeA = 0;
    source->_particles[i].sizeB = 0;
  }
  source->_y_move_error = 0.f;
  source->_to_spawn = 0;
}

void particle_source_reset_splash(struct particle_source *source,
    size_t num_particles) {
  size_t in_use = 0;
  for (size_t i = 0; i < source->_num_allocated_particles/2; i++) {
    if (in_use < num_particles) {
      source->_particles[i].sizeA = source->particle_size;
      source->_particles[i].posA[0] = 0;
      source->_particles[i].posA[1] = 0;
      source->_particles[i].posA[2] = 0;
      in_use++;
    }
    else {
      source->_particles[i].sizeA = 0;
    }

    if (in_use < num_particles) {
      source->_particles[i].sizeB = source->particle_size;
      source->_particles[i].posB[0] = 0;
      source->_particles[i].posB[1] = 0;
      source->_particles[i].posB[2] = 0;
      in_use++;
    }
    else {
      source->_particles[i].sizeB = 0;
    }
  }

  struct particle_meta *m = source->_meta;
  for (size_t i = 0; i < num_particles; i++, m++) {
    m->h = rand() % (source->max_height-source->min_height)
      + source->min_height;
    m->d = rand() % (source->max_dist-source->min_dist)
      + source->min_dist;

    float angle = rand_float(0.f, T3D_PI*2.f);
    m->dir[0] = cosf(angle);
    m->dir[1] = sinf(angle);
  }

  source->_time = 0.f;
  source->paused = false;
  source->render = true;
}

void particle_source_free(struct particle_source *source) {
  source->_num_allocated_particles = 0;
  source->_type = UNDEFINED;
  if (source->_particles) {
    free_uncached(source->_particles);
    source->_particles = NULL;
  }
  if (source->_transform) {
    free_uncached(source->_transform);
    source->_transform = NULL;
  }
  if (source->_meta) {
    free(source->_meta);
    source->_meta = NULL;
  }
}

static void particle_source_spawn_steam(struct particle_source *source,
    int8_t *pos, int8_t *size, struct particle_meta *meta) {
  int8_t cx = (rand() % (source->x_range*2+1)) - source->x_range;
  int8_t cz = (rand() % (source->z_range*2+1)) - source->z_range;
  meta->cx = cx;
  meta->cz = cz;

  float v = -128.f / T3D_PI;
  *size = source->particle_size;
  pos[0] = cx+sinf(v);
  pos[1] = -128;
  pos[2] = cz+cosf(v);
}

static void particle_source_iterate_steam(struct particle_source *source,
    float delta_time) {
  source->_y_move_error += ((float) source->height / source->time_to_rise)
    * delta_time;
  int y_move = (int) source->_y_move_error;
  source->_y_move_error -= (float) y_move;

  source->_to_spawn += ((float) source->max_particles / source->time_to_rise)
    * delta_time;
  int actual_to_spawn = (int) source->_to_spawn;
  int spawned = 0;

  TPXParticle *p = source->_particles;
  struct particle_meta *m = source->_meta;
  for (int i = 0; i < source->_num_allocated_particles/2; i++, p++, m++) {
    if (p->sizeA) {
      p->posA[1] += y_move;
      if ((int) p->posA[1] >= source->height - 128) {
        p->sizeA = 0;
      }
      else {
        p->posA[0] = m->cx
          + sinf((float) p->posA[1] / T3D_PI)
          * source->movement_amplitude;
        p->posA[2] = m->cz
          + cosf((float) p->posA[1] / T3D_PI)
          * source->movement_amplitude;
        float progress = (float) (p->posA[1] + 128) / (float) source->height;
        p->colorA[3] = roundf((1.f - progress) * 255.f);
      }
    }
    if (!p->sizeA && actual_to_spawn) {
      actual_to_spawn--;
      spawned++;
      particle_source_spawn_steam(source, p->posA, &p->sizeA, m);
    }

    m++;
    if (p->sizeB) {
      p->posB[1] += y_move;
      if ((int) p->posB[1] >= source->height - 128) {
        p->sizeB = 0;
      }
      else {
        p->posB[0] = (float) m->cx
          + sinf((float) p->posB[1] / T3D_PI)
          * (float) source->movement_amplitude;
        p->posB[2] = (float) m->cz
          + cosf((float) p->posB[1] / T3D_PI)
          * (float) source->movement_amplitude;
        float progress = (float) (p->posB[1] + 128) / (float) source->height;
        p->colorB[3] = roundf((1.f - progress) * 255.f);
      }
    }
    if (!p->sizeB && actual_to_spawn) {
      actual_to_spawn--;
      spawned++;
      particle_source_spawn_steam(source, p->posB, &p->sizeB, m);
    }
  }
  source->_to_spawn -= (float) spawned;
}

static void particle_source_iterate_snow(struct particle_source *source,
    float delta_time) {
  source->_y_move_error += (256.f / source->time_to_fall) * delta_time;
  int y_move = (int) source->_y_move_error;
  source->_y_move_error -= (float) y_move;

  TPXParticle *p = source->_particles;
  for (int i = 0; i < source->_num_allocated_particles/2; i++, p++) {
    p->posA[1] -= y_move;
    p->posB[1] -= y_move;
  }
}

static void particle_source_iterate_splash(struct particle_source *source,
    float delta_time) {
  source->_time += delta_time;
  float move = source->_time * source->speed;

  TPXParticle *p = source->_particles;
  struct particle_meta *m = source->_meta;
  size_t num_active = 0;
  for (int i = 0; i < source->_num_allocated_particles/2; i++, p++, m++) {
    if (p->sizeA) {
      if ((int8_t) move > m->d) {
        p->sizeA = 0;
      }
      else {
        p->posA[0] = roundf(move*m->dir[0]);
        p->posA[1] = roundf((float) m->h * sinf(move/(float) m->d * T3D_PI));
        p->posA[2] = roundf(move*m->dir[1]);
      }
    }
    if (p->sizeA) {
      num_active++;
    }

    m++;
    if (p->sizeB) {
      if ((int8_t) move > m->d) {
        p->sizeB = 0;
      }
      else {
        p->posB[0] = roundf(move*m->dir[0]);
        p->posB[1] = roundf((float) m->h * sinf(move/(float) m->d * T3D_PI));
        p->posB[2] = roundf(move*m->dir[1]);
      }
    }
    if (p->sizeB) {
      num_active++;
    }
  }

  if (!num_active) {
    source->paused = true;
    source->render = false;
  }
}

void particle_source_update_transform(struct particle_source *source) {
  t3d_mat4fp_from_srt_euler(source->_transform,
      source->scale.v,
      source->rot.v,
      source->pos.v);
}

void particle_source_iterate(struct particle_source *source,
    float delta_time) {
  if (source->paused) {
    return;
  }
  switch (source->_type) {
    case STEAM:
      particle_source_iterate_steam(source, delta_time);
      break;

    case SNOW:
      particle_source_iterate_snow(source, delta_time);
      break;

    case SPLASH:
      particle_source_iterate_splash(source, delta_time);
      break;
  }
}

void particle_source_draw(const struct particle_source *source) {
  tpx_matrix_push(source->_transform);
  tpx_particle_draw(source->_particles, source->_num_allocated_particles);
  tpx_matrix_pop(1);
}

float rand_float(float min, float max) {
  float r = (float) rand() / (float) RAND_MAX;
  return r*(max-min) + min;
}

void draw_fade(float fade) {
  int w = roundf(fade*320);
  w = w > 320? 320 : w;
  int h = roundf(fade*240);
  h = h > 320? 320 : h;
  int x = (320-w)/2;
  int y = (240-h)/2;
  rdpq_mode_push();
  rdpq_mode_zbuf(false, false);
  rdpq_set_mode_fill(RGBA32(0x0, 0x0, 0x0, 0xff));
  rdpq_fill_rectangle(x, y, x+w, y+h);
  rdpq_mode_pop();
}
