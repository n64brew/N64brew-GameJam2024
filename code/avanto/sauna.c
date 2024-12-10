#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/tpx.h>

#include "common.h"
#include "sauna.h"

#define LOYLY_SOUND_DELAY .9f
#define LOYLY_DELAY 1.5f
#define LOYLY_LENGTH 4.f
#define LOYLY_SCREEN_MAX_ALPHA 200.f
#define LOYLY_SCREEN_MIN_ALPHA 16.f
#define SAUNA_LEN 60.f
#define BASE_HEAT (.2f / 60.f)
#define LOYLY_CHANNEL 10
#define DOOR_CHANNEL 11
#define MINIGAME_CHANNEL 31
#define SCALE 2.5f
#define SAUNA_GRAVITY (GRAVITY*SCALE)
#define SAUNA_WALK_SPEED 100.f
#define KIUAS_MAX_PARTICLES 128

enum sauna_stages {
  SAUNA_INTRO,
  SAUNA_WALK_IN,
  SAUNA_COUNTDOWN,
  SAUNA_GAME,
  SAUNA_UNBEND,
  SAUNA_WALK_OUT,
  SAUNA_DONE,
  SAUNA_FADE_OUT,
  NUM_SAUNA_STAGES,
};

enum ukko_anims {
  THROW,
  NUM_UKKO_ANIMS,
};

struct sauna_ai {
  size_t pid;
  void (*handler)(struct sauna_ai *, joypad_buttons_t *t);
  float target;
};

extern struct character players[];
extern surface_t *z_buffer;
extern T3DViewport viewport;
extern struct camera cam;
extern xm64player_t music;
extern struct rdpq_textparms_s banner_params;
extern struct rdpq_textparms_s timer_params;
extern wav64_t sfx_countdown;
extern wav64_t sfx_start;
extern wav64_t sfx_stop;
extern wav64_t sfx_winner;

static T3DModel *ukko_model;
static struct character ukko;

static bool loyly_sound_queued;
static bool loyly_queued;
static float loyly_strength;
static wav64_t sfx_loyly;
static wav64_t sfx_door;
static int sauna_stage;
static bool sauna_stage_inited[NUM_SAUNA_STAGES];
static float upness[4];
static struct particle_source kiuas_particle_source;

static char banner_str[16];
static float banner_time;
static float time_left;
static float min_time_before_exiting;
static struct sauna_ai ais[4];

static T3DModel *cube_model;
static struct entity invisicubes[2];
static sprite_t *kiuas;

static bool end_when_over;
static float fade;

static void ai_coward(struct sauna_ai *ai, joypad_buttons_t *held) {
  held->raw = 0;
}

static void ai_bold(struct sauna_ai *ai, joypad_buttons_t *held) {
  held->raw = 0;
  held->z = 1;
}

static void ai_smart(struct sauna_ai *ai, joypad_buttons_t *held) {
  float elapsed = (SAUNA_LEN - time_left) / SAUNA_LEN;
  float expected = elapsed * ai->target;

  held->raw = 0;
  if (players[ai->pid].temperature > expected) {
    held->z = 1;
  }
}

static void ai_init(struct sauna_ai *ai, size_t pid, AiDiff diff) {
  ai->pid = pid;
  if (diff == DIFF_EASY) {
    ai->handler = rand() & 1? ai_coward : ai_bold;
  }
  else {
    ai->target = diff == DIFF_MEDIUM?
      rand_float(.6f, .75f) : rand_float(.85f, .95f);
    ai->handler = ai_smart;
  }
}

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

  uint8_t no_light[] = {0x0, 0x0, 0x0};
  t3d_light_set_ambient(no_light);
}
static struct scene sauna_scene = {
  .bg_path = "rom:/avanto/sauna.sprite",
  .fov = T3D_DEG_TO_RAD(78.f),
  .starting_cam = {
    .pos = (T3DVec3) {{0, 100, 0}},
    .target = (T3DVec3) {{125, 125, 155}}
  },
  .do_light = sauna_do_light,
  .ground = {
    .num_changes = 3,
    .changes = {
      {-INFINITY, -30, false},
      {140, 41, false},
      {230, 110, false}
    }
  },
};


