#include "font.h"
#include "player.h"

rdpq_font_t *font_squarewave;

void
font_init (void)
{
  font_squarewave = rdpq_font_load ("rom:/squarewave.font64");
  rdpq_text_register_font (FONT_SQUAREWAVE, font_squarewave);
}

void
font_cleanup (void)
{
  rdpq_text_unregister_font (FONT_SQUAREWAVE);
  rdpq_font_free (font_squarewave);
}
