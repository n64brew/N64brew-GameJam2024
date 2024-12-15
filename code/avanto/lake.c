#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/tpx.h>

#include "common.h"
#include "lake.h"

#define MAP_SCALE 1.f
#define FOV T3D_DEG_TO_RAD(60.f)
#define FOCUS_Y 36.f
#define FOCUS_X (PLAYER_MIN_X+1.5f*PLAYER_DISTANCE)
#define CAMERA_X -500.f
#define CAMERA_Y 380.f
#define WATER_S_SPEED -4.f
#define WATER_T_SPEED -2.f
#define PLAYER_MIN_X -125.f
#define PLAYER_DISTANCE 60.f
#define PLAYER_STARTING_Z -490.f
#define RACE_START_Z 1050.f
#define RACE_END_Z (80.f*64.f)
#define CELEB_Z (86.5f*64.f)
#define LAKE_WALK_SPEED 200.f
#define MAX_TIME 60.f
#define TEMP_LOSS_TIME 30.f
#define PENALTY_TIME 1.f
#define SPEED_GAIN 20.f
#define HEAT_BONUS 1.5f
#define NUM_BUTTONS 13
#define MAX_CAM_Z_OFFSET 200.f
#define INST_X_OFFSET_LEFT 50
#define INST_X_OFFSET_RIGHT 35
#define INST_MIN_X 30
#define INST_MAX_Y 170
#define INST_Y_GAP 22
#define SHADOW_SCALE .6f
#define NUM_SNOW_PARTICLES 512
#define NUM_STEAM_PARTICLES 32
#define LAKE_TIME 90.f
#define NUM_MOVES_TO_FINISH 64
#define ADVANCE_PER_STROKE ((RACE_END_Z-RACE_START_Z)/NUM_MOVES_TO_FINISH)
#define SWIM_SPEED 150.f
#define NUM_SPLASH_SOURCES 4
#define MIN_SPLASH_PARTICLES 8
#define MAX_SPLASH_PARTICLES 16
#define MINIGAME_CHANNEL 31
#define FIRST_SPLASH_CHANNEL 8
#define CHANCE_TO_SPLASH .1f
#define SPLASH_INTERVAL .5f
#define WATER_Y (-1.5f*64.f)

enum lake_stages {
  LAKE_INTRO,
  LAKE_GAME,
  LAKE_OUTRO,
  LAKE_END,
  LAKE_FADE_OUT,
  NUM_LAKE_STAGES,
};

struct button {
  sprite_t *sprite;
  uint16_t mask;
};

struct lake_ai {
  size_t pid;
  void (*handler) (struct lake_ai *,
      joypad_buttons_t *,
      joypad_buttons_t *,
      float);
  float miss_chance;
  float min_delay;
  float max_delay;
  float time_to_next;
};

extern T3DViewport viewport;
extern struct character players[];
extern wav64_t sfx_start;
extern wav64_t sfx_stop;
extern wav64_t sfx_winner;
extern const char *const PLAYER_TITLES[];
extern struct rdpq_textparms_s banner_params;
extern struct rdpq_textparms_s timer_params;
extern xm64player_t music;
extern struct camera cam;

static struct entity map;
static T3DModel *map_model;
static struct entity shadows[4];
static T3DModel *shadow_model;
static T3DObject *water_object;
static struct ground ground = {
  .num_changes = 6,
  .changes = {
    {-INFINITY, 0.f, false},
    {450.f, 10.f, false},
    {970.f, -64.f, false},
    {RACE_START_Z, -64.f*2.5f, false},
    {RACE_END_Z, -64.f*2.f, true},
    {83.4f*64.f, 0.f, false},
  },
};
static bool lake_stage_inited[NUM_LAKE_STAGES];
static size_t lake_stage;
static wav64_t sfx_splash[NUM_SPLASH_SOURCES];
static float penalties[4];
static const struct button *next_buttons[4];
static struct button buttons[NUM_BUTTONS];
static sprite_t *penalty_sprite;
static sprite_t *balloon_sprite;
static sprite_t *tail_sprite;
rdpq_blitparms_t tail_params;
static uint8_t winners_mask;
static char banner_str[32];
static float min_time_before_exiting;
static struct lake_ai ais[4];
static const T3DBone *leg_bones[4][2];
static struct particle_source snow_particle_source;
static float time_left;
static float expected_zs[4];
static struct particle_source steam_sources[4];
static struct particle_source splash_sources[NUM_SPLASH_SOURCES];
static float splash_cooldown;
static float fade;
static bool last_held_was_clean[4];

static void lake_intro_dynamic_loop(float delta_time);
static void lake_outro_dynamic_loop(float delta_time);

