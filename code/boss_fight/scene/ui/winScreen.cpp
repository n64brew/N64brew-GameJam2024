/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "winScreen.h"
#include "../../main.h"
#include "../../utils/math.h"
#include <libdragon.h>

namespace {
  constexpr float PL_TILE_SIZE = 26;

  sprite_t *txtWin{};
  sprite_t *txtPlayers{};
  uint8_t winners[4]{0};
  uint8_t winnerCount{0};
  float fxTimer{};
  Math::Timer timer{};
}

WinScreen::WinScreen() {
  assert(txtWin == nullptr); // singleton

  fxTimer = 0.0f;
  winnerCount = 0;
  timer = {};
  txtWin = sprite_load(FS_BASE_PATH "ui/txtWin.ia8.sprite");
  txtPlayers = sprite_load(FS_BASE_PATH "ui/txtPlayers.ia8.sprite");
}

WinScreen::~WinScreen() {
  sprite_free(txtWin);
  txtWin = nullptr;
  sprite_free(txtPlayers);
  txtPlayers = nullptr;
}

void WinScreen::draw(float deltaTime) {
  timer.update(deltaTime);
  if(timer.value <= 0.0f)return;
  fxTimer += deltaTime * 4.0f;

  float s = timer.value * 0.7f;
  s = Math::easeOutSin(fminf(s, 1.0f));
  rdpq_blitparms_t p{
    .cx = txtWin->width / 2,
    .cy = txtWin->height / 2,
    .scale_x = s * (1.0f + fm_sinf(fxTimer) * 0.1f),
    .scale_y = s * (1.0f + fm_cosf(fxTimer) * 0.1f),
  };

  // big "Win" text
  rdpq_sprite_blit(txtWin, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 40, &p);

  // player icons
  float tileSize = PL_TILE_SIZE * s * 0.8f;
  float tileSizeHalf = tileSize / 2;

  rdpq_sprite_upload(TILE0, txtPlayers, nullptr);
  float posX = (SCREEN_WIDTH / 2) - ((winnerCount-1) * 16);
  float basePosY = (SCREEN_HEIGHT / 2 - 12) * s;

  for (int i = 0; i < 4; ++i) {
    if (winners[i] == 0) continue;
    float posY = basePosY + fm_sinf(fxTimer + i * 0.8f) * 6.0f;
    rdpq_set_prim_color(PLAYER_COLORS[i]);
    rdpq_texture_rectangle_scaled(TILE0,
      posX - tileSizeHalf, posY - tileSizeHalf,
      posX + tileSizeHalf, posY + tileSizeHalf,
      i*PL_TILE_SIZE, 0,
      (i+1)*PL_TILE_SIZE-0.5f, PL_TILE_SIZE-0.5f
    );
    posX += 32;
  }
}

void WinScreen::setWinner(uint8_t winner[4]) {
  winnerCount = 0;
  for(int i = 0; i < 4; ++i) {
    winners[i] = winner[i];
    winnerCount += winner[i] != 0;
  }
  timer.target = 2.0f;
}