void sauna_init() {
  ukko_model = t3d_model_load("rom:/avanto/ukko.t3dm");
  ukko.rotation = T3D_DEG_TO_RAD(90.f);
  ukko.scale = 3.f;
  ukko.pos = (T3DVec3) {{400.f, 41.f, 150.f}};
  skeleton_init(&ukko.s, ukko_model, NUM_UKKO_ANIMS);
  ukko.s.anims[THROW] = t3d_anim_create(ukko_model, "throw");
  entity_init(&ukko.e,
      ukko_model,
      &(T3DVec3) {{ukko.scale, ukko.scale, ukko.scale}},
      &(T3DVec3) {{0.f, ukko.rotation, 0.f}},
      &ukko.pos,
      &ukko.s.skeleton,
      NULL);
  t3d_anim_attach(&ukko.s.anims[THROW], &ukko.s.skeleton);
  t3d_anim_update(&ukko.s.anims[THROW], 0);
  ukko.current_anim = THROW;
  ukko.visible = true;
  t3d_skeleton_update(&ukko.s.skeleton);
  t3d_anim_set_looping(&ukko.s.anims[THROW], false);
  t3d_anim_set_playing(&ukko.s.anims[THROW], false);
  loyly_sound_queued = false;
  loyly_queued = false;
  loyly_strength = 0.f;
  banner_time = 0.f;

  cube_model = t3d_model_load("rom:/avanto/unit-cube.t3dm");
  entity_init(&invisicubes[0],
      cube_model,
      &(T3DVec3) {{4.f*2.f, .55f*2.f, .6f*2.f}},
      &(T3DVec3) {{0.f, 0.f, 0.f}},
      &(T3DVec3) {{-40.f, 41.f, 330.f}},
      NULL,
      NULL);
  entity_init(&invisicubes[1],
      cube_model,
      &(T3DVec3) {{.6f*2.f, .55f*2, 4.f*2.f}},
      &(T3DVec3) {{0.f, 0.f, 0.f}},
      &(T3DVec3) {{400.f, 41.f, 260.f}},
      NULL,
      NULL);
  kiuas = sprite_load("rom:/avanto/kiuas.sprite");

  sauna_scene.bg = sprite_load(sauna_scene.bg_path);
  const struct camera *cam = &sauna_scene.starting_cam;
  t3d_viewport_set_projection(&viewport, sauna_scene.fov, 10, 400);
  t3d_viewport_look_at(&viewport,
      &cam->pos,
      &cam->target,
      &(T3DVec3) {{0, 1, 0}});

  wav64_open(&sfx_loyly, "rom:/avanto/loyly.wav64");
  wav64_open(&sfx_door, "rom:/avanto/door.wav64");

  particle_source_init(&kiuas_particle_source, KIUAS_MAX_PARTICLES, STEAM);
  kiuas_particle_source.pos = (T3DVec3) {{100.f, 100.f+128.f, 0.f}};
  kiuas_particle_source.scale = (T3DVec3) {{1.f, 1.f, 1.f}};
  kiuas_particle_source.rot = (T3DVec3) {{0.f, 0.f, 0.f}};
  kiuas_particle_source.render = false;
  kiuas_particle_source.x_range = 25;
  kiuas_particle_source.z_range = 30;
  kiuas_particle_source.height = 100;
  kiuas_particle_source.particle_size = 4;
  kiuas_particle_source.time_to_rise = 1.f;
  kiuas_particle_source.movement_amplitude = 5.f;
  kiuas_particle_source.max_particles = KIUAS_MAX_PARTICLES;
  kiuas_particle_source.paused = true;
  particle_source_update_transform(&kiuas_particle_source);

  min_time_before_exiting = 3.f;

  memset(ais, 0, sizeof(ais));
  for (size_t i = 0; i < 4; i++) {
    players[i].scale = SCALE;
    if (i >= core_get_playercount()) {
      ai_init(&ais[i], i, core_get_aidifficulty());
    }
  }

  end_when_over = false;
  fade = 1.f;
  sauna_stage = SAUNA_INTRO;

  for (size_t i = 0; i < NUM_SAUNA_STAGES; i++) {
    sauna_stage_inited[i] = false;
  }
}

