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
#include "lake.h"

const MinigameDef minigame_def = {
  .gamename = "Sauna Rush",
  .developername = "Flávio Zavan",
  .description = "A relaxing winter day turns into a competition",
  .instructions = "Inside: Hold Z to duck and avoid the heat from the steam. "
    "Don't pass out!\nOutside: Press the buttons indicated on the screen. "
    "The heat from the sauna gives you a speed boost",
};

surface_t *z_buffer;
T3DViewport viewport;
struct camera cam;
T3DModel *player_model;
struct character players[4];
struct scene *current_scene;
rdpq_font_t *normal_font;
rdpq_font_t *timer_font;
rdpq_font_t *banner_font;
color_t player_colors[4];
const color_t skin_tones[] = {
  (color_t) {0xff, 0xf5, 0xdf, 0xff},
  (color_t) {0xff, 0xdd, 0xc4, 0xff},
  (color_t) {0xff, 0xd9, 0xae, 0xff},
  (color_t) {0xff, 0xcb, 0x93, 0xff},
  (color_t) {0xf6, 0xe2, 0xac, 0xff},
  (color_t) {0xff, 0xd8, 0xb9, 0xff},
  (color_t) {0xee, 0xc2, 0xa8, 0xff},
  (color_t) {0xdc, 0xb7, 0x8b, 0xff},
  (color_t) {0xf3, 0xb7, 0x8d, 0xff},
  (color_t) {0xe7, 0xad, 0x86, 0xff},
  (color_t) {0xe4, 0xab, 0x84, 0xff},
  (color_t) {0xcd, 0x9a, 0x77, 0xff},
  (color_t) {0xca, 0x95, 0x75, 0xff},
  (color_t) {0xd6, 0x8b, 0x62, 0xff},
  (color_t) {0xcc, 0x8a, 0x6f, 0xff},
  (color_t) {0xc4, 0x8b, 0x69, 0xff},
  (color_t) {0xc6, 0x7f, 0x5a, 0xff},
  (color_t) {0xb4, 0x71, 0x51, 0xff},
  (color_t) {0xa4, 0x67, 0x4a, 0xff},
  (color_t) {0x93, 0x5c, 0x41, 0xff},
  (color_t) {0x86, 0x55, 0x3c, 0xff},
  (color_t) {0xa5, 0x73, 0x58, 0xff},
  (color_t) {0x98, 0x69, 0x51, 0xff},
  (color_t) {0x8e, 0x64, 0x4d, 0xff},
  (color_t) {0x86, 0x55, 0x3c, 0xff},
  (color_t) {0x89, 0x3a, 0x2e, 0xff},
  (color_t) {0x88, 0x41, 0x43, 0xff},
  (color_t) {0x91, 0x34, 0x00, 0xff},
  (color_t) {0x6d, 0x44, 0x31, 0xff},
  (color_t) {0x58, 0x35, 0x27, 0xff},
  (color_t) {0x4c, 0x2e, 0x21, 0xff},
  (color_t) {0x41, 0x27, 0x1c, 0xff},
};
struct rdpq_textparms_s banner_params;
struct rdpq_textparms_s timer_params;
rspq_block_t *empty_hud_block;
bool paused;
struct subgame subgames[] = {
  {
    .fixed_loop = sauna_fixed_loop,
    .dynamic_loop_pre = sauna_dynamic_loop_pre,
    .dynamic_loop_render = sauna_dynamic_loop_render,
    .dynamic_loop_post = sauna_dynamic_loop_post,
    .cleanup = sauna_cleanup,
    .init = sauna_init,
  },
  {
    .fixed_loop = lake_fixed_loop,
    .dynamic_loop_pre = lake_dynamic_loop_pre,
    .dynamic_loop_render = lake_dynamic_loop_render,
    .dynamic_loop_post = lake_dynamic_loop_post,
    .cleanup = lake_cleanup,
    .init = lake_init,
  },
  {
    .fixed_loop = NULL,
    .dynamic_loop_pre = NULL,
    .dynamic_loop_render = NULL,
    .dynamic_loop_post = NULL,
    .cleanup = NULL,
    .init = NULL,
  },
};
struct subgame *current_subgame;

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
      FILTERS_RESAMPLE);
  z_buffer = display_get_zbuf();

  t3d_init((T3DInitParams){});
  tpx_init((TPXInitParams){});
  viewport = t3d_viewport_create();

  player_model = t3d_model_load("rom:/avanto/guy.t3dm");
  T3DModelDrawConf player_draw_conf = {
    .userData = NULL,
    .tileCb = NULL,
    .filterCb = filter_player_hair_color,
    .dynTextureCb = NULL,
    .matrices = NULL,
  };
  static T3DObject *body_object = NULL;
  T3DModelIter it = t3d_model_iter_create(player_model, T3D_CHUNK_TYPE_OBJECT);
  while (t3d_model_iter_next(&it)) {
    if (!strcmp("body", it.object->name)) {
      body_object = it.object;
      break;
    }
  }
  for (size_t i = 0; i < 4; i++) {
    players[i].rotation = 0;
    skeleton_init(&players[i].s, player_model, NUM_PLAYER_ANIMS);
    players[i].s.anims[WALK] = t3d_anim_create(player_model, "walking");
    players[i].s.anims[CLIMB] = t3d_anim_create(player_model, "climbing");
    t3d_anim_set_looping(&players[i].s.anims[CLIMB], false);
    players[i].s.anims[SIT] = t3d_anim_create(player_model, "sitting");
    t3d_anim_set_looping(&players[i].s.anims[SIT], false);
    players[i].s.anims[BEND] = t3d_anim_create(player_model, "bending");
    t3d_anim_set_looping(&players[i].s.anims[BEND], false);
    players[i].s.anims[UNBEND] = t3d_anim_create(player_model, "unbending");
    t3d_anim_set_looping(&players[i].s.anims[UNBEND], false);
    players[i].s.anims[STAND_UP] =
      t3d_anim_create(player_model, "standing_up");
    t3d_anim_set_looping(&players[i].s.anims[STAND_UP], false);
    players[i].s.anims[PASS_OUT] =
      t3d_anim_create(player_model, "passing_out");
    t3d_anim_set_looping(&players[i].s.anims[PASS_OUT], false);
    players[i].s.anims[SWIM] =
      t3d_anim_create(player_model, "swimming");
    t3d_anim_set_looping(&players[i].s.anims[SWIM], true);
    players[i].s.anims[DANCE] =
      t3d_anim_create(player_model, "dancing");
    t3d_anim_set_looping(&players[i].s.anims[DANCE], true);
    players[i].pos = (T3DVec3) {{0, 0, 0}};
    players[i].scale = 1.f;
    players[i].current_anim = -1;
    players[i].visible = false;
    players[i].temperature = 0.f;
    players[i].out = false;
    player_draw_conf.userData = &player_colors[i];
    int skin_color_index = rand() % (sizeof(skin_tones)/sizeof(color_t));
    body_object->material->primColor = skin_tones[skin_color_index];
    entity_init(&players[i].e,
        player_model,
        &(T3DVec3) {{players[i].scale, players[i].scale, players[i].scale}},
        &(T3DVec3) {{0, players[i].rotation, 0}},
        &players[i].pos,
        &players[i].s.skeleton,
        &player_draw_conf);
  }

  const color_t BLACK = RGBA32(0x00, 0x00, 0x00, 0xff);
  const color_t YELLOW = RGBA32(0xff, 0xff, 0x00, 0xff);
  const color_t WHITE = RGBA32(0xff, 0xff, 0xff, 0xff);
  const color_t LIGHT_BLUE = RGBA32(0x00, 0xc9, 0xff, 0xff);
  normal_font = rdpq_font_load("rom:/squarewave.font64");
  rdpq_text_register_font(FONT_NORMAL, normal_font);
  timer_font = rdpq_font_load("rom:/avanto/timer.font64");
  rdpq_text_register_font(FONT_TIMER, timer_font);
  banner_font = rdpq_font_load("rom:/avanto/banner.font64");
  rdpq_text_register_font(FONT_BANNER, banner_font);

  rdpq_font_t *fonts[] = {normal_font, timer_font, banner_font};
  for (size_t i = 0; i < 3; i++) {
    rdpq_font_t *font = fonts[i];

    rdpq_font_style(font,
        SW_NORMAL,
        &(rdpq_fontstyle_t) {.color = WHITE, .outline_color = BLACK});
    rdpq_font_style(font,
        SW_BANNER,
        &(rdpq_fontstyle_t) {.color = YELLOW, .outline_color = BLACK});
    rdpq_font_style(font,
        SW_TIMER,
        &(rdpq_fontstyle_t) {.color = YELLOW, .outline_color = BLACK});
    rdpq_font_style(font,
        SW_OUT,
        &(rdpq_fontstyle_t) {.color = LIGHT_BLUE, .outline_color = BLACK});
    rdpq_font_style(font,
        SW_OUT,
        &(rdpq_fontstyle_t) {.color = LIGHT_BLUE, .outline_color = BLACK});
    rdpq_font_style(font,
        SW_SELECTED,
        &(rdpq_fontstyle_t) {.color = YELLOW, .outline_color = BLACK});
    for (size_t j = 0; j < 4; j++) {
      rdpq_font_style(font,
          SW_PLAYER1 + j,
          &(rdpq_fontstyle_t) {
            .color = player_colors[j],
            .outline_color = BLACK,
          });
    }
  }
  memset(&banner_params, 0, sizeof(banner_params));
  banner_params.style_id = SW_BANNER;
  banner_params.align = 1;
  banner_params.valign = 1;
  banner_params.width = 320;

  memset(&timer_params, 0, sizeof(timer_params));
  timer_params.style_id = SW_TIMER;
  timer_params.align = 1;
  timer_params.width = 320;

  xm64player_open(&music, "rom:/avanto/sj-polkka.xm64");

  wav64_open(&sfx_start, "rom:/core/Start.wav64");
  wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
  wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
  wav64_open(&sfx_winner, "rom:/core/Winner.wav64");

  mixer_set_vol(1.f);
  for (int i = xm64player_num_channels(&music); i < 32; i++) {
    mixer_ch_set_vol(i, 0.5f, 0.5f);
    mixer_ch_set_limits(i, 0, 48000, 0);
  }

  empty_hud_block = build_empty_hud_block();

  paused = false;

  current_subgame = &subgames[0];
  current_subgame->init();
}

