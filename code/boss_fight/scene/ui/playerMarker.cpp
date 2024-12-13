/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "playerMarker.h"
#include "../../main.h"
#include "../scene.h"
#include <libdragon.h>

namespace {
  constexpr int BORDER_SIZE = 8;
  constexpr int SPRITE_SIZE = 16;
  constexpr float FADE_TIME = 0.33f;

  constexpr int MIN_SCREEN_X = BORDER_SIZE;
  constexpr int MAX_SCREEN_X = (int)SCREEN_WIDTH - BORDER_SIZE;

  sprite_t *texMarker{nullptr};

  struct Marker {
    int posX;
    int posY;
    float timer{};
  };
  Marker marker[4]{};
}

PlayerMarker::PlayerMarker() {
  assert(texMarker == nullptr); // singleton
  texMarker = sprite_load(FS_BASE_PATH "ui/marker.ia8.sprite");
}

PlayerMarker::~PlayerMarker() {
  sprite_free(texMarker);
  texMarker = nullptr;
}

void PlayerMarker::update(Scene &scene, float deltaTime)
{
  for(uint32_t i=0; i<4; ++i)
  {
    auto &screenPos = scene.getPlayer(i).getScreenPos();
    bool isActive = !scene.getPlayer(i).isDead() && (screenPos.x < MIN_SCREEN_X || screenPos.x > MAX_SCREEN_X);
    if(isActive) {
      marker[i].posX = Math::max(MIN_SCREEN_X, (int)screenPos.x);
      marker[i].posX = Math::min((int)MAX_SCREEN_X - SPRITE_SIZE, marker[i].posX);
      marker[i].posY = Math::max(BORDER_SIZE, (int)screenPos.y);
      marker[i].posY = Math::min((int)SCREEN_HEIGHT - BORDER_SIZE - SPRITE_SIZE, marker[i].posY);
      marker[i].timer = fminf(marker[i].timer + deltaTime, FADE_TIME);
    } else {
      marker[i].timer = fmaxf(marker[i].timer - deltaTime, 0.0f);
    }
  }
}

void PlayerMarker::draw()
{
  rdpq_mode_alphacompare(32);
  rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
  rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
  rdpq_sprite_upload(TILE0, texMarker, nullptr);

  for(uint32_t i=0; i<4; ++i)
  {
    auto &m = marker[i];
    if(m.timer > 0.0f)
    {
      int offsetU = i * 16;
      int offsetV = m.posX > ((int)SCREEN_WIDTH / 2) ? SPRITE_SIZE : 0;

      auto col = PLAYER_COLORS[i];
      col.a = (uint8_t)(m.timer / FADE_TIME * 255.0f);
      rdpq_set_prim_color(col);

      rdpq_texture_rectangle_scaled(TILE0,
        m.posX, m.posY, m.posX + SPRITE_SIZE, m.posY + SPRITE_SIZE,
        offsetU, offsetV, offsetU + SPRITE_SIZE, offsetV + SPRITE_SIZE
      );
    }
  }

  rdpq_set_prim_color({0xFF, 0xFF, 0xFF, 0xFF});
}