static void generate_splash(T3DVec3 pos, bool silent) {
  struct particle_source *source = NULL;
  size_t index;
  for (size_t i = 0; i < NUM_SPLASH_SOURCES; i++) {
    if (splash_sources[i].paused) {
      source = &splash_sources[i];
      index = i;
      break;
    }
  }

  if (!source) {
    return;
  }

  size_t num_particles = rand() % (MAX_SPLASH_PARTICLES-MIN_SPLASH_PARTICLES);
  num_particles += MIN_SPLASH_PARTICLES;

  source->pos = pos;
  particle_source_reset_splash(source, num_particles);
  particle_source_update_transform(source);
  if (!silent) {
    wav64_play(&sfx_splash[index], FIRST_SPLASH_CHANNEL+index);
  }
}

static void generate_one_splash_per_player() {
  bool silent = false;
  for (size_t i = 0; i < 4; i++) {
    if (players[i].out) {
      continue;
    }
    generate_splash(
        (T3DVec3) {{players[i].pos.v[0], WATER_Y, players[i].pos.v[2]}},
        silent);
    silent = true;
  }
}

static void ai_standard(struct lake_ai *ai,
    joypad_buttons_t *pressed,
    joypad_buttons_t *held,
    float delta_time) {
  pressed->raw = 0;
  held->raw = 0;

  float time_error = 0.f;
  if (ai->time_to_next > 1000.f) {
    ai->time_to_next = rand_float(ai->min_delay, ai->max_delay);
  } else {
    time_error = delta_time - ai->time_to_next;
    ai->time_to_next -= delta_time;
  }

  if (penalties[ai->pid] >= EPS || ai->time_to_next >= EPS) {
    return;
  }

  if (rand_float(0.f, 1.f) < ai->miss_chance) {
    pressed->a = 1;
    pressed->b = 1;
  }
  else {
    pressed->raw = next_buttons[ai->pid]->mask;
  }
  ai->time_to_next = rand_float(ai->min_delay, ai->max_delay) - time_error;
}

static void ai_init(struct lake_ai *ai, size_t pid, AiDiff diff) {
  ai->pid = pid;
  ai->handler = ai_standard;
  ai->time_to_next = INFINITY;

  if (diff == DIFF_EASY) {
    ai->miss_chance = .04f;
    ai->min_delay = .7f;
    ai->max_delay = 1.2f;
  }
  else if (diff == DIFF_MEDIUM) {
    ai->miss_chance = .03f;
    ai->min_delay = .6f;
    ai->max_delay = 1.1f;
  }
  else {
    ai->miss_chance = .02f;
    ai->min_delay = .4f;
    ai->max_delay = .9f;
  }
}

static void update_cam(float z) {
  cam.target.v[2] = z;
  cam.pos.v[2] = z;
  cam.pos.v[0] = CAMERA_X;
  cam.target.v[0] = FOCUS_X;
  t3d_viewport_look_at(&viewport,
      &cam.pos,
      &cam.target,
      &(T3DVec3) {{0, 1, 0}});
}

bool filter_out_water(void *user_data, const T3DObject *object) {
  if (!strcmp("water", object->name)) {
    return false;
  }
  return true;
}

static void load_buttons() {
  joypad_buttons_t b;

  b.raw = 0;
  b.a = 1;
  buttons[0].sprite = sprite_load("rom:/core/AButton.sprite");
  buttons[0].mask = b.raw;

  b.raw = 0;
  b.b = 1;
  buttons[1].sprite = sprite_load("rom:/core/BButton.sprite");
  buttons[1].mask = b.raw;

  b.raw = 0;
  b.c_up = 1;
  buttons[2].sprite = sprite_load("rom:/core/CUp.sprite");
  buttons[2].mask = b.raw;

  b.raw = 0;
  b.c_down = 1;
  buttons[3].sprite = sprite_load("rom:/core/CDown.sprite");
  buttons[3].mask = b.raw;

  b.raw = 0;
  b.c_left = 1;
  buttons[4].sprite = sprite_load("rom:/core/CLeft.sprite");
  buttons[4].mask = b.raw;

  b.raw = 0;
  b.c_right = 1;
  buttons[5].sprite = sprite_load("rom:/core/CRight.sprite");
  buttons[5].mask = b.raw;

  b.raw = 0;
  b.d_up = 1;
  buttons[6].sprite = sprite_load("rom:/core/DUp.sprite");
  buttons[6].mask = b.raw;

  b.raw = 0;
  b.d_down = 1;
  buttons[7].sprite = sprite_load("rom:/core/DDown.sprite");
  buttons[7].mask = b.raw;

  b.raw = 0;
  b.d_left = 1;
  buttons[8].sprite = sprite_load("rom:/core/DLeft.sprite");
  buttons[8].mask = b.raw;

  b.raw = 0;
  b.d_right = 1;
  buttons[9].sprite = sprite_load("rom:/core/DRight.sprite");
  buttons[9].mask = b.raw;

  b.raw = 0;
  b.l = 1;
  buttons[10].sprite = sprite_load("rom:/core/LTrigger.sprite");
  buttons[10].mask = b.raw;

  b.raw = 0;
  b.r = 1;
  buttons[11].sprite = sprite_load("rom:/core/RTrigger.sprite");
  buttons[11].mask = b.raw;

  b.raw = 0;
  b.z = 1;
  buttons[12].sprite = sprite_load("rom:/core/ZTrigger.sprite");
  buttons[12].mask = b.raw;
}