void minigame_fixedloop(float delta_time) {
  if (current_subgame->fixed_loop && !paused) {
    if (current_subgame->fixed_loop(delta_time)) {
      current_subgame->cleanup();
      current_subgame++;
      if (current_subgame->init) {
        current_subgame->init();
      }
      else {
        minigame_end();
      }
    }
  }
}

void minigame_loop(float delta_time) {
  static int paused_controller = 0;
  static int paused_selection = 0;
  static surface_t *first_paused = NULL;

  if (current_subgame->dynamic_loop_pre && !paused) {
    current_subgame->dynamic_loop_pre(delta_time);
  }

  surface_t *display_surface;
  display_surface = display_get();
  rdpq_attach(display_surface, z_buffer);
  t3d_frame_start();
  t3d_viewport_attach(&viewport);

  if (current_subgame->dynamic_loop_render && !paused) {
    current_subgame->dynamic_loop_render(delta_time);
  }

  rdpq_sync_pipe();
  rdpq_mode_push();
  rdpq_mode_zbuf(false, false);

  if (paused) {
    if (!first_paused) {
      rdpq_mode_push();
      rdpq_set_fog_color(RGBA32(0xff, 0xff, 0xff, 0xc0));
      rdpq_set_prim_color(RGBA32(0x00, 0x00, 0x00, 0xff));
      rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
      rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
      rdpq_fill_rectangle(0, 0, 320, 240);
      rdpq_mode_pop();

      first_paused = display_surface;
    }
    else if (first_paused != display_surface) {
      rdpq_mode_push();
      rdpq_set_mode_copy(false);
      rdpq_tex_blit(first_paused, 0, 0, NULL);
      rdpq_mode_pop();
    }

    MITIGATE_FONT_BUG;
    rdpq_text_print(&banner_params, FONT_BANNER, 0, 60, "PAUSED");

    rdpq_textparms_t params = {
      .width = 300,
      .height = 130,
      .wrap = WRAP_WORD,
    };

    MITIGATE_FONT_BUG;
    rdpq_text_printf(&params,
        FONT_NORMAL,
        10,
        85,
        "%s by %s\n\n%s\n\n%s",
        minigame_def.gamename,
        minigame_def.developername,
        minigame_def.description,
        minigame_def.instructions);

    MITIGATE_FONT_BUG;
    rdpq_text_printf(NULL,
        FONT_NORMAL,
        40,
        215,
        "%sRESUME",
        paused_selection? SW_NORMAL_S : SW_SELECTED_S);
    MITIGATE_FONT_BUG;
    rdpq_text_printf(NULL,
        FONT_NORMAL,
        40+160,
        215,
        "%sQUIT",
        !paused_selection? SW_NORMAL_S : SW_SELECTED_S);
  }

  rdpq_mode_pop();

  rdpq_detach_show();

  if (current_subgame->dynamic_loop_post && !paused) {
    current_subgame->dynamic_loop_post(delta_time);
  }

  if (!paused) {
    for (size_t i = 0; i < core_get_playercount(); i++) {
      joypad_buttons_t pressed = joypad_get_buttons_pressed(
          core_get_playercontroller(i));
      if (pressed.start) {
        first_paused = NULL;
        paused_controller = core_get_playercontroller(i);
        paused_selection = 0;
        paused = true;
        mixer_set_vol(.2f);
        break;
      }
    }
  }
  else {
    joypad_buttons_t pressed = joypad_get_buttons_pressed(paused_controller);
    int axis = joypad_get_axis_pressed(paused_controller, JOYPAD_AXIS_STICK_X);
    if (pressed.start || pressed.b || (pressed.a && !paused_selection)) {
      paused = false;
      mixer_set_vol(1.f);
    } else if (pressed.a) {
      minigame_end();
    } else if (pressed.d_left || axis || pressed.d_right) {
      paused_selection ^= 1;
    }
  }
}

void minigame_cleanup() {
  rspq_wait();

  rspq_block_free(empty_hud_block);

  if (current_subgame->cleanup) {
    current_subgame->cleanup();
  }
  wav64_close(&sfx_start);
  wav64_close(&sfx_countdown);
  wav64_close(&sfx_stop);
  wav64_close(&sfx_winner);

  xm64player_stop(&music);
  xm64player_close(&music);

  rdpq_text_unregister_font(FONT_NORMAL);
  rdpq_font_free(normal_font);
  rdpq_text_unregister_font(FONT_TIMER);
  rdpq_font_free(timer_font);
  rdpq_text_unregister_font(FONT_BANNER);
  rdpq_font_free(banner_font);

  for (size_t i = 0; i < 4; i++) {
    entity_free(&players[i].e);
    skeleton_free(&players[i].s);
  }
  t3d_model_free(player_model);

  tpx_destroy();
  t3d_destroy();
  display_close();
}
