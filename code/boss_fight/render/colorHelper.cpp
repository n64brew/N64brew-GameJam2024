/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "colorHelper.h"

color_t ColorHelper::primHurtEffect(float timer, float invMaxTime) {
  float red = (0.5f + 0.5f * sinf(timer * 30.0f));
  red = 1.0f - (red * timer * invMaxTime);
  return {
    0xFF,
    (uint8_t)(red * 0xFF),
    (uint8_t)(red * 0xFF),
    0xFF
  };
}

color_t ColorHelper::primHealEffect(float timer, float invMaxTime) {
  float green = (0.5f + 0.5f * sinf(timer * 30.0f));
  green = 1.0f - (green * timer * invMaxTime);
  return {
    (uint8_t)(green * 0xFF),
    0xFF,
    (uint8_t)(green * 0xFF),
    0xFF
  };
}