const struct button *get_next_button(const struct button *prev) {
  struct button *b = NULL;
  do {
    b = &buttons[rand() % NUM_BUTTONS];
  } while(b == prev);
  return b;
}

void lake_init() {
  cam.target = (T3DVec3) {{FOCUS_X, FOCUS_Y, 0.f}};
  cam.pos = (T3DVec3) {{CAMERA_X, CAMERA_Y, 0.f}};

  T3DModelDrawConf map_draw_conf = {
    .userData = NULL,
    .tileCb = NULL,
    .filterCb = filter_out_water,
    .dynTextureCb = NULL,
    .matrices = NULL,
  };
  map_model = t3d_model_load("rom:/avanto/map.t3dm");
  water_object = NULL;
  T3DModelIter it = t3d_model_iter_create(map_model, T3D_CHUNK_TYPE_OBJECT);
  while (t3d_model_iter_next(&it)) {
    if (!strcmp("water", it.object->name)) {
      water_object = it.object;
      break;
    }
  }
  entity_init(&map,
      map_model,
      &(T3DVec3) {{MAP_SCALE, MAP_SCALE, MAP_SCALE}},
      &(T3DVec3) {{0.f, 0.f, 0.f}},
      &(T3DVec3) {{0.f, 0.f, 0.f}},
      NULL,
      &map_draw_conf);

  shadow_model = t3d_model_load("rom:/avanto/shadow.t3dm");
  for (size_t i = 0; i < 4; i++) {
    entity_init(&shadows[i],
        shadow_model,
        &(T3DVec3) {{1.f, 1.f, 1.f}},
        &(T3DVec3) {{0.f, 0.f, 0.f}},
        &(T3DVec3) {{0.f, 0.f, 0.f}},
        NULL,
        NULL);
  }

  t3d_viewport_set_projection(&viewport, FOV, 50, 5000);
  update_cam(PLAYER_STARTING_Z);

  static T3DVec3 light_dir[] = {
    (T3DVec3) {{1.f, 1.f, 1.f}},
    (T3DVec3) {{-1.f, 1.f, -1.f}},
  };
  static uint8_t light_color[] = {0xff, 0xff, 0xff, 0xff};
  static uint8_t ambient_color[] = {0x40, 0x9c, 0xff, 0xff};
  t3d_vec3_norm(&light_dir[0]);
  t3d_vec3_norm(&light_dir[1]);
  t3d_light_set_directional(0, light_color, &light_dir[0]);
  t3d_light_set_directional(1, light_color, &light_dir[1]);
  t3d_light_set_ambient(ambient_color);
  t3d_light_set_count(2);

  memset(ais, 0, sizeof(ais));
  for (size_t i = 0; i < 4; i++) {
    players[i].visible = !players[i].out;
    players[i].rotation = 0.f;
    players[i].pos =
      (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*i, 0.f, PLAYER_STARTING_Z}};
    t3d_anim_attach(&players[i].s.anims[WALK], &players[i].s.skeleton);
    t3d_anim_update(&players[i].s.anims[WALK], 0);
    players[i].current_anim = WALK;
    t3d_anim_set_playing(&players[i].s.anims[players[i].current_anim], false);
    players[i].scale = 1.f;
    penalties[i] = 0.f;
    expected_zs[i] = RACE_START_Z;
    next_buttons[i] = get_next_button(NULL);
    last_held_was_clean[i] = false;
    if (i >= core_get_playercount()) {
      ai_init(&ais[i], i, core_get_aidifficulty());
    }

    int bone_index;
    bone_index = t3d_skeleton_find_bone(&players[i].s.skeleton, "LeftLeg");
    leg_bones[i][0] = bone_index == -1?
      NULL : &players[i].s.skeleton.bones[bone_index];
    bone_index = t3d_skeleton_find_bone(&players[i].s.skeleton, "RightLeg");
    leg_bones[i][1] = bone_index == -1?
      NULL : &players[i].s.skeleton.bones[bone_index];

    particle_source_init(&steam_sources[i], NUM_STEAM_PARTICLES, STEAM);
    steam_sources[i].render = !players[i].out;
    steam_sources[i].scale = (T3DVec3) {{1.f, 1.f, 1.f}};
    steam_sources[i].rot = (T3DVec3) {{0.f, 0.f, 0.f}};
    steam_sources[i].x_range = 15;
    steam_sources[i].z_range = 20;
    steam_sources[i].height = 64.f * 2.f;
    steam_sources[i].particle_size = 1;
    steam_sources[i].time_to_rise = 5.f;
    steam_sources[i].movement_amplitude = 5.f;
    steam_sources[i].paused = players[i].out;
  }

  for (size_t i = 0; i < NUM_SPLASH_SOURCES; i++) {
    particle_source_init(&splash_sources[i], MAX_SPLASH_PARTICLES, SPLASH);
    splash_sources[i].render = false;
    splash_sources[i].paused = true;
    splash_sources[i].speed = 64.f;
    splash_sources[i].min_dist = 8.f;
    splash_sources[i].max_dist = 48.f;
    splash_sources[i].min_height = 8.f;
    splash_sources[i].max_height = 24.f;
    splash_sources[i].particle_size = 4;
    splash_sources[i].scale = (T3DVec3) {{1.f, 1.f, 1.f}};
    splash_sources[i].rot = (T3DVec3) {{0.f, 0.f, 0.f}};
    wav64_open(&sfx_splash[i], "rom:/avanto/splash.wav64");
  }

  load_buttons();
  penalty_sprite = sprite_load("rom:/avanto/penalty.sprite");
  balloon_sprite = sprite_load("rom:/avanto/balloon.sprite");
  tail_sprite = sprite_load("rom:/avanto/tail.sprite");
  memset(&tail_params, 0, sizeof(tail_params));

  winners_mask = 0;
  banner_str[0] = 0;
  min_time_before_exiting = 3.f;

  snow_particle_source.render = true;
  snow_particle_source.time_to_fall = 5.f;
  snow_particle_source.paused = false;
  snow_particle_source.rot = (T3DVec3) {{0.f, 0.f, 0.f}};
  // Hardcoded for map at scale 1
  const float span[3][2] = {
    {-10.305f*64.f, 4.6f*64.f},
    {-1.5f*64.f, 10.f*64.f},
    {-8.78f*64.f, 92.437f*64.f},
  };
  float longest = span[2][1] - span[2][0];
  float scale = longest/255.f;
  snow_particle_source.x_range = 127.f * ((span[0][1]-span[0][0])/longest);
  snow_particle_source.z_range = 127.f * ((span[2][1]-span[2][0])/longest);
  snow_particle_source.scale = (T3DVec3) {{
    scale,
    (span[1][1]-span[1][0])/256.f,
    scale
  }};
  snow_particle_source.pos = (T3DVec3) {{
    (span[0][1]+span[0][0])/2.f,
    span[1][1]-128.f*snow_particle_source.scale.v[1],
    (span[2][1]+span[2][0])/2.f,
  }};
  particle_source_init(&snow_particle_source, NUM_SNOW_PARTICLES, SNOW);
  particle_source_update_transform(&snow_particle_source);

  time_left = INFINITY;
  splash_cooldown = 0.f;
  fade = 1.f;

  for (size_t i = 0; i < NUM_LAKE_STAGES; i++) {
    lake_stage_inited[i] = false;
  }
  lake_stage = LAKE_INTRO;
}

