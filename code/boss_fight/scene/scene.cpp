/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "../main.h"
#include "shadows.h"
#include "scene.h"
#include "../debug/debugDraw.h"
#include "../debug/overlay.h"
#include "../render/screenFX.h"

#include "actors/coin.h"
#include "actors/grass.h"
#include "actors/boss.h"
#include "actors/particles.h"
#include "actors/vase.h"
#include "actors/box.h"
#include "actors/void.h"
#include "actors/can.h"
#include "../utils/memory.h"

namespace {
  constexpr float CAM_MOVE_SPEED = 7.55f;
  constexpr float CAM_MOVE_FADE_DIST = 30.0f;
  constexpr float CAM_MOVE_WIN_DIST = 3.0f;

  constexpr float FADE_TIME_MAX = 2.0f;

  bool needsDetach = false;
  bool showFPS = false;
  bool debugOverlay = false;

  sprite_t *texGo;

  InputState getInputState(PlyNum p, float camRotY) {
    auto ctrl = core_get_playercontroller(p);
    auto btn = joypad_get_buttons_pressed(ctrl);
    auto stick = joypad_get_inputs(ctrl);

    T3DVec3 stickDir{{(float)stick.stick_x, 0, -(float)stick.stick_y}};

    InputState inp{
      .move = {{
        cosf(camRotY) * stickDir.v[0] - sinf(camRotY) * stickDir.v[2], 0.0f,
        sinf(camRotY) * stickDir.v[0] + cosf(camRotY) * stickDir.v[2]
      }},
      .jump = btn.a != 0,
      .jumpHold = stick.btn.a != 0,
      .attack = btn.b != 0
    };

    inp.move.v[0] *= 1.0f / 110.0f;
    inp.move.v[2] *= 1.0f / 110.0f;
    inp.move.v[0] = fminf(fmaxf(inp.move.v[0], -1.0f), 1.0f);
    inp.move.v[2] = fminf(fmaxf(inp.move.v[2], -1.0f), 1.0f);
    return inp;
  }
}

Scene::Scene()
:
  mapModel(FS_BASE_PATH "map.t3dm"),
  ptCoins{FS_BASE_PATH "ptx/coin.i4.sprite"},
  ptSpark{FS_BASE_PATH "ptx/spark.ia8.sprite", true},
  ptSwirl{FS_BASE_PATH "ptx/swirl.i4.sprite", true}
{
  needsDetach = false;
  followPlayer = false;

  Debug::init();
  Shadows::init();

  ptCoins.setColor({0xFF, 0xDF, 0x61, 0xFF});
  ptSpark.setColor({0xFF, 0xDF, 0x61, 0xFF});
  ptSwirl.setColor({0xFF, 0xFF, 0xFF, 0xFF});

  initCutscenes();
  texGo = sprite_load(FS_BASE_PATH "ui/txtGo.ia8.sprite");
  collMesh = Coll::Mesh::load(FS_BASE_PATH "map.coll");
  collMeshInstance.mesh = collMesh;
  collScene.registerMesh(&collMeshInstance);

  viewport = t3d_viewport_create();
  cam.reset();

  loadScene(FS_BASE_PATH "map.scene");
  cam.setTarget({{getPlayer(0).getPos().x*COLL_WORLD_SCALE, 0, -8}});

  changeState(State::INTRO);
}

Scene::~Scene() {
  for(auto actor : actors) {
    delete actor;
  }

  free(collMesh);
  Shadows::destroy();
  Debug::destroy();
  sprite_free(texGo);
}

void Scene::changeState(Scene::State newState) {
  state = newState;
}

const T3DVec3& Scene::getClosesRespawn(const T3DVec3 &pos) const {
  float minDist = 9999.0f;
  const T3DVec3 *closest = &respawnPoints[0];

  for(const auto &pt : respawnPoints) {
    float diffX = pos.v[0] - pt.v[0];
    float diffZ = pos.v[2] - pt.v[2];
    float dist2 = diffX * diffX + diffZ * diffZ;
    if(dist2 < minDist) {
      minDist = dist2;
      closest = (T3DVec3*)&pt;
    }
  }
  return *closest;
}

