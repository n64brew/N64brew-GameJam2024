#include "font.h"
#include "color.h"
#include "player.h"

rdpq_font_t *font_squarewave;
rdpq_font_t *font_anita;
rdpq_font_t *font_bbrick;

void
font_init (void)
{
  font_squarewave = rdpq_font_load ("rom:/squarewave.font64");
  rdpq_text_register_font (FONT_SQUAREWAVE, font_squarewave);

  font_anita = rdpq_font_load ("rom:/landgrab/anita_semi_square.font64");
  rdpq_text_register_font (FONT_ANITA, font_anita);

  font_bbrick = rdpq_font_load ("rom:/landgrab/bbrick.font64");
  rdpq_text_register_font (FONT_BBRICK, font_bbrick);

  PLAYER_FOREACH (p)
  {
    rdpq_fontstyle_t player_style
        = { .color = p % 2 == 1 ? COLOR_BLACK : COLOR_WHITE,
            .outline_color = PLAYER_COLORS[p] };
    rdpq_font_style (font_squarewave, p, &player_style);
    rdpq_font_style (font_anita, p, &player_style);
    rdpq_font_style (font_bbrick, p, &player_style);
  }

  rdpq_fontstyle_t style
      = { .color = COLOR_WHITE, .outline_color = COLOR_DARK_GRAY };

  rdpq_font_style (font_squarewave, FONT_STYLE_WHITE, &style);
  rdpq_font_style (font_anita, FONT_STYLE_WHITE, &style);
  rdpq_font_style (font_bbrick, FONT_STYLE_WHITE, &style);
}

void
font_cleanup (void)
{
  rdpq_text_unregister_font (FONT_BBRICK);
  rdpq_font_free (font_bbrick);

  rdpq_text_unregister_font (FONT_ANITA);
  rdpq_font_free (font_anita);

  rdpq_text_unregister_font (FONT_SQUAREWAVE);
  rdpq_font_free (font_squarewave);
}