void lake_dynamic_loop_pre(float delta_time) {
  joypad_buttons_t pressed[4];
  joypad_buttons_t held[4];
  for (size_t i = 0; i < core_get_playercount(); i++) {
    pressed[i] = joypad_get_buttons_pressed(core_get_playercontroller(i));
    held[i] = joypad_get_buttons_held(core_get_playercontroller(i));
  }
  for (size_t i = core_get_playercount(); i < 4; i++) {
    if (lake_stage == LAKE_GAME) {
      ais[i].handler(&ais[i], &pressed[i], &held[i], delta_time);
    }
    else {
      pressed[i].raw = 0;
    }
  }

  // Logic handling camera movement needs to follow framerate
  if (lake_stage == LAKE_INTRO) {
    lake_intro_dynamic_loop(delta_time);
  }
  else if (lake_stage == LAKE_OUTRO) {
    lake_outro_dynamic_loop(delta_time);
  }

  if (lake_stage == LAKE_END) {
    if (min_time_before_exiting >= EPS) {
      min_time_before_exiting -= delta_time;
    }
    else {
      for (size_t i = 0; i < core_get_playercount(); i++) {
        size_t c = core_get_playercontroller(i);
        if (pressed[c].a || pressed[c].b) {
          lake_stage++;
        }
      }
    }
  }

  if (lake_stage != LAKE_GAME) {
    return;
  }

  if (time_left > LAKE_TIME) {
    time_left = LAKE_TIME;
  }
  else {
    time_left -= delta_time;
  }
  if (time_left < EPS) {
    lake_stage++;
    return;
  }

  bool ended = false;
  int num_in = 0;
  float avg_z = 0.f;
  float max_z = -INFINITY;
  if (splash_cooldown >= EPS) {
    splash_cooldown -= delta_time;
  }
  for (size_t i = 0; i < 4; i++) {
    if (players[i].out) {
      continue;
    }
    num_in++;
    float bonus_mul = players[i].temperature >= EPS? HEAT_BONUS : 1.f;

    if (players[i].pos.v[2] < expected_zs[i]) {
      if (splash_cooldown < EPS && rand_float(0, 1.f) < CHANCE_TO_SPLASH) {
        generate_splash(
            (T3DVec3) {{players[i].pos.v[0], WATER_Y, players[i].pos.v[2]}},
            false);
        splash_cooldown = SPLASH_INTERVAL;
      }
      float nz = players[i].pos.v[2] + SWIM_SPEED*bonus_mul*delta_time;
      if (nz > expected_zs[i]) {
        nz = expected_zs[i];
      }
      players[i].pos.v[2] = nz;
      t3d_anim_set_playing(&players[i].s.anims[players[i].current_anim], true);
    } else {
      t3d_anim_set_looping(&players[i].s.anims[players[i].current_anim],
          false);
    }

    avg_z += players[i].pos.v[2];
    max_z = players[i].pos.v[2] > max_z? players[i].pos.v[2] : max_z;

    if (RACE_END_Z - players[i].pos.v[2] < EPS) {
      ended = true;
      players[i].pos.v[2] = RACE_END_Z;
      winners_mask |= 1 << i;
      core_set_winner(i);
      continue;
    }

    if (penalties[i] >= EPS) {
      penalties[i] -= delta_time;
    }
    else if (last_held_was_clean[i]) {
      if (pressed[i].raw == next_buttons[i]->mask) {
        expected_zs[i] += ADVANCE_PER_STROKE*bonus_mul;
        next_buttons[i] = get_next_button(next_buttons[i]);
      }
      else if (pressed[i].raw && !pressed[i].start) {
        penalties[i] = PENALTY_TIME;
      }
    }

    if (players[i].temperature >= EPS) {
      players[i].temperature -= delta_time / TEMP_LOSS_TIME;
    }

    last_held_was_clean[i] = !held[i].raw;
  }
  avg_z /= (float) num_in;

  float cam_z = max_z - avg_z < MAX_CAM_Z_OFFSET?
    avg_z : max_z - MAX_CAM_Z_OFFSET;
  update_cam(cam_z);

  if (ended) {
    lake_stage++;
  }
}