static void sauna_intro_fixed_loop(float delta_time) {
  static float delay;
  static bool thrown;
  if (!sauna_stage_inited[SAUNA_INTRO]) {
    sauna_stage_inited[SAUNA_INTRO] = true;
    delay = 1.f;
    thrown = false;
  }

  if (fade >= EPS) {
    fade -= delta_time / FADE_TIME;
    return;
  }

  if (delay > 0) {
      delay -= delta_time;
      if (delay > 0) {
        return;
      }
  }

  if (!thrown) {
    thrown = true;
    t3d_anim_set_playing(&ukko.s.anims[THROW], true);
    loyly_sound_queued = true;
    loyly_queued = true;
    delay = 5.f;
    return;
  }

  sauna_stage++;
}

struct script_action walk_in_actions[][14] = {
  {
    {.type = ACTION_PLAY_SFX, .sfx = &sfx_door, .channel = DOOR_CHANNEL},
    {.type = ACTION_WAIT, .time = 2.f},
    {.type = ACTION_START_XM64, .xm64 = &music, .first_channel = 0},
    {.type = ACTION_WARP_TO, .pos = (T3DVec3) {{-100, 0, 110}}},
    {.type = ACTION_SET_VISIBILITY, .visibility = true},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = 0.f, .speed = T3D_PI},
    {.type = ACTION_CLIMB_TO, .pos = (T3DVec3) {{100, 0, 200}}},
    {.type = ACTION_START_ANIM, .anim = WALK},
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(-90.f), .speed = -T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{300, 0, 210}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(180.f), .speed = -T3D_PI},
    {.type = ACTION_DO_WHOLE_ANIM, .anim = SIT},
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT, .time = 2.f*2.f},
    {.type = ACTION_WARP_TO, .pos = (T3DVec3) {{-100, 0, 110}}},
    {.type = ACTION_SET_VISIBILITY, .visibility = true},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = 0.f, .speed = T3D_PI},
    {.type = ACTION_CLIMB_TO, .pos = (T3DVec3) {{100, 0, 200}}},
    {.type = ACTION_START_ANIM, .anim = WALK},
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(-90.f), .speed = -T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{210, 0, 210}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(180.f), .speed = -T3D_PI},
    {.type = ACTION_DO_WHOLE_ANIM, .anim = SIT},
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT, .time = 2.f*4.f},
    {.type = ACTION_WARP_TO, .pos = (T3DVec3) {{-100, 0, 110}}},
    {.type = ACTION_SET_VISIBILITY, .visibility = true},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = 0.f, .speed = T3D_PI},
    {.type = ACTION_CLIMB_TO, .pos = (T3DVec3) {{100, 0, 200}}},
    {.type = ACTION_START_ANIM, .anim = WALK},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{120, 0, 210}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(180.f), .speed = -T3D_PI},
    {.type = ACTION_DO_WHOLE_ANIM, .anim = SIT},
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT, .time = 2.f*3.f},
    {.type = ACTION_WARP_TO, .pos = (T3DVec3) {{-100, 0, 110}}},
    {.type = ACTION_SET_VISIBILITY, .visibility = true},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = 0.f, .speed = T3D_PI},
    {.type = ACTION_CLIMB_TO, .pos = (T3DVec3) {{100, 0, 200}}},
    {.type = ACTION_START_ANIM, .anim = WALK},
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(90.f), .speed = T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{30, 0, 210}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(180.f), .speed = T3D_PI},
    {.type = ACTION_DO_WHOLE_ANIM, .anim = SIT},
    {.type = ACTION_END},
  },
};
static void sauna_walk_in_fixed_loop(float delta_time) {
  static struct script_state script_states[4];
  if (!sauna_stage_inited[SAUNA_WALK_IN]) {
    sauna_stage_inited[SAUNA_WALK_IN] = true;
    script_states[0] = (struct script_state)
      {.character = &players[0], .action = walk_in_actions[0], .time = 0.f};
    script_states[1] = (struct script_state)
      {.character = &players[1], .action = walk_in_actions[1], .time = 0.f};
    script_states[2] = (struct script_state)
      {.character = &players[2], .action = walk_in_actions[2], .time = 0.f};
    script_states[3] = (struct script_state)
      {.character = &players[3], .action = walk_in_actions[3], .time = 0.f};
  }

  bool done = true;
  for (size_t i = 0; i < 4; i++) {
    if (!script_update(&script_states[i], delta_time)) {
      done = false;
    }
  }

  if (done) {
    sauna_stage++;
  }
}

