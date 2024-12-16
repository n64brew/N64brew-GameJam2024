#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

/**
 * Simple clone of Hole.io
 * Basically just pushing myself to make something in a shorter time.
 */

const MinigameDef minigame_def = {
    .gamename = "holes",
    .developername = "Strawberry Byte: s4ys",
    .description = "Clone of Hole.io",
    .instructions = "Try to devour as much as possible!"};

// Global subsystems
#include "util.h"
#include "sound.h"

// Locals
T3DModel *model;
T3DModel *modelCar;
T3DModel *modelBuilding;
T3DModel *modelHydrant;

////////// MAP
#include "map.h"
map_data map;

////////// CAMERA
camera_data cam[MAXPLAYERS];

////////// OBJECTS
#include "objects.h"
object_type objects[NUM_OBJ_TYPES];
T3DVec3 gridPos[MAX_GRID_POINTS];
size_t gridPointCount = 0;
T3DObject *buildings[2];
bool spray[NUM_OBJECTS] = {false};
bool stop[NUM_OBJECTS] = {false};

////////// PLAYERS
#include "player.h"
player_data players[MAXPLAYERS];

////////// TEXT
#include "ui.h"

////////// VIEWPORTS
#include "screen.h"
surface_t *depthBuffer;
T3DViewport viewport[MAXPLAYERS];

////////// SCENE
#include "render.h"
#include "scene.h"
scene_data scenes[NUM_SCENES];

game_data game;

void minigame_init(void)
{

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
  depthBuffer = display_get_zbuf();

#ifdef DEBUG_RDP
  rdpq_debug_start();
#endif

  t3d_init((T3DInitParams){});

  ui_init();

  viewport_create(viewport);

  map_init(&map);

  ////////// OBJECTS
  modelCar = t3d_model_load("rom:/sb_holes/car.t3dm");
  modelBuilding = t3d_model_load("rom:/sb_holes/building.t3dm");
  modelHydrant = t3d_model_load("rom:/sb_holes/hydrant.t3dm");

  for (int i = 0; i < NUM_OBJ_TYPES; i++)
  {
    object_initBatch(&objects[i], i);
  }

  ////////// PLAYERS
  model = t3d_model_load("rom:/sb_holes/hole.t3dm");

  T3DVec3 start_positions[] = {
      (T3DVec3){{-50, 5, -52}},
      (T3DVec3){{52, 5, -52}},
      (T3DVec3){{-50, 5, 40}},
      (T3DVec3){{52, 5, 40}},
  };

  float start_rotations[] = {
      M_PI / 2,
      0,
      3 * M_PI / 2,
      M_PI};

  const color_t colors[] = {
      PLAYERCOLOR_1,
      PLAYERCOLOR_2,
      PLAYERCOLOR_3,
      PLAYERCOLOR_4,
  };

  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    player_init(&players[i], colors[i], start_positions[i], start_rotations[i]);
    players[i].plynum = i;
    cam[i].position = (T3DVec3){{players[i].playerPos.x, players[i].playerPos.y + 150.0f, players[i].playerPos.z + 100.0f}};
    cam[i].target = players[i].playerPos;
  }
  //////////

  game.playerCount = core_get_playercount();
  game.introTimer = INTRO_DELAY;
  game.countDownTimer = COUNTDOWN_DELAY;
  game.isEnding = false;
  game.endTimer = 0;
  game.winner = 0;
  game.syncPoint = 0;
  scene_init(&game, scenes);
  game.scene = INTRO;

  // Decide whether to limit FPS and/or disable background music
  switch (game.playerCount)
  {
  case 1:
    sound_load(true);
    break;
  case 2:
  case 3:
    sound_load(true);
    display_set_fps_limit(display_get_refresh_rate() * 0.5f);
    break;
  case 4:
    sound_load(false);
    display_set_fps_limit(display_get_refresh_rate() * 0.5f);
    break;
  }
}

void minigame_fixedloop(float deltaTime)
{
  scenes[game.scene].fixedLoop(&game, deltaTime);
}

void minigame_loop(float deltaTime)
{
  scenes[game.scene].loop(&game, deltaTime);
}

void minigame_cleanup(void)
{
#ifdef DEBUG_RDP
  rdpq_debug_stop();
#endif

  sound_cleanup();

  display_set_fps_limit(0);

  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    player_cleanup(&players[i]);
  }

  for (int i = 0; i < NUM_OBJ_TYPES; i++)
  {
    object_destroyBatch(&objects[i]);
  }

  t3d_model_free(model);

  map_cleanup(&map);

  ui_cleanup();

  t3d_destroy();

  display_close();
}
