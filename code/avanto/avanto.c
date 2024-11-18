#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#include "defs.h"
#include "sauna.h"

const MinigameDef minigame_def = {
    .gamename = "Avanto",
    .developername = "Flavio Zavan",
    .description = "Whatever description",
    .instructions = "Sauna: Hold Z to crouch\nWater: Blabla"
};

surface_t *z_buffer;
T3DViewport viewport;
struct camera cam;
T3DModel *player_model;
T3DModel *ukko_model;
struct character player;
struct character ukko;
struct scene *current_scene;
rdpq_font_t *debug_font;

void scene_unload() {
  sprite_free(current_scene->bg);
  if (current_scene->z) {
    sprite_free(current_scene->z);
  }
  current_scene = NULL;
}

void scene_load(struct scene *scene) {
  current_scene = scene;

  current_scene->bg = sprite_load(current_scene->bg_path);
  if (current_scene->z_path) {
    current_scene->z = sprite_load(current_scene->z_path);
  }
  else {
    current_scene->z = NULL;
  }

  const struct camera *cam = &current_scene->starting_cam;
  t3d_viewport_set_projection(&viewport, current_scene->fov, 10, 400);
  t3d_viewport_look_at(&viewport,
      &cam->pos,
      &cam->target,
      &(T3DVec3) {{0, 1, 0}});
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
    color_t primary_color) {

  e->model = model;
  e->transform = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_from_srt_euler(e->transform, scale->v, rotation->v, pos->v);
  e->skeleton = skeleton;

  rspq_block_begin();
  t3d_matrix_push(e->transform);
  rdpq_set_prim_color(primary_color);
  if (e->skeleton) {
    t3d_model_draw_skinned(e->model, e->skeleton);
  }
  else {
    t3d_model_draw(e->model);
  }
  t3d_matrix_pop(1);
  e->display_block = rspq_block_end();
}

void entity_free(struct entity *e) {
  free_uncached(e->transform);
  rspq_block_free(e->display_block);
}

float get_ground_height(float z) {
  float height = 0;
  for (size_t i = 0; i < current_scene->ground.num_changes; i++) {
    if (current_scene->ground.changes[i].start_z > z) {
      break;
    }
    height = current_scene->ground.changes[i].height;
  }
  return height;
}

void minigame_init() {
  display_init(RESOLUTION_320x240,
      DEPTH_16_BPP,
      3,
      GAMMA_NONE,
      FILTERS_RESAMPLE_ANTIALIAS);
  z_buffer = display_get_zbuf();

  t3d_init((T3DInitParams){});
  viewport = t3d_viewport_create();

  //player_model = t3d_model_load("rom:/snake3d/snake.t3dm");
  player_model = t3d_model_load("rom:/avanto/guy.t3dm");
  player.rotation = 0;
  skeleton_init(&player.s, player_model, 1);
  //player.s.anims[0] = t3d_anim_create(player_model, "Snake_Idle");
  player.pos = (T3DVec3) {{200, 110, 110}};
  player.scale = 1.f/.5f;
  entity_init(&player.e,
      player_model,
      &(T3DVec3) {{player.scale, player.scale, player.scale}},
      &(T3DVec3) {{0, player.rotation, 0}},
      &player.pos,
      &player.s.skeleton,
      PLAYERCOLOR_1);
  //t3d_anim_attach(&player.s.anims[0], &player.s.skeleton);


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
      RGBA32(0, 0, 0, 255));
  t3d_anim_attach(&ukko.s.anims[THROW], &ukko.s.skeleton);
  t3d_anim_update(&ukko.s.anims[THROW], 0);
  t3d_skeleton_update(&ukko.s.skeleton);
  t3d_anim_set_looping(&ukko.s.anims[THROW], false);
  t3d_anim_set_playing(&ukko.s.anims[THROW], false);

  scene_load(&sauna_scene);

  rdpq_font_t *debug_font= rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
  rdpq_text_register_font(FONT_DEBUG, debug_font);
}

void minigame_fixedloop(float deltaTime) {
  player.pos.v[2] += 1;
  if (player.pos.v[2] >= 400) {
    player.pos.v[2] = 0;
  }
  player.pos.v[1] = get_ground_height(player.pos.v[2]);
}

void minigame_loop(float deltaTime) {
  joypad_buttons_t pressed = joypad_get_buttons_pressed(0);

  if (pressed.start) {
    core_set_winner(0);
    minigame_end();
  }

  if (pressed.a) {
    t3d_anim_set_playing(&ukko.s.anims[THROW], true);
  }

  // Reset Z buffer to match BG
  if (current_scene->z) {
    rdpq_attach(z_buffer, NULL);
    rdpq_mode_push();
    rdpq_set_mode_copy(false);
    rdpq_sprite_blit(current_scene->z, 0, 0, NULL);
    rdpq_mode_pop();
    rdpq_detach();
  }
  else {
    t3d_screen_clear_depth();
  }


  // Render 3D
  rdpq_attach(display_get(), z_buffer);
  t3d_frame_start();
  t3d_viewport_attach(&viewport);

  if (current_scene->do_light) {
    current_scene->do_light();
  }

  // BG
  rdpq_mode_push();
  rdpq_set_mode_copy(false);
  rdpq_sprite_blit(current_scene->bg, 0, 0, NULL);
  rdpq_mode_pop();

  // Player
  //t3d_anim_update(&player.s.anims[0], deltaTime);
  t3d_skeleton_update(&player.s.skeleton);
  t3d_mat4fp_from_srt_euler(player.e.transform,
    (float[3]) {player.scale, player.scale, player.scale},
    (float[3]) {0, player.rotation, 0},
    player.pos.v);
  rspq_block_run(player.e.display_block);

  // Ukko
  t3d_anim_update(&ukko.s.anims[THROW], deltaTime);
  t3d_skeleton_update(&ukko.s.skeleton);
  rspq_block_run(ukko.e.display_block);

  // Debug
  rdpq_text_printf(NULL, FONT_DEBUG, 10, 15,
      "FPS: %.2f", 1.f/deltaTime);

  rdpq_sync_tile();
  rdpq_sync_pipe();
  rdpq_detach_show();

}

void minigame_cleanup() {
  rdpq_text_unregister_font(FONT_DEBUG);
  rdpq_font_free(debug_font);

  scene_unload();

  entity_free(&player.e);
  skeleton_free(&player.s);
  t3d_model_free(player_model);

  entity_free(&ukko.e);
  skeleton_free(&ukko.s);
  t3d_model_free(ukko_model);

  t3d_destroy();
  display_close();
}