static void sauna_countdown_fixed_loop(float delta_time) {
  static float total_time;
  static int count;
  static float next_step;
  if (!sauna_stage_inited[SAUNA_COUNTDOWN]) {
    sauna_stage_inited[SAUNA_COUNTDOWN] = true;
    total_time = 0.f;
    count = 3;
    next_step = 0.f;
  }

  if (total_time == 0.f) {
    xm64player_set_vol(&music, .5f);
  }

  if (total_time >= next_step) {
    if (count) {
      sprintf(banner_str, "%d", count);
      count--;
      next_step += 1.f;
      wav64_play(&sfx_countdown, MINIGAME_CHANNEL);
    }
    else {
      strcpy(banner_str, "START");
      sauna_stage++;
      wav64_play(&sfx_start, MINIGAME_CHANNEL);
      xm64player_set_vol(&music, 1.f);
    }
    banner_time = 1.f;
  }

  total_time += delta_time;
}

static void sauna_game_fixed_loop(float delta_time) {
  static float next_throw;
  if (!sauna_stage_inited[SAUNA_GAME]) {
    sauna_stage_inited[SAUNA_GAME] = true;
    time_left = SAUNA_LEN;
    next_throw = rand_float(0.f, LOYLY_LENGTH);
    delta_time = 0.f;

    for (size_t i = 0; i < 4; i++) {
      T3DAnim *anim = &players[i].s.anims[UNBEND];
      players[i].current_anim = UNBEND;
      t3d_anim_attach(anim, &players[i].s.skeleton);
      t3d_anim_set_playing(anim, false);
    }
    return;
  }

  time_left -= delta_time;
  next_throw -= delta_time;

  if (next_throw < EPS) {
    next_throw = rand_float(LOYLY_LENGTH * 1.05f, LOYLY_LENGTH * 2.5f);

    if (time_left < next_throw + LOYLY_LENGTH + LOYLY_DELAY) {
      next_throw = INFINITY;
    }

    t3d_anim_set_playing(&ukko.s.anims[THROW], true);
    loyly_sound_queued = true;
    loyly_queued = true;
  }

  bool all_out = true;
  for (size_t i = 0; i < 4; i++) {
    if (players[i].out) {
      continue;
    }
    all_out = false;

    float loyly_effect = loyly_strength > .5f? 1.f : loyly_strength/.5f;
    float delta_temp = BASE_HEAT
      * (1.f + upness[i]*4.f)
      * (1.f + loyly_effect*4.f)
      * delta_time;
    players[i].temperature += delta_temp;
    if (players[i].temperature > 1.f) {
      players[i].out = true;
    }
  }

  if (time_left < EPS || all_out) {
    wav64_play(&sfx_stop, MINIGAME_CHANNEL);
    sauna_stage++;
  }
}

