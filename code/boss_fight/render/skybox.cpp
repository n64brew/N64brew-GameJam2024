/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "skybox.h"
#include "../main.h"

namespace
{
  constexpr uint32_t TMEM_SIZE = 0x1000;
  constexpr int BG_START_Y = 32;

  constexpr float FADE_START = 370.0f;
  constexpr float FADE_END = 630.0f;

  sprite_t *bg00{nullptr};
  float scrollX = 0.0f;
  float timer = 0.0f;
  float brightness = 1.0f;
}

Skybox::Skybox() {
  assert(bg00 == nullptr);
  bg00 = sprite_load(FS_BASE_PATH "bg00.rgba16.sprite");
}

Skybox::~Skybox() {
  sprite_free(bg00);
  bg00 = nullptr;
}

void Skybox::update(const T3DVec3 &camPos, float deltaTime)
{
  timer += deltaTime;
  scrollX = camPos.x * 0.25f;
  if(scrollX < 0.0f)scrollX += 512;
  float maxWidth = bg00->width * 2;
  if(scrollX > maxWidth)scrollX -= maxWidth;

  if(camPos.x > FADE_START) {
    brightness = 1.0f - (camPos.x - FADE_START) / (FADE_END - FADE_START);
    if(brightness < 0.0f)brightness = 0.0f;
  } else {
    brightness = 1.0f;
  }
}

void Skybox::draw()
{
  if(brightness <= 0)return;

  // top bar solid color, later partially overdrawn by UI
  rdpq_set_mode_fill({
   (uint8_t)(0x80 * brightness),
   (uint8_t)(0xb8 * brightness),
   (uint8_t)(0xd8 * brightness),
    0xFF
  });
  rdpq_fill_rectangle(0, 0, SCREEN_WIDTH, BG_START_Y);

  // image, we upload and draw slice by slice to make use of tile scrolling
  rdpq_set_mode_standard();
  rdpq_mode_dithering(DITHER_SQUARE_SQUARE);
  rdpq_mode_filter(FILTER_BILINEAR);
  rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
  uint8_t col = (uint8_t)(0xFF * brightness);
  rdpq_set_prim_color({col, col, col, 0xFF});


  uint32_t sliceHeight = TMEM_SIZE / (bg00->width * 2);
  assertf(bg00->height % sliceHeight == 0, "Bg-height: %ld", sliceHeight);

  auto surf = sprite_get_pixels(bg00);
  rdpq_texparms_t texParam = {
      .s = {.translate = 0, .repeats = REPEAT_INFINITE},
  };

  uint32_t posY = BG_START_Y;
  for(uint32_t srcPosY = 0; srcPosY < bg00->height; srcPosY += sliceHeight) {
    /*if(srcPosY > 60) {
      texParam.s.translate = scrollX + fm_sinf(timer*5.0f + srcPosY * 0.1f) * 6.0f;
      if(texParam.s.translate < 0.0f)texParam.s.translate += 512;
    }*/

    auto surfSub = surface_make_sub(&surf, 0, srcPosY, bg00->width, sliceHeight);
    rdpq_tex_upload(TILE0, &surfSub, &texParam);
    rdpq_texture_rectangle_scaled(TILE0,
      0, posY,
      SCREEN_WIDTH, posY + sliceHeight*2,
      scrollX, 0,
      scrollX + bg00->width * 0.75f, sliceHeight
    );
    posY += sliceHeight*2;
  }
}
