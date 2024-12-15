/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "healthMeter.h"

namespace {
  constexpr float OFFSET_Y = 22.0f;
  constexpr float VISIBLE_TIME = 1.75f;
  constexpr float FADE_OUT_TIME = 0.125f;

  color_t getGradientColor(float value) {
    if(value < 0.5f) {
      return (color_t){
        (uint8_t)(255 * value * 2),
        255,
        0,
        255
      };
    }
    return (color_t){
      255,
      (uint8_t)(255 * (1.0f - value) * 2),
      0,
      255
    };
  }
}

void HealthMeter::reduce(int amount) {
  value -= amount;
  if(value < 0)value = 0;
  visibleTimer = VISIBLE_TIME;
}

void HealthMeter::increase(int amount) {
  value += amount;
  if(value > 100)value = 100;
  visibleTimer = VISIBLE_TIME;
}

void HealthMeter::draw(const T3DVec3 &posScreen, float maxWidth) {
  if(visibleTimer == 0.0f)return;

  if(visibleTimer <= FADE_OUT_TIME) {
    maxWidth *= visibleTimer * (1.0f / FADE_OUT_TIME);
  }

  float maxHalfWidth = maxWidth / 2.0f;
  float healthWidth = maxWidth * (value / 100.0f);
  auto basePos = posScreen - T3DVec3{{maxHalfWidth-1, OFFSET_Y, 0}};

  // background
  rdpq_set_prim_color({0x00, 0x00, 0x00, 0xFF});
  rdpq_fill_rectangle(
    basePos.v[0], basePos.v[1],
    basePos.v[0] + maxWidth,
    basePos.v[1] + 5
  );

  // health line
  rdpq_set_prim_color(getGradientColor(1.0f - (value / 100.0f)));
  rdpq_fill_rectangle(
    basePos.v[0]+1, basePos.v[1]+1,
    basePos.v[0] + healthWidth - 1,
    basePos.v[1] + 4
  );
}
