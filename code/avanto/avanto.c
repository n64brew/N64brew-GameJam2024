#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#include "common.h"
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
struct character players[4];
struct scene *current_scene;
rdpq_font_t *debug_font;
color_t player_colors[4];

xm64player_t music;
wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

static bool filter_player_hair_color(void *user_data, const T3DObject *obj) {
  color_t *color = (color_t *) user_data;
  if (!strcmp(obj->name, "hair")) {
    rdpq_set_prim_color(*color);
  }

  return true;
}

void minigame_init() {
  player_colors[0] = PLAYERCOLOR_1;
  player_colors[1] = PLAYERCOLOR_2;
  player_colors[2] = PLAYERCOLOR_3;
  player_colors[3] = PLAYERCOLOR_4;

  display_init(RESOLUTION_320x240,
      DEPTH_16_BPP,
      3,
      GAMMA_NONE,
      FILTERS_RESAMPLE_ANTIALIAS);
  z_buffer = display_get_zbuf();

  t3d_init((T3DInitParams){});
  viewport = t3d_viewport_create();

  player_model = t3d_model_load("rom:/avanto/guy.t3dm");
  T3DModelDrawConf player_draw_conf = {
    .userData = NULL,
    .tileCb = NULL,
    .filterCb = filter_player_hair_color,
    .dynTextureCb = NULL,
    .matrices = NULL,
  };
  for (size_t i = 0; i < 4; i++) {
    players[i].rotation = 0;
    skeleton_init(&players[i].s, player_model, 4);
    players[i].s.anims[WALK] = t3d_anim_create(player_model, "walking");
    players[i].s.anims[CLIMB] = t3d_anim_create(player_model, "climbing");
    t3d_anim_set_looping(&players[i].s.anims[CLIMB], false);
    players[i].s.anims[SIT] = t3d_anim_create(player_model, "sitting");
    t3d_anim_set_looping(&players[i].s.anims[SIT], false);
    players[i].s.anims[BEND] = t3d_anim_create(player_model, "bending");
    t3d_anim_set_looping(&players[i].s.anims[BEND], false);
    players[i].pos = (T3DVec3) {{0, 0, 0}};
    players[i].scale = 2.5f;
    players[i].current_anim = -1;
    players[i].visible = false;
    player_draw_conf.userData = &player_colors[i];
    entity_init(&players[i].e,
        player_model,
        &(T3DVec3) {{players[i].scale, players[i].scale, players[i].scale}},
        &(T3DVec3) {{0, players[i].rotation, 0}},
        &players[i].pos,
        &players[i].s.skeleton,
        player_draw_conf);
  }

  debug_font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
  rdpq_text_register_font(FONT_DEBUG, debug_font);

  xm64player_open(&music, "rom:/avanto/sj-polkka.xm64");

  wav64_open(&sfx_start, "rom:/core/Start.wav64");
  wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
  wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
  wav64_open(&sfx_winner, "rom:/core/Winner.wav64");

  for (int i = 0; i < NUM_SFX_CHANNELS; i++) {
    int channel = FIRST_SFX_CHANNEL + i;
    mixer_ch_set_vol(channel, 0.5f, 0.5f);
    mixer_ch_set_limits(channel, 0, 48000, 0);
  }

  sauna_init();
}

void minigame_fixedloop(float delta_time) {
  sauna_fixed_loop(delta_time);
}

void minigame_loop(float delta_time) {
  sauna_loop(delta_time);
}

void minigame_cleanup() {
  sauna_cleanup();
  wav64_close(&sfx_start);
  wav64_close(&sfx_countdown);
  wav64_close(&sfx_stop);
  wav64_close(&sfx_winner);

  xm64player_stop(&music);
  xm64player_close(&music);

  rdpq_text_unregister_font(FONT_DEBUG);
  rdpq_font_free(debug_font);

  for (size_t i = 0; i < 4; i++) {
    entity_free(&players[i].e);
    skeleton_free(&players[i].s);
  }
  t3d_model_free(player_model);

  t3d_destroy();
  display_close();
}