void Scene::spawnActor(uint32_t type, const T3DVec3 &pos, uint16_t param) {
  switch(type) {
    case "Coin"_u32: actors.push_back(new Actor::Coin(*this, pos, param)); break;
    case "Grss"_u32: actors.push_back(new Actor::Grass(*this, pos, param)); break;
    case "Boss"_u32: actors.push_back(new Actor::Boss(*this, pos, param)); break;
    case "Part"_u32: actors.push_back(new Actor::Particles(*this, pos, param)); break;
    case "Vase"_u32: actors.push_back(new Actor::Vase(*this, pos, param)); break;
    case "WBox"_u32: actors.push_back(new Actor::Box(*this, pos, param)); break;
    case "Void"_u32: actors.push_back(new Actor::Void(*this, pos, param)); break;
    case "TCan"_u32: actors.push_back(new Actor::Can(*this, pos, param)); break;
    default:
      debugf("Unknown actor %08lX pos=%f,%f,%f param=%d\n", type, pos.x, pos.y, pos.z, param);
    break;
  }
}

void Scene::updateVisibility()
{
  ticksCull = get_ticks();
  mapModel.update(cam.getTarget());
  ticksCull = get_ticks() - ticksCull;
}

void Scene::update(float deltaTime) {
  if(wantsExit) {
    rspq_wait();
    return minigame_end();
  }

  lastFB = currentFB;
  currentFB = display_get();
  rdpq_attach(currentFB, display_get_zbuf());
  needsDetach = true;

  t3d_viewport_attach(&viewport);

  ++frameIdx;
  collScene.ticks = 0;
  collScene.ticksBVH = 0;
  collScene.raycastCount = 0;

  if(!pauseMenu.isPaused) {
    switch(state) {
      case State::INTRO    : cutsceneIntro.update(deltaTime); break;
      case State::GAME     : cutsceneGame.update(deltaTime);  break;
      case State::GAME_OVER: cutsceneOutro.update(deltaTime); break;
    }
  }

  cam.update(viewport);
  skybox.update(cam.getTarget(), deltaTime);

  auto camMidPoint = (cam.getTarget() + cam.getPos()) * 0.5f; // make sounds appear closer
  audioManager.update(camMidPoint / COLL_WORLD_SCALE, cam.getTarget() / COLL_WORLD_SCALE, deltaTime);

  Shadows::reset();

  updateGame(deltaTime);

  currMostCoins = 0;
  for(auto &p : players) {
    currMostCoins = Math::max(currMostCoins, p.getCoinCount());
  }
}

