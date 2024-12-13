/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

#include <rspq_constants.h>
#if RSPQ_PROFILE
  #include <rspq_profile.h>
#endif

extern "C" {
  #include "../../core.h"
  #include "../../minigame.h"
}

#define FS_BASE_PATH "rom:/boss_fight/"

struct InputState {
  T3DVec3 move{};
  bool jump{};
  bool jumpHold{};
  bool attack{};
};

consteval uint32_t operator"" _u32(const char *str, size_t len) {
  uint32_t result = 0;
  for (size_t i = 0; i < len; i++) {
    result |= ((uint32_t)str[i] << (8 * (3-i)));
  }
  return result;
}

static_assert("abcd"_u32 == 0x61626364);
static_assert("abc"_u32  == 0x61626300);

consteval uint64_t operator"" _u64(const char *str, size_t len) {
  uint64_t result = 0;
  for (size_t i = 0; i < len; i++) {
    result |= ((uint64_t)str[i] << (8 * (7-i)));
  }
  return result;
}

static_assert("abcd0123"_u64 == 0x61626364'30313233);
static_assert("abcd012"_u64 == 0x61626364'30313200);
static_assert("abc"_u64  == 0x61626300'00000000);

constexpr float COLL_WORLD_SCALE = 16.0f;
constexpr uint32_t SCREEN_WIDTH = 320;
constexpr uint32_t SCREEN_HEIGHT = 240;

constexpr color_t PLAYER_COLORS[4] = {
  {0xAA, 0x44, 0x44, 0xFF},
  {0x44, 0xAA, 0x44, 0xFF},
  {0x44, 0x44, 0xAA, 0xFF},
  {0xAA, 0xAA, 0x44, 0xFF},
};