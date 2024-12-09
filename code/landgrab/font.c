#include "font.h"
#include "player.h"

rdpq_font_t *font_squarewave;

void
font_init (void)
{
  font_squarewave = rdpq_font_load ("rom:/squarewave.font64");
  rdpq_text_register_font (FONT_SQUAREWAVE, font_squarewave);

  PLAYER_FOREACH (i)
  {
    rdpq_font_style (font_squarewave, i,
                     &(rdpq_fontstyle_t){ .color = COLOR_WHITE, .outline_color = PLAYER_COLORS[i] });
  }

  rdpq_font_style (font_squarewave, STYLE_SQUAREWAVE_WHITE,
                   &(rdpq_fontstyle_t){ .color = COLOR_WHITE, .outline_color = COLOR_DARK_GRAY });
}

void
font_cleanup (void)
{
  rdpq_text_unregister_font (FONT_SQUAREWAVE);
  rdpq_font_free (font_squarewave);
}