void Scene::updateGame(float deltaTime)
{
  float camRotY = cam.getRotY();

  // DEBUG CONTROLS:
  {
    auto ctrl = core_get_playercontroller(PLAYER_1);
    auto btn = joypad_get_buttons_pressed(ctrl);
    auto held = joypad_get_buttons_held(ctrl);

    if(held.z) {
      if(btn.d_up)debugOverlay = !debugOverlay;
      if(btn.d_left)cutsceneIntro.skipToEnd();
      if(btn.d_down)showFPS = !showFPS;
    }
  }

  if(state == State::GAME)
  {
    bool wasPaused = pauseMenu.isPaused;
    pauseMenu.update(*this, deltaTime);
    if(pauseMenu.isPaused || wasPaused) { // skip one frame after un-pausing to ignore A-press
      updateVisibility();
      return;
    }
  }

  titleGoTimer.update(deltaTime);
  playerMarker.update(*this, deltaTime);

  ptCoins.clear();
  ptSpark.clear();
  ptSwirl.simulateDust(deltaTime);

  if(followPlayer) {
    float avgPosX = (players[0].getPos().x + players[1].getPos().x + players[2].getPos().x + players[3].getPos().x) / 4;
    cam.move({{(avgPosX*COLL_WORLD_SCALE - cam.getTarget().x) * 0.25f, 0,0}});
  }
  if(autoScroll) {
    auto diff = camEndPosX - cam.getTarget().x;

    float moveFactor = 1.0f;
    if(diff < CAM_MOVE_FADE_DIST) {
      moveFactor = diff / CAM_MOVE_FADE_DIST;
    }
    if(diff > 0.0f) {
      cam.move({deltaTime * CAM_MOVE_SPEED * moveFactor, 0, 0});
    }

    if(!hasBgmStopped && diff < (CAM_MOVE_WIN_DIST + 100.f)) {
      getAudio().stopBGM();
      hasBgmStopped = true;
    }

    if(diff < CAM_MOVE_WIN_DIST) {
      if(!hadBossSpawn) {
        hadBossSpawn = true;
        spawnActor("Boss"_u32, {(camEndPosX + 140) / COLL_WORLD_SCALE, 0, 0}, 0);
        getAudio().playBGM("Boss"_u64);
      }
      //changeState(State::GAME_OVER);
    }
  }

  ticksActorUpdate = get_ticks();
  // Players / Boss
  uint32_t playerCount = forceAI ? 0 : core_get_playercount();
  for(uint32_t i=0; i<4; ++i)
  {
    if(!overrideInput) {
      if(i < playerCount) {
        input[i] = getInputState((PlyNum)(PlyNum::PLAYER_1 + i), camRotY);
      } else {
        input[i] = playerAI[i].update(deltaTime);
        //input[i] = {};
      }
    }
    players[i].update(input[i], deltaTime);
  }

  if(!actorSpawnReqs.empty()) {
    for(auto &spawnReq : actorSpawnReqs) {
      spawnActor(spawnReq.type, spawnReq.pos, spawnReq.param);
    }
    actorSpawnReqs.clear();
  }

  activeActorCount = actors.size();
  drawActorCount = 0;

  for(auto & actor : actors) {
    actor->update(deltaTime);
    if(actor->drawMask != 0)++drawActorCount;
  }

  uint32_t actorIdx = 0;
  for(auto & actor : actors) {
    if(actor->deleteFlag) {
      delete actor;
    } else {
      actors[actorIdx++] = actor;
    }
  }
  actors.resize(actorIdx);

  ticksActorUpdate = get_ticks() - ticksActorUpdate;
  collScene.update(deltaTime);
  updateVisibility();
}