static void update_water_offset(float delta_time) {
  T3DMaterialTexture *t = &water_object->material->textureA;

  t->s.low += WATER_S_SPEED * delta_time;
  t->s.low = fm_fmodf(t->s.low, t->s.height);

  t->t.low += WATER_T_SPEED * delta_time;
  t->t.low = fm_fmodf(t->t.low, t->t.height);
}

void lake_dynamic_loop_render(float delta_time) {
  if (!lake_stage_inited[LAKE_INTRO]) {
    return;
  }

  t3d_screen_clear_color(RGBA32(0x00, 0xb5, 0xe2, 0xff));
  t3d_screen_clear_depth();

  // Map
  rspq_block_run(map.display_block);

  // Water
  update_water_offset(delta_time);
  t3d_matrix_push(map.transform);
  t3d_model_draw_material(water_object->material, NULL);
  t3d_model_draw_object(water_object, NULL);
  t3d_matrix_pop(1);

  for (size_t i = 0; i < 4; i++) {
    if (!players[i].visible) {
      continue;
    }

    if (players[i].current_anim != -1) {
      t3d_anim_update(&players[i].s.anims[players[i].current_anim],
          delta_time);
      t3d_skeleton_update(&players[i].s.skeleton);
    }

    T3DMat4 player_matrix;
    t3d_mat4_from_srt_euler(&player_matrix,
      (float[3]) {players[i].scale, players[i].scale, players[i].scale},
      (float[3]) {0, players[i].rotation, 0},
      players[i].pos.v);
    t3d_mat4_to_fixed_3x4(players[i].e.transform, &player_matrix);
    rspq_block_run(players[i].e.display_block);

    T3DVec3 tmp;
    T3DVec3 tmp2;

    T3DVec3 left_pos;
    tmp = leg_bones[i][0]->position;
    t3d_mat3_mul_vec3(&tmp2, &leg_bones[i][0]->matrix, &tmp);
    t3d_mat3_mul_vec3(&left_pos, &player_matrix, &tmp2);

    T3DVec3 right_pos;
    tmp = leg_bones[i][1]->position;
    t3d_mat3_mul_vec3(&tmp2, &leg_bones[i][1]->matrix, &tmp);
    t3d_mat3_mul_vec3(&right_pos, &player_matrix, &tmp2);

    t3d_vec3_add(&tmp, &left_pos, &right_pos);
    t3d_vec3_scale(&tmp2, &tmp, .5f);

    T3DVec3 shadow_pos;
    t3d_vec3_add(&shadow_pos, &players[i].pos, &tmp2);
    shadow_pos.v[1] = get_ground_height(players[i].pos.v[2], &ground) + 4.f;

    t3d_mat4fp_from_srt_euler(shadows[i].transform,
      (float[3]) {SHADOW_SCALE, SHADOW_SCALE, SHADOW_SCALE},
      (float[3]) {get_ground_angle(shadow_pos.v[2], &ground), 0, 0},
      shadow_pos.v);
    rspq_block_run(shadows[i].display_block);
  }

  // Particles
  rdpq_sync_pipe();

  rdpq_mode_push();
  rdpq_set_mode_standard();
  rdpq_mode_zbuf(true, true);
  rdpq_mode_zoverride(true, 0, 0);
  rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
  rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
  tpx_state_from_t3d();

  tpx_state_set_scale(1.f, 1.f);
  particle_source_draw(&snow_particle_source);

  for (size_t i = 0; i < NUM_SPLASH_SOURCES; i++) {
    if (!splash_sources[i].render) {
      continue;
    }
    particle_source_draw(&splash_sources[i]);
  }

  tpx_state_set_scale(.5f, 1.f);
  for (size_t i = 0; i < 4; i++) {
    if (!steam_sources[i].render) {
      continue;
    }

    steam_sources[i].pos = players[i].pos;
    steam_sources[i].pos.v[1] += 128.f + 1.3f*64.f;
    steam_sources[i].rot.v[1] = players[i].rotation;
    particle_source_update_transform(&steam_sources[i]);
    particle_source_draw(&steam_sources[i]);
  }
  tpx_state_set_scale(1.f, 1.f);

  rdpq_sync_pipe();
  rdpq_mode_pop();

  // Sync before manual draws
  rdpq_sync_load();

  if (lake_stage == LAKE_GAME) {
    rdpq_mode_push();
    rdpq_mode_zbuf(false, false);
    for (size_t i = 0; i < 4; i++) {
      if (players[i].out) {
        continue;
      }

      T3DVec3 player_head = {{
        players[i].pos.v[0],
        players[i].pos.v[1] + 1.8f*64.f,
        players[i].pos.v[2],
      }};
      T3DVec3 p_pos;
      t3d_viewport_calc_viewspace_pos(&viewport, &p_pos, &player_head);
      int inst_x = p_pos.v[0] < 100.f?
        (int) p_pos.v[0] + INST_X_OFFSET_RIGHT
        : (int) p_pos.v[0] - INST_X_OFFSET_LEFT;
      inst_x = inst_x < INST_MIN_X? INST_MIN_X : inst_x;
      int inst_y = INST_MAX_Y - i*INST_Y_GAP;

      int tail_x;
      if (inst_x < (int) p_pos.v[0]) {
        tail_params.flip_x = false;
        tail_x = inst_x + balloon_sprite->width - 2;
      }
      else {
        tail_x = inst_x - tail_sprite->width + 1;
        tail_params.flip_x = true;
      }
      int tail_y = (int) p_pos.v[1];
      if (tail_y < inst_y) {
        tail_y = inst_y;
      }
      else if (tail_y >= inst_y+balloon_sprite->height-tail_sprite->height) {
        tail_y = inst_y + balloon_sprite->height - tail_sprite->height;
      }

      rdpq_mode_push();
      rdpq_set_mode_standard();
      rdpq_mode_alphacompare(0xff);
      rdpq_sprite_blit(balloon_sprite, inst_x, inst_y, NULL);
      rdpq_sprite_blit(tail_sprite, tail_x, tail_y, &tail_params);
      rdpq_sprite_blit(
          next_buttons[i]->sprite,
          inst_x+16,
          inst_y+3,
          NULL);
      if (penalties[i] >= EPS) {
        rdpq_sprite_blit(
            penalty_sprite,
            inst_x+16,
            inst_y+3,
            NULL);
      }
      rdpq_mode_pop();

      MITIGATE_FONT_BUG;
      rdpq_text_print(NULL,
          FONT_NORMAL,
          inst_x+3,
          inst_y+14,
          PLAYER_TITLES[i]);
    }
    rdpq_mode_pop();
  }

  if (lake_stage < LAKE_OUTRO) {
    draw_hud();
  }

  if (lake_stage == LAKE_GAME) {
    MITIGATE_FONT_BUG;
    rdpq_text_printf(&timer_params,
      FONT_TIMER,
      0,
      TIMER_Y,
      "%.0f",
      ceilf(time_left));
  }

  if (banner_str[0]) {
    rdpq_mode_push();
    rdpq_mode_zbuf(false, false);
    MITIGATE_FONT_BUG;
    rdpq_text_print(&banner_params, FONT_BANNER, 0, 120, banner_str);
    rdpq_mode_pop();
  }

  if (fade >= EPS) {
    draw_fade(fade);
  }
}

