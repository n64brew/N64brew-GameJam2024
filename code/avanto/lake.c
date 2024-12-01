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
#define LAKE_WALK_SPEED 200.f

enum lake_stages {
  LAKE_INTRO,
  NUM_LAKE_STAGES,
};

extern T3DViewport viewport;
extern struct character players[];

static struct entity map;
static T3DModel *map_model;
static T3DObject *water_object;
static struct camera cam = {
  .target = (T3DVec3) {{FOCUS_X, FOCUS_Y, 0.f}},
  .pos = (T3DVec3) {{CAMERA_X, CAMERA_Y, 0.f}},
};
static struct ground ground = {
  .num_changes = 6,
  .changes = {
    {-INFINITY, 0.f, false},
    {450.f, 10.f, false},
    {970.f, -64.f, false},
    {RACE_START_Z, -64.f*2.5f, false},
    {RACE_END_Z, -64.f*2.5f, true},
    {83.4f*64.f, 0.f, false},
  },
};
static bool lake_stage_inited[NUM_LAKE_STAGES];
static size_t lake_stage;
static wav64_t sfx_splash;

static void update_cam(float z) {
  cam.target.v[2] = z;
  cam.pos.v[2] = z;
  cam.pos.v[0] = CAMERA_X;
  cam.target.v[0] = FOCUS_X;
  /*
  debugf("T %f %f %f; P %f %f %f\n",
      cam.target.v[0],
      cam.target.v[1],
      cam.target.v[2],
      cam.pos.v[0],
      cam.pos.v[1],
      cam.pos.v[2]);
      */
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

void lake_init() {
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

  t3d_viewport_set_projection(&viewport, FOV, 50, 5000);
  update_cam(PLAYER_STARTING_Z);

  static T3DVec3 light_dir[] = {
    (T3DVec3) {{1.f, 1.f, 1.f}},
    (T3DVec3) {{-1.f, 1.f, -1.f}},
  };
  static uint8_t light_color[] = {0xff, 0xff, 0xff, 0xff};
  //static uint8_t ambient_color[] = {0x40, 0x9c, 0xff, 0xff};
  //static uint8_t ambient_color[] = {0x7b, 0xb1, 0xfc, 0xff};
  static uint8_t ambient_color[] = {0x64, 0x9c, 0xff, 0xff};
  t3d_vec3_norm(&light_dir[0]);
  t3d_vec3_norm(&light_dir[1]);
  t3d_light_set_directional(0, light_color, &light_dir[0]);
  t3d_light_set_directional(1, light_color, &light_dir[1]);
  t3d_light_set_ambient(ambient_color);
  t3d_light_set_count(2);

  for (size_t i = 0; i < 4; i++) {
    players[i].visible = !players[i].out;
    players[i].rotation = 0.f;
    players[i].pos =
      (T3DVec3) {{PLAYER_MIN_X+PLAYER_DISTANCE*i, 0.f, PLAYER_STARTING_Z}};
    t3d_anim_attach(&players[i].s.anims[WALK], &players[i].s.skeleton);
    players[i].current_anim = WALK;
    players[i].scale = 1.f;
  }

  wav64_open(&sfx_splash, "rom:/avanto/splash.wav64");

  for (size_t i = 0; i < NUM_LAKE_STAGES; i++) {
    lake_stage_inited[i] = false;
  }
  lake_stage = LAKE_INTRO;
}

void lake_dynamic_loop_pre(float delta_time) {
  joypad_buttons_t pressed = joypad_get_buttons_pressed(0);
  joypad_buttons_t held = joypad_get_buttons_held(0);
  /*
  if (pressed.d_up) {
    cam.pos.v[1] += 10.f;
  }
  if (pressed.d_down) {
    cam.pos.v[1] -= 10.f;
  }
  if (pressed.d_left) {
    cam.pos.v[0] -= 10.f;
  }
  if (pressed.d_right) {
    cam.pos.v[0] += 10.f;
  }
  */
  bool mudou = false;
  static T3DVec3 light = (T3DVec3) {{1.f, 2.f, 1.f}};
  static T3DVec3 light2;
  static int qual = 0;
  if (pressed.d_up) {
    light.v[qual]++;
    mudou = true;
  }
  if (pressed.d_down) {
    light.v[qual]--;
    mudou = true;
  }
  if (pressed.z) {
    qual = (qual+1) % 3;
    mudou = true;
  }

  if (mudou) {
    light2.v[0] = light.v[0];
    light2.v[1] = light.v[1];
    light2.v[2] = light.v[2];
    t3d_vec3_norm(&light2);
    static uint8_t light_color[] = {0xff, 0xff, 0xff, 0xff};
    t3d_light_set_directional(0, light_color, &light2);
    for (int i = 0; i < 3; i++) {
      if (qual == i) {
        debugf("(");
      }
      debugf("%.f ", light.v[i]);
      if (qual == i) {
        debugf(")");
      }
    }
    debugf("\n");
  }

  mudou = false;
  float d = held.l? 1.f : 50.f;
  if (pressed.c_right) {
    players[0].pos.v[2] += d;
    players[1].pos.v[2] += d;
    players[2].pos.v[2] += d;
    players[3].pos.v[2] += d;
    mudou = true;
  }
  if (pressed.c_left) {
    players[0].pos.v[2] -= d;
    players[1].pos.v[2] -= d;
    players[2].pos.v[2] -= d;
    players[3].pos.v[2] -= d;
    mudou = true;
  }
  if (pressed.c_up) {
    players[0].pos.v[0] += d;
    players[1].pos.v[0] += d;
    players[2].pos.v[0] += d;
    players[3].pos.v[0] += d;
    mudou = true;
  }
  if (pressed.c_down) {
    players[0].pos.v[0] -= d;
    players[1].pos.v[0] -= d;
    players[2].pos.v[0] -= d;
    players[3].pos.v[0] -= d;
    mudou = true;
  }
  if (mudou) {
    update_cam((players[1].pos.v[2]+players[2].pos.v[2])/2.f);
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
    if (players[i].current_anim != -1) {
      t3d_anim_update(&players[i].s.anims[players[i].current_anim],
          delta_time);
      t3d_skeleton_update(&players[i].s.skeleton);
    }
    if (players[i].visible) {
      t3d_mat4fp_from_srt_euler(players[i].e.transform,
        (float[3]) {players[i].scale, players[i].scale, players[i].scale},
        (float[3]) {0, players[i].rotation, 0},
        players[i].pos.v);
      rspq_block_run(players[i].e.display_block);
    }
  }

  draw_hud();
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
    {.type = ACTION_WAIT, .time = 1.f},
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
    {.type = ACTION_PLAY_SFX, .sfx = &sfx_splash},
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
    {.type = ACTION_ANIM_SET_PLAYING, .playing = false},
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
    {.type = ACTION_ANIM_SET_PLAYING, .playing = false},
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
    {.type = ACTION_ANIM_SET_PLAYING, .playing = false},
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
    {.type = ACTION_ANIM_SET_PLAYING, .playing = false},
    {.type = ACTION_END},
  },
};
static void lake_cam_intro_fixed_loop(float delta_time) {
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

  bool done = true;
  for (size_t i = 0; i < 5; i++) {
    if ((!script_states[i].character || !script_states[i].character->out)
        && !script_update(&script_states[i], delta_time)) {
      done = false;
    }
  }

  if (done) {
    lake_stage++;
  }
}

bool lake_fixed_loop(float delta_time) {
  switch (lake_stage) {
    case LAKE_INTRO:
      lake_cam_intro_fixed_loop(delta_time);
      break;
  }

  for (size_t i = 0; i < 4; i++) {
    float expected_height = get_ground_height(players[i].pos.v[2], &ground);
    players[i].pos.v[1] -= delta_time * GRAVITY;
    players[i].pos.v[1] = expected_height > players[i].pos.v[1]?
      expected_height : players[i].pos.v[1];
  }

  return false;
}

void lake_cleanup() {
  wav64_close(&sfx_splash);
  entity_free(&map);
  t3d_model_free(map_model);
}