void Scene::draw(float deltaTime)
{
  if(wantsExit) {
    if(needsDetach)rdpq_detach_show();
    needsDetach = false;
    return;
  }

  if(pauseMenu.isPaused)
  {
    pauseMenu.draw();
    if(needsDetach)rdpq_detach_show();
    needsDetach = false;
    return;
  }

  skybox.draw();

  t3d_frame_start();
  rdpq_mode_dithering(DITHER_NONE_NONE);
  rdpq_mode_antialias(AA_NONE);

  t3d_screen_clear_depth();

  T3DVec3 lightDir{{0.6f, 0.6f, 0.31f}};
  t3d_vec3_norm(lightDir);

  t3d_light_set_ambient({0xAA, 0x7A, 0x7A, 0x00});
  //t3d_light_set_ambient({0x22, 0x22, 0x22, 0x00});
  t3d_light_set_directional(0, color_t{0xFF, 0xFF, 0xFF, 0x00}, lightDir);
  for(uint32_t i=0; i<pointLightCount; ++i) {
    float strength = fabsf(pointLights[i].strength);
    t3d_light_set_point(i+1, pointLights[i].color, pointLights[i].pos, strength, pointLights[i].strength < 0.0f);
  }
  t3d_light_set_count(1 + pointLightCount);
  pointLightCount = 0;

  t3d_matrix_push_pos(1);

  rdpq_set_prim_color({0xFF, 0xFF, 0xFF, 0xFF});
  rdpq_set_env_color({0xFF, 0xFF, 0xFF, 0x00});
  //t3d_light_set_ambient(ambientColor);

  t3dState = t3d_model_state_create();
  uint32_t triCount = mapModel.draw(t3dState);

  for(auto & player : players) {
    player.draw(deltaTime);
  }

  t3dState = t3d_model_state_create();
  for(auto actor : actors) {
    if(actor->drawMask & Actor::DRAW_MASK_3D) {
      actor->draw3D(deltaTime);
    }
  }

  t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);
  t3dState.lastVertFXFunc = T3D_VERTEX_FX_NONE;

  Shadows::draw();

  // Particles
  rdpq_sync_pipe();
  rdpq_mode_push();
  rdpq_set_mode_standard();
  rdpq_mode_zbuf(true, true);
  rdpq_mode_zoverride(true, 0, 0);
  rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,0,ENV,0), (PRIM,0,ENV,0)));
  rdpq_set_env_color({0xFF, 0xFF, 0xFF, 0xFF});

  t3d_matrix_pop(1);

  tpx_state_from_t3d();
  tpx_state_set_scale(0.5f, 0.5f);

  for(auto actor : actors) {
    if(actor->drawMask & Actor::DRAW_MASK_PTX) {
      actor->drawPtx(deltaTime);
    }
  }

  rdpq_sync_load();
  ptCoins.draw(deltaTime);

  rdpq_sync_pipe();
  rdpq_set_env_color({0xFF, 0xFF, 0xFF, 0xFF});
  ptSpark.draw(deltaTime); // @TODO: disable blending

  rdpq_sync_pipe();
  rdpq_sync_tile();
  rdpq_sync_load();

  rdpq_set_env_color({0xFF, 0xFF, 0xFF, 0xAA});
  ptSwirl.draw(deltaTime);

  rdpq_sync_pipe();
  rdpq_mode_blender(0);
  rdpq_mode_zbuf(true, true);
  rdpq_set_env_color({0xFF, 0xFF, 0xFF, 0xFF});

  rdpq_mode_pop();
  t3d_matrix_push_pos(1);
  for(auto & player : players) {
    player.drawTransp(deltaTime);
  }
  t3d_matrix_pop(1);

  // 2D / UI
  rdpq_sync_pipe();
  rdpq_sync_tile();
  rdpq_set_mode_standard();
  rdpq_mode_filter(FILTER_POINT);
  rdpq_mode_zbuf(false, false);

  for(auto actor : actors) {
    if(actor->drawMask & Actor::DRAW_MASK_2D) {
      actor->draw2D(deltaTime);
    }
  }

  for(auto & player : players) {
    player.draw2D();
  }

  float camXTotal = camEndPosX - camStartPosX;
  float camXCur = cam.getTarget().x - camStartPosX;
  float relFactor = camXCur / camXTotal;
  relFactor = fminf(fmaxf(relFactor, 0.0f), 1.0f);
  progBar.draw(relFactor, deltaTime);

  playerMarker.draw();

  if(state == State::GAME_OVER) {
    winScreen.draw(deltaTime);
  }

  if(titleGoTimer.value > 0.0f) {
    float s = titleGoTimer.value * 1.5f;
    s = Math::easeOutSin(fminf(s, 1.0f));
    rdpq_blitparms_t p{
      .cx = texGo->width / 2,
      .cy = texGo->height / 2,
      .scale_x = s, .scale_y = s
    };
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_sprite_blit(texGo, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, &p);
    rdpq_mode_filter(FILTER_POINT);
  }

  rdpq_set_mode_fill({});

  fadeTimer.update(deltaTime);
  if(fadeTimer.value > 0.0f) {
    float s = Math::easeOutSin(fadeTimer.value / FADE_TIME_MAX);
    FX::drawNGonOverlay(6,
      {SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0},
      (1.0f-s) * 230.0f, 240.0f,
      s*1.75f, {0x00, 0x00, 0x00, 0xFF}
    );
  }

  uiBarTimer.update(deltaTime);
  if(uiBarTimer.value > 0.0f) {
    FX::drawBars(uiBarTimer.value * 20.0f);
  }

  // Debug UI
  if(debugOverlay) {
    debugOvl.draw(*this, triCount, deltaTime);
    Debug::draw((uint16_t*)currentFB->buffer);

    Debug::printStart();
    for(auto &ai : playerAI) {
      ai.debugDraw();
    }

  } else if(showFPS) {
    Debug::printStart();
    Debug::printf(24, 220, "FPS %.2f", display_get_fps());
  }

  rdpq_detach_show();
  #if RSPQ_PROFILE
    rspq_profile_next_frame();
    if(frameIdx % 60 == 0) {
      rspq_wait();
      rspq_profile_dump();
      rspq_profile_reset();
    }
  #endif
}

