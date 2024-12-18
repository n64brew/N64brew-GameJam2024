/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "progBar.h"
#include "../../main.h"

namespace {
  sprite_t *texProgPoint;
  sprite_t *texProgHead;
  rspq_block_t *setupDPL;

  constexpr float BAR_WIDTH = 200.0f;
  constexpr float BAR_START_Y = 12.0f;
  float BANNER_START_X{};
}

ProgBar::ProgBar() {
  BANNER_START_X = SCREEN_WIDTH * 0.5f - BAR_WIDTH * 0.5f;

  texProgPoint = sprite_load(FS_BASE_PATH "ui/progPoint.ia8.sprite");
  texProgHead = sprite_load(FS_BASE_PATH "ui/head.rgba32.sprite");

  rspq_block_begin();
    rdpq_tex_multi_begin();
      rdpq_sprite_upload(TILE0, texProgPoint, nullptr);
      rdpq_sprite_upload(TILE1, texProgHead, nullptr);
    rdpq_tex_multi_end();

    rdpq_set_prim_color({0x98, 0x95, 0xAD, 0xFF});

    rdpq_mode_begin();
      rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
      rdpq_mode_alphacompare(10);
      rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_end();

    // static bar in the background
    float xEnd = BANNER_START_X + BAR_WIDTH - 16;
    rdpq_texture_rectangle(TILE0, BANNER_START_X, BAR_START_Y, xEnd, BAR_START_Y + 16, 0, 0);
    rdpq_texture_rectangle(TILE0, xEnd+16-1, BAR_START_Y, xEnd-1, BAR_START_Y + 16, 0,0);

    rdpq_set_prim_color({0xFF, 0xFF, 0xFF, 0xFF});

  setupDPL = rspq_block_end();
}

ProgBar::~ProgBar() {
  sprite_free(texProgPoint);
  sprite_free(texProgHead);
  rspq_block_free(setupDPL);
}

void ProgBar::draw(float prog, float deltaTime)
{
  float posX = BANNER_START_X + prog * (BAR_WIDTH-18) + 2;
  constexpr float posY = BAR_START_Y - 4;

  rspq_block_run(setupDPL);
  rdpq_texture_rectangle(TILE1, posX, posY, posX + texProgHead->width, posY + texProgHead->height, 0,0);
}