void lake_dynamic_loop_post(float delta_time) {
}

struct script_action intro_actions[][9] = {
  {
    {
      .type = ACTION_MOVE_CAMERA_TO,
      .pos = (T3DVec3) {{FOCUS_X-200.f, FOCUS_Y*1.5f, RACE_END_Z + 5.f*64.f}},
      .target = (T3DVec3) {{FOCUS_X, FOCUS_Y*1.5f, RACE_END_Z + 5.f*64.f}},
      .travel_time = 0.f,
    },
    {.type = ACTION_WAIT, .time = 2.f},
    {
      .type = ACTION_MOVE_CAMERA_TO,
      .pos = (T3DVec3) {{FOCUS_X-200.f, FOCUS_Y*1.5f, RACE_END_Z+4.5f*64.f}},
      .target = (T3DVec3) {{FOCUS_X, FOCUS_Y*1.5f, RACE_END_Z}},
      .travel_time = 1.f,
    },
    {
      .type = ACTION_MOVE_CAMERA_TO,
      .pos = (T3DVec3) {{FOCUS_X-200, FOCUS_Y*1.5f, PLAYER_STARTING_Z+200}},
      .target = (T3DVec3) {{FOCUS_X, FOCUS_Y*1.5f, PLAYER_STARTING_Z}},
      .travel_time = 5.f,
    },
    {.type = ACTION_SEND_SIGNAL, .signal = 0},
    {
      .type = ACTION_MOVE_CAMERA_TO,
      .pos = (T3DVec3) {{FOCUS_X-200, FOCUS_Y*1.5f, RACE_START_Z+200}},
      .target = (T3DVec3) {{FOCUS_X, FOCUS_Y*1.5f, RACE_START_Z}},
      .travel_time = (RACE_START_Z-PLAYER_STARTING_Z)/LAKE_WALK_SPEED,
    },
    {.type = ACTION_CALLBACK, .callback = generate_one_splash_per_player},
    {
      .type = ACTION_MOVE_CAMERA_TO,
      .pos = (T3DVec3) {{CAMERA_X, CAMERA_Y, RACE_START_Z}},
      .target = (T3DVec3) {{FOCUS_X, FOCUS_Y, RACE_START_Z}},
      .travel_time = 2.f,
    },
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT_FOR_SIGNAL, .signal = 0},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X, 0, RACE_START_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_START_ANIM, .anim = SWIM},
    {.type = ACTION_ANIM_UPDATE_TO_TS, .time = 0.f},
    {.type = ACTION_ANIM_SET_PLAYING, .playing = true},
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT_FOR_SIGNAL, .signal = 0},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*1.f, 0, RACE_START_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_START_ANIM, .anim = SWIM},
    {.type = ACTION_ANIM_UPDATE_TO_TS, .time = 0.f},
    {.type = ACTION_ANIM_SET_PLAYING, .playing = true},
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT_FOR_SIGNAL, .signal = 0},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*2.f, 0, RACE_START_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_START_ANIM, .anim = SWIM},
    {.type = ACTION_ANIM_UPDATE_TO_TS, .time = 0.f},
    {.type = ACTION_ANIM_SET_PLAYING, .playing = true},
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT_FOR_SIGNAL, .signal = 0},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*3.f, 0, RACE_START_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_START_ANIM, .anim = SWIM},
    {.type = ACTION_ANIM_UPDATE_TO_TS, .time = 0.f},
    {.type = ACTION_ANIM_SET_PLAYING, .playing = true},
    {.type = ACTION_END},
  },
};
static void lake_intro_dynamic_loop(float delta_time) {
  static struct script_state script_states[5];
  if (!lake_stage_inited[LAKE_INTRO]) {
    lake_stage_inited[LAKE_INTRO] = true;
    script_reset_signals();
    script_states[0] = (struct script_state)
      {.character = NULL, .action = intro_actions[0], .time = 0.f};
    script_states[1] = (struct script_state)
      {.character = &players[0], .action = intro_actions[1], .time = 0.f};
    script_states[2] = (struct script_state)
      {.character = &players[1], .action = intro_actions[2], .time = 0.f};
    script_states[3] = (struct script_state)
      {.character = &players[2], .action = intro_actions[3], .time = 0.f};
    script_states[4] = (struct script_state)
      {.character = &players[3], .action = intro_actions[4], .time = 0.f};
    delta_time = 0.f;
  }
  else if (fade >= EPS) {
    fade -= delta_time;
  }


  bool done = true;
  for (size_t i = 0; i < 5; i++) {
    if ((!script_states[i].character || !script_states[i].character->out)
        && !script_update(&script_states[i], delta_time)) {
      done = false;
    }
  }

  if (done) {
    wav64_play(&sfx_start, MINIGAME_CHANNEL);
    lake_stage++;
  }
}