struct script_action walk_out_actions[][9] = {
  {
    {.type = ACTION_WAIT, .time = 3.f},
    {.type = ACTION_DO_WHOLE_ANIM, .anim = STAND_UP},
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(90.f), .speed = -T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{100, 0, 200}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(-180.f), .speed = T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(90.f), .speed = -T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{-100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT, .time = 2.f},
    {.type = ACTION_DO_WHOLE_ANIM, .anim = STAND_UP},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{210, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(90.f), .speed = -T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{-100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_WAIT, .time = 1.f},
    {.type = ACTION_DO_WHOLE_ANIM, .anim = STAND_UP},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{120, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(90.f), .speed = -T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{-100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_END},
  },
  {
    {.type = ACTION_DO_WHOLE_ANIM, .anim = STAND_UP},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{30, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_ROTATE_TO, .rot = T3D_DEG_TO_RAD(90.f), .speed = -T3D_PI},
    {
      .type = ACTION_WALK_TO,
      .pos = (T3DVec3) {{-100, 0, 110}},
      .walk_speed = SAUNA_WALK_SPEED
    },
    {.type = ACTION_END},
  },
};
static void sauna_walk_out_fixed_loop(float delta_time) {
  static struct script_state script_states[4];
  if (!sauna_stage_inited[SAUNA_WALK_OUT]) {
    sauna_stage_inited[SAUNA_WALK_OUT] = true;
    script_states[0] = (struct script_state)
      {.character = &players[0], .action = walk_out_actions[0], .time = 0.f};
    script_states[1] = (struct script_state)
      {.character = &players[1], .action = walk_out_actions[1], .time = 0.f};
    script_states[2] = (struct script_state)
      {.character = &players[2], .action = walk_out_actions[2], .time = 0.f};
    script_states[3] = (struct script_state)
      {.character = &players[3], .action = walk_out_actions[3], .time = 0.f};
  }

  bool done = true;
  for (size_t i = 0; i < 4; i++) {
    if (!players[i].out && !script_update(&script_states[i], delta_time)) {
      done = false;
    }
  }

  if (done) {
    sauna_stage++;
  }
}

static void sauna_done_fixed_loop(float delta_time) {
  if (sauna_stage_inited[SAUNA_DONE]) {
    return;
  }

  sauna_stage_inited[SAUNA_DONE] = true;

  bool draw = true;
  for (size_t i = 0; i < 4; i++) {
    if (!players[i].out) {
      draw = false;
      break;
    }
  }

  if (!draw) {
    sauna_stage++;
    return;
  }

  strcpy(banner_str, "DRAW");
  banner_time = INFINITY;
  end_when_over = true;
  xm64player_set_vol(&music, .5f);
  wav64_play(&sfx_winner, MINIGAME_CHANNEL);
}

static bool sauna_fade_out_fixed_loop(float delta_time) {
  if (!sauna_stage_inited[SAUNA_FADE_OUT]) {
    sauna_stage_inited[SAUNA_FADE_OUT] = true;
    return false;
  }

  fade += delta_time / FADE_TIME;
  if (1.5f - fade < EPS) {
    if (end_when_over) {
      minigame_end();
    }
    else {
      return true;
    }
  }

  return false;
}

static void sauna_change_anim_and_play_from(struct character *character,
    size_t anim_id,
    float normal_time) {
  T3DAnim *anim = &character->s.anims[anim_id];
  float time = anim->animRef->duration * normal_time;

  character->current_anim = anim_id;
  t3d_anim_attach(anim, &character->s.skeleton);
  t3d_anim_set_time(anim, time);
  t3d_anim_set_playing(anim, true);
}

static float sauna_get_anim_normal_time(const T3DAnim *anim) {
  if (!anim->isPlaying) {
    return 1.f;
  }

  return anim->time / anim->animRef->duration;
}

static void sauna_unbend_fixed_loop(float delta_time) {
  static float delay;
  if (!sauna_stage_inited[SAUNA_UNBEND]) {
    sauna_stage_inited[SAUNA_UNBEND] = true;
    delay = 2.f;
    return;
  }

  delay -= delta_time;
  if (delay < EPS) {
    sauna_stage++;
  }
}

bool sauna_fixed_loop(float delta_time) {
  switch (sauna_stage) {
    case SAUNA_INTRO:
      sauna_intro_fixed_loop(delta_time);
      break;

    case SAUNA_WALK_IN:
      sauna_walk_in_fixed_loop(delta_time);
      break;

    case SAUNA_COUNTDOWN:
      sauna_countdown_fixed_loop(delta_time);
      break;

    case SAUNA_GAME:
      sauna_game_fixed_loop(delta_time);
      break;

    case SAUNA_UNBEND:
      sauna_unbend_fixed_loop(delta_time);
      break;

    case SAUNA_WALK_OUT:
      sauna_walk_out_fixed_loop(delta_time);
      break;

    case SAUNA_DONE:
      sauna_done_fixed_loop(delta_time);
      break;

    case SAUNA_FADE_OUT:
      if (sauna_fade_out_fixed_loop(delta_time)) {
        return true;
      }
      break;
  }

  for (size_t i = 0; i < 4; i++) {
    if (players[i].current_anim != CLIMB) {
      float expected_height = get_ground_height(players[i].pos.v[2],
          &sauna_scene.ground);
      players[i].pos.v[1] -= delta_time * SAUNA_GRAVITY;
      players[i].pos.v[1] = expected_height > players[i].pos.v[1]?
        expected_height : players[i].pos.v[1];
    }
  }

  particle_source_iterate(&kiuas_particle_source, delta_time);

  return false;
}

void sauna_dynamic_loop_pre(float delta_time) {
  if (loyly_strength >= EPS) {
    loyly_strength -= delta_time/LOYLY_LENGTH;
    if (loyly_strength < EPS) {
      loyly_strength = 0.f;
      kiuas_particle_source.render = false;
      kiuas_particle_source.paused = true;
    }
  }
  if (loyly_sound_queued
      && ukko.s.anims[THROW].time + delta_time >= LOYLY_SOUND_DELAY) {
    wav64_play(&sfx_loyly, LOYLY_CHANNEL);
    loyly_sound_queued = false;
  }
  if (loyly_queued && ukko.s.anims[THROW].time + delta_time >= LOYLY_DELAY) {
    loyly_strength = 1.f;
    kiuas_particle_source.render = true;
    kiuas_particle_source.paused = false;
    particle_source_reset_steam(&kiuas_particle_source);
    loyly_queued = false;
  }
  int max_particles = (int) ((float) KIUAS_MAX_PARTICLES * loyly_strength);
  max_particles = max_particles < 0? 0 : max_particles;
  max_particles = max_particles > KIUAS_MAX_PARTICLES?
    KIUAS_MAX_PARTICLES : max_particles;
  kiuas_particle_source.max_particles = max_particles;
  if (kiuas_particle_source.max_particles > KIUAS_MAX_PARTICLES) {
    kiuas_particle_source.max_particles = KIUAS_MAX_PARTICLES;
  }
  kiuas_particle_source.time_to_rise = 2.f-loyly_strength;

  for (size_t i = 0; i < 4; i++) {
    if (!players[i].out) {
      continue;
    }

    if (players[i].current_anim == BEND) {
      sauna_change_anim_and_play_from(&players[i],
          UNBEND,
          1.f - sauna_get_anim_normal_time(&players[i].s.anims[BEND]));
    }
    else if (players[i].current_anim == UNBEND
        && !players[i].s.anims[UNBEND].isPlaying) {
      sauna_change_anim_and_play_from(&players[i], PASS_OUT, 0.f);
    }
  }
}

void sauna_dynamic_loop_render(float delta_time) {
  t3d_screen_clear_depth();

  // Cubes, rendered behind the BG to set the depth
  rspq_block_run(invisicubes[0].display_block);
  rspq_block_run(invisicubes[1].display_block);

  // Kiuas mask, also only sets the depth
  rdpq_sync_pipe();
  rdpq_mode_push();
  rdpq_set_mode_standard();
  rdpq_mode_zbuf(false, true);
  rdpq_mode_zoverride(true, 0.f, 0);
  rdpq_mode_alphacompare(1);
  rdpq_sprite_blit(kiuas, 0, 240-kiuas->height, NULL);
  rdpq_mode_pop();

  // BG
  rdpq_mode_push();
  rdpq_set_mode_copy(false);
  rdpq_sprite_blit(sauna_scene.bg, 0, 0, NULL);
  rdpq_mode_pop();

  sauna_scene.do_light();

  // Players
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

  // Ukko
  t3d_anim_update(&ukko.s.anims[ukko.current_anim], delta_time);
  t3d_skeleton_update(&ukko.s.skeleton);
  if (ukko.visible) {
    rspq_block_run(ukko.e.display_block);
  }

  // Particles
  if (kiuas_particle_source.render) {
    rdpq_sync_pipe();

    rdpq_mode_push();
    rdpq_set_mode_standard();
    rdpq_mode_zbuf(true, true);
    rdpq_mode_zoverride(true, 0, 0);
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    tpx_state_from_t3d();

    tpx_state_set_scale(1.f, 1.f);
    particle_source_draw(&kiuas_particle_source);

    rdpq_sync_pipe();

    rdpq_mode_pop();
  }

  rdpq_sync_pipe();

  if (loyly_strength >= EPS) {
    rdpq_mode_push();
    int screen_alpha =
      (int) ((LOYLY_SCREEN_MAX_ALPHA-LOYLY_SCREEN_MIN_ALPHA)*loyly_strength)
      + (int) LOYLY_SCREEN_MIN_ALPHA;
    rdpq_set_fog_color(RGBA32(0xff, 0xff, 0xff, screen_alpha));
    rdpq_set_prim_color(RGBA32(0xff, 0xff, 0xff, 0xff));
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
    rdpq_mode_zbuf(false, false);
    rdpq_fill_rectangle(0, 0, 320, 240);
    rdpq_mode_pop();
  }


  // HUD
  if (sauna_stage >= SAUNA_COUNTDOWN) {
    draw_hud();
  }

  // Time
  if (sauna_stage == SAUNA_GAME) {
    MITIGATE_FONT_BUG;
    rdpq_text_printf(&timer_params,
      FONT_TIMER,
      0,
      TIMER_Y,
      "%.0f",
      ceilf(time_left));
  }

  // Banner
  if (banner_time > EPS) {
    banner_time -= delta_time;
    MITIGATE_FONT_BUG;
    rdpq_text_print(&banner_params, FONT_BANNER, 0, 120, banner_str);
  }

  if (fade >= EPS) {
    draw_fade(fade);
  }
}

void sauna_dynamic_loop_post(float delta_time) {
  joypad_buttons_t held[4];
  joypad_buttons_t pressed[4];
  for (size_t i = 0; i < core_get_playercount(); i++) {
    held[i] = joypad_get_buttons_held(core_get_playercontroller(i));
    pressed[i] = joypad_get_buttons_pressed(core_get_playercontroller(i));
  }
  for (size_t i = core_get_playercount(); i < 4; i++) {
    pressed[i].raw = 0;
    if (sauna_stage == SAUNA_GAME) {
      ais[i].handler(&ais[i], &held[i]);
    }
    else {
      held[i].raw = 0;
    }
  }

  if (sauna_stage == SAUNA_DONE) {
    if (min_time_before_exiting >= EPS) {
      min_time_before_exiting -= delta_time;
    }
    else {
      for (size_t i = 0; i < 4; i++) {
        if (pressed[i].a || pressed[i].b) {
          sauna_stage++;
        }
      }
    }
  }

  if ((sauna_stage != SAUNA_GAME && sauna_stage != SAUNA_UNBEND)
      || !sauna_stage_inited[SAUNA_GAME]) {
    return;
  }

  for (size_t i = 0; i < 4; i++) {
    if (players[i].out) {
      continue;
    }
    if (sauna_stage == SAUNA_UNBEND) {
      held[i].raw = 0;
    }

    // If the player is not out, animation will always be BEND or UNBEND
    const T3DAnim *anim = &players[i].s.anims[players[i].current_anim];
    if (players[i].current_anim == BEND) {
      upness[i] = anim->isPlaying?
        1.f - anim->time/anim->animRef->duration : 0.f;
    }
    else {
      upness[i] = anim->isPlaying? anim->time/anim->animRef->duration : 1.f;
    }
    
    if (held[i].z && players[i].current_anim != BEND) {
      sauna_change_anim_and_play_from(&players[i],
          BEND,
          1.f - sauna_get_anim_normal_time(&players[i].s.anims[UNBEND]));
    }
    else if (!held[i].z && players[i].current_anim != UNBEND) {
      sauna_change_anim_and_play_from(&players[i],
          UNBEND,
          1.f - sauna_get_anim_normal_time(&players[i].s.anims[BEND]));
    }
  }
}

void sauna_cleanup() {
  rspq_wait();

  particle_source_free(&kiuas_particle_source);

  sprite_free(sauna_scene.bg);

  wav64_close(&sfx_loyly);
  wav64_close(&sfx_door);

  sprite_free(kiuas);
  entity_free(&invisicubes[0]);
  entity_free(&invisicubes[1]);
  t3d_model_free(cube_model);

  entity_free(&ukko.e);
  skeleton_free(&ukko.s);
  t3d_model_free(ukko_model);
}

