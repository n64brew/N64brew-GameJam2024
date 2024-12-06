#include "font.h"
#include "player.h"

rdpq_font_t *font_billboard;

void
font_init (void)
{
  font_billboard = rdpq_font_load ("rom:/squarewave.font64");
  rdpq_text_register_font (FONT_BILLBOARD, font_billboard);
  for (size_t i = 0; i < MAXPLAYERS; i++)
    {
      rdpq_font_style (font_billboard, i,
                       &(rdpq_fontstyle_t){ .color = PLAYER_COLORS[i] });
    }
}

void
font_cleanup (void)
{
  rdpq_text_unregister_font (FONT_BILLBOARD);
  rdpq_font_free (font_billboard);
}
