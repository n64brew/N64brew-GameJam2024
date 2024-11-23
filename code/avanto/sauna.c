#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#include "common.h"
#include "sauna.h"

#define LOYLY_SOUND_DELAY .9f
#define LOYLY_DELAY 1.5f
#define LOYLY_LENGTH 4.f
#define LOYLY_SCREEN_MAX_ALPHA 200

extern struct character players[];
extern surface_t *z_buffer;
extern T3DViewport viewport;
extern struct camera cam;

static T3DModel *ukko_model;
static struct character ukko;

static bool loyly_sound_queued;
static bool loyly_queued;
static float loyly_strength;
static wav64_t sfx_loyly;

static void sauna_do_light() {
  uint8_t light_color[4] = {255, 255, 255, 255};
  T3DVec3 lights[] = {
    (T3DVec3) {{-40, 200, 330}},
    (T3DVec3) {{415, 200, 330}},
    (T3DVec3) {{125, 100, 0}},
  };
  t3d_light_set_point(0, light_color, &lights[0], 1, false);
  t3d_light_set_point(1, light_color, &lights[1], 1, false);
  t3d_light_set_point(2, light_color, &lights[2], 1, false);
  t3d_light_set_count(3);
}
static struct scene sauna_scene = {
  .bg_path = "rom:/avanto/sauna.sprite",
  .z_path = "rom:/avanto/sauna-depth.sprite",
  .fov = T3D_DEG_TO_RAD(78.f),
  .starting_cam = {
    .pos = (T3DVec3) {{0, 100, 0}},
    .target = (T3DVec3) {{125, 125, 155}}
  },
  .do_light = sauna_do_light,
  .ground = {
    .num_changes = 3,
    .changes = {
      {-INFINITY, -30},
      {140, 41},
      {230, 110}
    }
  },
};

enum sauna_stages {
  SAUNA_INTRO,
  SAUNA_WALK_IN,
  SAUNA_COUNTDOWN,
  SAUNA_GAME,
  SAUNA_WALK_OUT,
};

void sauna_init() {
  ukko_model = t3d_model_load("rom:/avanto/ukko.t3dm");
  ukko.rotation = T3D_DEG_TO_RAD(90);
  ukko.scale = 3;
  ukko.pos = (T3DVec3) {{400, 41, 150}};
  skeleton_init(&ukko.s, ukko_model, 1);
  ukko.s.anims[THROW] = t3d_anim_create(ukko_model, "throw");
  entity_init(&ukko.e,
      ukko_model,
      &(T3DVec3) {{ukko.scale, ukko.scale, ukko.scale}},
      &(T3DVec3) {{0, ukko.rotation, 0}},
      &ukko.pos,
      &ukko.s.skeleton,
      (T3DModelDrawConf) {NULL, NULL, NULL, NULL, NULL});
  t3d_anim_attach(&ukko.s.anims[THROW], &ukko.s.skeleton);
  t3d_anim_update(&ukko.s.anims[THROW], 0);
  t3d_skeleton_update(&ukko.s.skeleton);
  t3d_anim_set_looping(&ukko.s.anims[THROW], false);
  t3d_anim_set_playing(&ukko.s.anims[THROW], false);
  loyly_sound_queued = false;
  loyly_queued = false;
  loyly_strength = 0.f;

  sauna_scene.bg = sprite_load(sauna_scene.bg_path);
  sauna_scene.z = sprite_load(sauna_scene.z_path);
  const struct camera *cam = &sauna_scene.starting_cam;
  t3d_viewport_set_projection(&viewport, sauna_scene.fov, 10, 400);
  t3d_viewport_look_at(&viewport,
      &cam->pos,
      &cam->target,
      &(T3DVec3) {{0, 1, 0}});

  wav64_open(&sfx_loyly, "rom:/avanto/loyly.wav64");

  for (size_t i = 0; i < 4; i++) {
    players[i].pos = (T3DVec3) {{50*(i+1), 110, 110}};
  }
}

void sauna_fixed_loop(float delta_time) {
  for (size_t i = 0; i < 4; i++) {
    players[i].pos.v[2] += 1;
    if (players[i].pos.v[2] >= 400) {
      players[i].pos.v[2] = 0;
    }
    players[i].pos.v[1] = get_ground_height(players[i].pos.v[2],
        &sauna_scene.ground);
  }
}

void sauna_loop(float delta_time) {
  joypad_buttons_t pressed = joypad_get_buttons_pressed(0);

  if (pressed.start) {
    core_set_winner(0);
    minigame_end();
  }

  if (pressed.a) {
    t3d_anim_set_playing(&ukko.s.anims[THROW], true);
    loyly_sound_queued = true;
    loyly_queued = true;
  }

  if (loyly_strength > 0) {
    loyly_strength -= delta_time/LOYLY_LENGTH;
  }
  if (loyly_sound_queued
      && ukko.s.anims[THROW].time + delta_time >= LOYLY_SOUND_DELAY) {
    wav64_play(&sfx_loyly, get_next_sfx_channel());
    loyly_sound_queued = false;
  }
  if (loyly_queued && ukko.s.anims[THROW].time + delta_time >= LOYLY_DELAY) {
    loyly_strength = 1.f;
    loyly_queued = false;
  }

  // Reset Z buffer to match BG
  rdpq_attach(z_buffer, NULL);
  rdpq_mode_push();
  rdpq_set_mode_copy(false);
  rdpq_sprite_blit(sauna_scene.z, 0, 0, NULL);
  rdpq_mode_pop();
  rdpq_detach();

  // Render 3D
  rdpq_attach(display_get(), z_buffer);
  t3d_frame_start();
  t3d_viewport_attach(&viewport);

  // BG
  rdpq_mode_push();
  rdpq_set_mode_copy(false);
  rdpq_sprite_blit(sauna_scene.bg, 0, 0, NULL);
  rdpq_mode_pop();

  sauna_scene.do_light();

  // Players
  for (size_t i = 0; i < 4; i++) {
    //t3d_anim_update(&players[i].s.anims[0], delta_time);
    t3d_skeleton_update(&players[i].s.skeleton);
    t3d_mat4fp_from_srt_euler(players[i].e.transform,
      (float[3]) {players[i].scale, players[i].scale, players[i].scale},
      (float[3]) {0, players[i].rotation, 0},
      players[i].pos.v);
    rspq_block_run(players[i].e.display_block);
  }

  // Ukko
  t3d_anim_update(&ukko.s.anims[THROW], delta_time);
  t3d_skeleton_update(&ukko.s.skeleton);
  rspq_block_run(ukko.e.display_block);

  if (loyly_strength > 0) {
    rdpq_mode_push();
    int screen_alpha = (int) (LOYLY_SCREEN_MAX_ALPHA*loyly_strength);
    rdpq_set_fog_color(RGBA32(0xff, 0xff, 0xff, screen_alpha));
    rdpq_set_mode_standard();
    rdpq_set_prim_color(RGBA32(0xff, 0xff, 0xff, 0xff));
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
    rdpq_fill_rectangle(0, 0, 320, 240);
    rdpq_mode_pop();
  }

  // Debug
  rdpq_text_printf(NULL, FONT_DEBUG, 10, 15,
      "FPS: %.2f", 1.f/delta_time);

  rdpq_sync_tile();
  rdpq_sync_pipe();
  rdpq_detach_show();
}

void sauna_cleanup() {
  sprite_free(sauna_scene.bg);
  sprite_free(sauna_scene.z);

  wav64_close(&sfx_loyly);

  entity_free(&ukko.e);
  skeleton_free(&ukko.s);
  t3d_model_free(ukko_model);
}