struct script_action outro_actions[][4] = {
  {
    {
      .type = ACTION_MOVE_CAMERA_TO,
      .pos = (T3DVec3) {{FOCUS_X-200, FOCUS_Y*1.5f, CELEB_Z-200.f}},
      .target = (T3DVec3) {{FOCUS_X, FOCUS_Y*1.5f, CELEB_Z}},
      .travel_time = (CELEB_Z-RACE_END_Z)/LAKE_WALK_SPEED,
    },
    {.type = ACTION_END},
  },
  {
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*0.f, 0, CELEB_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_PI, .speed = T3D_PI},
    {.type = ACTION_START_ANIM, .anim = DANCE},
    {.type = ACTION_END},
  },
  {
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*1.f, 0, CELEB_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_PI, .speed = T3D_PI},
    {.type = ACTION_START_ANIM, .anim = DANCE},
    {.type = ACTION_END},
  },
  {
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*2.f, 0, CELEB_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_PI, .speed = T3D_PI},
    {.type = ACTION_START_ANIM, .anim = DANCE},
    {.type = ACTION_END},
  },
  {
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*3.f, 0, CELEB_Z}},
      .walk_speed = LAKE_WALK_SPEED,
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_PI, .speed = T3D_PI},
    {.type = ACTION_START_ANIM, .anim = DANCE},
    {.type = ACTION_END},
  },
};
static void lake_outro_dynamic_loop(float delta_time) {
  static struct script_state script_states[5];
  static size_t num_active_scripts;
  if (!lake_stage_inited[LAKE_OUTRO]) {
    lake_stage_inited[LAKE_OUTRO] = true;
    num_active_scripts = 1;

    script_states[0] = (struct script_state)
      {.character = NULL, .action = outro_actions[0], .time = 0.f};

    for (size_t i = 0; i < 4; i++) {
      if (!(winners_mask & (1<<i))) {
        continue;
      }
      script_states[num_active_scripts++] = (struct script_state)
        {.character = &players[i], .action = outro_actions[i+1], .time = 0.f};
    }
    delta_time = 0.f;
    wav64_play(&sfx_stop, MINIGAME_CHANNEL);
  }

  bool done = true;
  for (size_t i = 0; i < num_active_scripts; i++) {
    if (!script_update(&script_states[i], delta_time)) {
      done = false;
    }
  }

  if (done) {
    lake_stage++;
  }
}

