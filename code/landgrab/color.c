#include <libdragon.h>

color_t
color_between (color_t start_color, color_t end_color, float p)
{
  assert (p >= 0.0f && p <= 1.0f);
  return (color_t){ .r = (1.0f - p) * start_color.r + p * end_color.r + 0.5,
                    .g = (1.0f - p) * start_color.g + p * end_color.g + 0.5,
                    .b = (1.0f - p) * start_color.b + p * end_color.b + 0.5,
                    .a = (1.0f - p) * start_color.a + p * end_color.a + 0.5 };
}
