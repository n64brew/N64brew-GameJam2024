/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "overlay.h"
#include "debugDraw.h"
#include "../scene/scene.h"

namespace {
  constexpr float barWidth = 200.0f;
  constexpr float barHeight = 3.0f;
  constexpr float barRefTimeMs = 1000.0f / 30.0f; // FPS

  constexpr color_t COLOR_BVH{0,0xAA,0x22, 0xFF};
  constexpr color_t COLOR_COLL{0x22,0xFF,0, 0xFF};
  constexpr color_t COLOR_ACTOR_UPDATE{0xAA,0,0, 0xFF};
  constexpr color_t COLOR_CULL{0xFF,0x11,0x99, 0xFF};

  uint64_t ticksSelf = 0;

  constexpr float usToWidth(long timeUs) {
    double timeMs = (double)timeUs / 1000.0;
    return (float)(timeMs / barRefTimeMs) * barWidth;
  }

  bool showCollMesh = false;
  bool showCollSpheres = false;
  bool actorDebug = false;
}

void Debug::Overlay::draw(Scene &scene, int triCount, float deltaTime) {
  auto collScene = scene.getCollScene();
  uint64_t newTicksSelf = get_ticks();

  auto btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
  auto held = joypad_get_buttons_held(JOYPAD_PORT_1);

  if(menu.items.empty()) {
    menu.items.push_back({"Spheres", 0, true, [](MenuItem &item) {
      showCollSpheres = item.value;
    }});
    menu.items.push_back({"Coll-Tri", 0, true, [](MenuItem &item) {
      showCollMesh = item.value;
    }});
    menu.items.push_back({"Actor", 0, true, [](MenuItem &item) {
      actorDebug = item.value;
    }});
    menu.items.push_back({"Focus Player", scene.followPlayer, true, [&scene](MenuItem &item) {
      scene.followPlayer = item.value;
    }});
    menu.items.push_back({"Scroll", scene.autoScroll, true, [&scene](MenuItem &item) {
      scene.autoScroll = item.value;
    }});
    menu.items.push_back({"All-AI", scene.forceAI, true, [&scene](MenuItem &item) {
      scene.forceAI = item.value;
    }});
    menu.items.push_back({"Boss", 1, false, [&scene](MenuItem &item) {
      auto p = scene.getCamera().getTarget();
      p.x = 810;
      scene.getCamera().setTarget(p);
      item.value = 1;
    }});
  }

  if(btn.d_up)menu.currIndex = (menu.currIndex - 1) % menu.items.size();
  if(btn.d_down)menu.currIndex = (menu.currIndex + 1) % menu.items.size();
  if(btn.d_left)menu.items[menu.currIndex].value--;
  if(btn.d_right)menu.items[menu.currIndex].value++;
  if(btn.d_left || btn.d_right) {
    auto &item = menu.items[menu.currIndex];
    if(item.isBool)item.value = (item.value < 0) ? 1 : (item.value % 2);
    item.onChange(item);
  }

  if(held.l)scene.getCamera().move({held.z ? -8.0f : -2.0f, 0.0f, 0.0f});
  if(held.r)scene.getCamera().move({held.z ? 8.0f : 2.0f, 0.0f, 0.0f});

  collScene.debugDraw(showCollMesh, showCollSpheres);

  /*auto navPt = scene.getNavPoints();
  for(const auto &point : navPt.points) {
    auto ptWorld = point * COLL_WORLD_SCALE;
    Debug::drawSphere(ptWorld, 8.0f, {0xFF,0x00,0x00, 0xFF});
  }*/

  if(actorDebug) {
    for(const auto actor : scene.getActors()) {
      actor->drawDebug();
    }
  }

  float posX = 24;
  float posY = 24;

  Debug::printStart();
  Debug::printf(posX + barWidth + 2, 14, "FPS %.2f", display_get_fps());
  Debug::printf(posX + barWidth + 2, 34, "Cam %.2f", scene.getCamera().pos.x);

  heap_stats_t heap_stats;
  sys_get_heap_stats(&heap_stats);

  rdpq_set_prim_color(COLOR_BVH);
  posX = Debug::printf(posX, posY, "%.2f", (double)TICKS_TO_US(collScene.ticksBVH) / 1000.0) + 8;
  rdpq_set_prim_color(COLOR_COLL);
  posX = Debug::printf(posX, posY, "%.2f", (double)TICKS_TO_US(collScene.ticks - collScene.ticksBVH) / 1000.0) + 2;
  posX = Debug::printf(posX, posY, ":%d", collScene.raycastCount) + 8;
  rdpq_set_prim_color(COLOR_ACTOR_UPDATE);
  posX = Debug::printf(posX, posY, "%.2f", (double)TICKS_TO_US(scene.ticksActorUpdate) / 1000.0) + 8;
  rdpq_set_prim_color(COLOR_CULL);
  Debug::printf(posX, posY + 9, "%.2f", (double)TICKS_TO_US(scene.ticksCull) / 1000.0);
  posX = Debug::printf(posX, posY, "%.2f", (double)TICKS_TO_US(scene.getAudio().ticks) / 1000.0) + 8;

  rdpq_set_prim_color({0xFF,0xFF,0xFF, 0xFF});

  posX = 24 + barWidth - 50;
  posX = Debug::printf(posX, posY, "A:%d/%d", scene.activeActorCount, scene.drawActorCount) + 8;
  posX = Debug::printf(posX, posY, "T:%d", triCount) + 8;
  Debug::printf(posX, posY, "H:%d", heap_stats.used / 1024);

  posX = 24;

  // Player Pos / Velocity)
  if(actorDebug) {
    for(int i=0; i<4; ++i) {
      auto &posPlayer = scene.getPlayer(i).getScreenPos();
      auto plColl = scene.getPlayer(i).getColl();
      Debug::printf(posPlayer.v[0], posPlayer.v[1] - 16, "%.1f:%.1f", plColl.center.x, plColl.velocity.x);
      Debug::printf(posPlayer.v[0], posPlayer.v[1] -  8, "%.1f:%.1f", plColl.center.y, plColl.velocity.y);
      Debug::printf(posPlayer.v[0], posPlayer.v[1] -  0, "%.1f:%.1f", plColl.center.z, plColl.velocity.z);
    }
  }

  // Menu
  posY = 38;
  for(auto &item : menu.items) {
    bool isSel = menu.currIndex == &item - &menu.items[0];
    Debug::printf(posX, posY, "%c %s: %d", isSel ? '>' : ' ', item.text, item.value);
    posY += 8;
  }

  // audio channels
  posX = 24;
  posY = SCREEN_HEIGHT - 24;

  posX = Debug::printf(posX, posY, "CH ");
  uint32_t audioMask = scene.getAudio().getActiveChannelMask();
  for(int i=0; i<16; ++i) {
    bool isActive = audioMask & (1 << i);
    posX = Debug::printf(posX, posY, isActive ? "%d" : "-", i);
  }

  posX = 24;
  posY = 16;

  // Performance graph
  float timeCollBVH = usToWidth(TICKS_TO_US(collScene.ticksBVH));
  float timeColl = usToWidth(TICKS_TO_US(collScene.ticks - collScene.ticksBVH));
  float timeActorUpdate = usToWidth(TICKS_TO_US(scene.ticksActorUpdate));
  float timeCull = usToWidth(TICKS_TO_US(scene.getAudio().ticks));
  float timeSelf = usToWidth(TICKS_TO_US(ticksSelf));

  rdpq_set_mode_fill({0,0,0, 0xFF});
  rdpq_fill_rectangle(posX-1, posY-1, posX + (barWidth/2), posY + barHeight+1);
  rdpq_set_mode_fill({0x33,0x33,0x33, 0xFF});
  rdpq_fill_rectangle(posX-1 + (barWidth/2), posY-1, posX + barWidth+1, posY + barHeight+1);

  rdpq_set_fill_color(COLOR_BVH);
  rdpq_fill_rectangle(posX, posY, posX + timeCollBVH, posY + barHeight); posX += timeCollBVH;
  rdpq_set_fill_color(COLOR_COLL);
  rdpq_fill_rectangle(posX, posY, posX + timeColl, posY + barHeight); posX += timeColl;
  rdpq_set_fill_color(COLOR_ACTOR_UPDATE);
  rdpq_fill_rectangle(posX, posY, posX + timeActorUpdate, posY + barHeight); posX += timeActorUpdate;
  rdpq_set_fill_color(COLOR_CULL);
  rdpq_fill_rectangle(posX, posY, posX + timeCull, posY + barHeight); posX += timeCull;
  rdpq_set_fill_color({0xFF,0xFF,0xFF, 0xFF});
  rdpq_fill_rectangle(24 + barWidth - timeSelf, posY, 24 + barWidth, posY + barHeight);

  newTicksSelf = get_ticks() - newTicksSelf;
  if(newTicksSelf < TICKS_FROM_MS(2))
  {
    ticksSelf = newTicksSelf;
  }
}
