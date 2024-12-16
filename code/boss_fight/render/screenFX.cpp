/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "screenFX.h"
#include "../main.h"
#include <libdragon.h>

void FX::drawNGonOverlay(int sides, const T3DVec3 &pos, float radiusInner, float radiusOuter, float angle, color_t color)
{
  rdpq_set_mode_standard();
  rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
  rdpq_set_prim_color(color);

  T3DVec3 lastInner, lastOuter;
  float angleSection = (T3D_DEG_TO_RAD(360.0f) / (float)sides);

  for(int i=0; i<=sides; ++i) {
    T3DVec3 dirVec{fm_cosf(angle), fm_sinf(angle), 0};
    auto inner = pos + dirVec * radiusInner;
    auto outer = pos + dirVec * radiusOuter;

    if(i > 0) {
      rdpq_triangle(&TRIFMT_FILL, lastInner.v, outer.v, inner.v);
      rdpq_triangle(&TRIFMT_FILL, lastInner.v, outer.v, lastOuter.v);
    }

    lastInner = inner;
    lastOuter = outer;
    angle += angleSection;
  }
}

void FX::drawBars(float height) {
  if(height <= 0) return;
  rdpq_mode_push();
  rdpq_set_mode_fill({});
  rdpq_fill_rectangle(0, 0, SCREEN_WIDTH, height);
  rdpq_fill_rectangle(0, SCREEN_HEIGHT - height, SCREEN_WIDTH, SCREEN_HEIGHT);
  rdpq_mode_pop();
}