void lake_end_fixed_loop(float delta_time) {
  static float winner_sfx_time_left;
  if (lake_stage_inited[LAKE_END]) {
    if (winner_sfx_time_left >= EPS) {
      winner_sfx_time_left -= delta_time;
      if (winner_sfx_time_left < EPS) {
        xm64player_set_vol(&music, 1.f);
      }
    }
    return;
  }

  lake_stage_inited[LAKE_END] = true;
  winner_sfx_time_left = 2.3f;

  size_t num_winners = 0;
  size_t o = 0;
  for (size_t i = 0; i < 4; i++) {
    if (!(winners_mask & (1<<i))) {
      continue;
    }
    o += sprintf(banner_str+o,
        "%s%s",
        num_winners? (num_winners == 3? "\n" : " ") : "",
        PLAYER_TITLES[i]);
    num_winners++;
  }
  if (num_winners) {
    sprintf(banner_str+o, "%s%s%s",
        num_winners < 4? "\n" : " ",
        SW_BANNER_S,
        num_winners > 1? "WIN" : "WINS");
  }
  else {
    strcpy(banner_str, "DRAW");
  }
  xm64player_set_vol(&music, .5f);
  wav64_play(&sfx_winner, MINIGAME_CHANNEL);
}

void lake_fade_out_fixed_loop(float delta_time) {
  if (!lake_stage_inited[LAKE_FADE_OUT]) {
    lake_stage_inited[LAKE_FADE_OUT] = true;
    return;
  }

  fade += delta_time / FADE_TIME;
  if (1.5f - fade < EPS) {
    minigame_end();
  }
}

bool lake_fixed_loop(float delta_time) {
  switch (lake_stage) {
    case LAKE_END:
      lake_end_fixed_loop(delta_time);
      break;

    case LAKE_FADE_OUT:
      lake_fade_out_fixed_loop(delta_time);
      break;
  }

  particle_source_iterate(&snow_particle_source, delta_time);

  for (size_t i = 0; i < NUM_SPLASH_SOURCES; i++) {
    particle_source_iterate(&splash_sources[i], delta_time);
  }

  for (size_t i = 0; i < 4; i++) {
    if (players[i].out) {
      continue;
    }

    float expected_height = get_ground_height(players[i].pos.v[2], &ground);
    players[i].pos.v[1] -= delta_time * GRAVITY;
    players[i].pos.v[1] = expected_height > players[i].pos.v[1]?
      expected_height : players[i].pos.v[1];

    if (players[i].temperature >= EPS) {
      particle_source_iterate(&steam_sources[i], delta_time);
      steam_sources[i].max_particles = ceilf(
          (float) NUM_STEAM_PARTICLES * players[i].temperature);
    }
    else {
      steam_sources[i].render = false;
      steam_sources[i].paused = true;
    }
  }

  return false;
}

void lake_cleanup() {
  sprite_free(tail_sprite);
  sprite_free(balloon_sprite);
  sprite_free(penalty_sprite);
  for (size_t i = 0; i < NUM_BUTTONS; i++) {
    sprite_free(buttons[i].sprite);
  }
  for (size_t i = 0; i < NUM_SPLASH_SOURCES; i++) {
    wav64_close(&sfx_splash[i]);
    particle_source_free(&splash_sources[i]);
  }
  particle_source_free(&snow_particle_source);
  for (size_t i = 0; i < 4; i++) {
    entity_free(&shadows[i]);
    particle_source_free(&steam_sources[i]);
  }
  t3d_model_free(shadow_model);
  entity_free(&map);
  t3d_model_free(map_model);
}
