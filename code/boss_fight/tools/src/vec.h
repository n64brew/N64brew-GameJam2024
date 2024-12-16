/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <cstdint>
#include <cmath>

struct IVec3 {
  int16_t pos[3]{};
  operator int16_t*() { return pos; }
};