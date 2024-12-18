/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "debugDraw.h"
#include "../main.h"
#include <t3d/t3d.h>
#include <vector>

namespace
{
  struct Line {
    T3DVec3 a{};
    T3DVec3 b{};
    uint16_t color;
    uint16_t _padding;
  };

  std::vector<Line> lines{};

  sprite_t *font{};

  void debugDrawLine(uint16_t *fb, int px0, int py0, int px1, int py1, uint16_t color)
  {
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    if((px0 > width + 200) || (px1 > width + 200) ||
       (py0 > height + 200) || (py1 > height + 200)) {
      return;
    }

    float pos[2]{(float)px0, (float)py0};
    int dx = px1 - px0;
    int dy = py1 - py0;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    if(steps <= 0)return;
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;

    for(int i=0; i<steps; ++i)
    {
      if((i%3 != 0) && pos[1] >= 0 && pos[1] < height && pos[0] >= 0 && pos[0] < width) {
        fb[(int)pos[1] * width + (int)pos[0]] = color;
      }
      pos[0] += xInc;
      pos[1] += yInc;
    }
  }
}

void Debug::init() {
  font = sprite_load(FS_BASE_PATH "font.ia4.sprite");
  lines = {};
}

void Debug::destroy() {
  lines.clear();
  lines.shrink_to_fit();
  sprite_free(font);
}

void Debug::drawLine(const T3DVec3 &a, const T3DVec3 &b, color_t color) {
  lines.push_back({a, b, color_to_packed16(color)});
}

void Debug::drawSphere(const T3DVec3 &center, float radius, color_t color) {

  int steps = 12;
  float step = 2.0f * M_PI / steps;
  T3DVec3 last = center + T3DVec3{radius, 0, 0};
  for(int i=1; i<=steps; ++i) {
    float angle = i * step;
    T3DVec3 next = center + T3DVec3{radius * fm_cosf(angle), 0, radius * fm_sinf(angle)};
    drawLine(last, next, color);
    last = next;
  }
  last = center + T3DVec3{0, radius, 0};
  for(int i=1; i<=steps; ++i) {
    float angle = i * step;
    T3DVec3 next = center + T3DVec3{0, radius * fm_cosf(angle), radius * fm_sinf(angle)};
    drawLine(last, next, color);
    last = next;
  }
  last = center + T3DVec3{0, 0, radius};
  for(int i=1; i<=steps; ++i) {
    float angle = i * step;
    T3DVec3 next = center + T3DVec3{radius * fm_cosf(angle), radius * fm_sinf(angle), 0};
    drawLine(last, next, color);
    last = next;
  }
}

void Debug::draw(uint16_t *fb) {
  if(lines.empty())return;
  debugf("Drawing %d lines\n", lines.size());
  rspq_wait();

  for(auto &line : lines) {
    t3d_viewport_calc_viewspace_pos(nullptr, &line.a, &line.a);
    t3d_viewport_calc_viewspace_pos(nullptr, &line.b, &line.b);
  }

  float maxX = SCREEN_WIDTH;
  float maxY = SCREEN_HEIGHT;
  for(auto &line : lines) {
    if(line.a.x < 0 && line.b.x < 0)continue;
    if(line.a.y < 0 && line.b.y < 0)continue;
    if(line.a.x > maxX && line.b.x > maxX)continue;
    if(line.a.y > maxY && line.b.y > maxY)continue;
    debugDrawLine(fb, line.a.x, line.a.y, line.b.x, line.b.y, line.color);
  }

  lines.clear();
  lines.shrink_to_fit();
}

void Debug::printStart() {
  rdpq_sync_pipe();
  rdpq_sync_tile();
  rdpq_sync_load();

  rdpq_set_mode_standard();
  rdpq_mode_antialias(AA_NONE);
  rdpq_mode_combiner(RDPQ_COMBINER1((TEX0,0,PRIM,0), (TEX0,0,PRIM,0)));
  rdpq_mode_alphacompare(1);
  rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));

  rdpq_sprite_upload(TILE0, font, NULL);
}

float Debug::print(float x, float y, const char *str) {
  int width = 8;
  int height = 8;
  int s = 0;

  while(*str) {
    uint8_t c = *str;
    if(c != ' ' && c != '\n')
    {
      if(c >= 'a' && c <= 'z')c &= ~0x20;
      s = (c - 33) * width;
      rdpq_texture_rectangle_raw(TILE0, x, y, x+width, y+height, s, 0, 1, 1);
    }
    ++str;
    x += 7;
  }
  return x;
}

float Debug::printf(float x, float y, const char *fmt, ...) {
  char buffer[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 128, fmt, args);
  va_end(args);
  return Debug::print(x, y, buffer);
}
