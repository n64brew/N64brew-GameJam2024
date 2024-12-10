#include "scoreboard.h"
#include "color.h"
#include "font.h"

PlayerScore scoreboard[MAXPLAYERS];
int winners;

static sprite_t *star_sprite;

void
scoreboard_init (void)
{
  star_sprite = sprite_load ("rom:/landgrab/star.ia8.sprite");
}

void
scoreboard_cleanup (void)
{
  sprite_free (star_sprite);
}

int
scoreboard_compare (const void *a, const void *b)
{
  return (((PlayerScore *)b)->score - ((PlayerScore *)a)->score);
}

void
scoreboard_calculate (bool game_over)
{
  PLAYER_FOREACH (p)
  {
    scoreboard[p].p = p;
    scoreboard[p].score = player_score (&players[p]);
  }

  qsort (scoreboard, MAXPLAYERS, sizeof (PlayerScore), scoreboard_compare);

  if (game_over)
    {
      winners = 1;
      for (int i = 1; i < MAXPLAYERS; i++)
        {
          if (scoreboard[i].score == scoreboard[0].score)
            {
              winners++;
            }
        }

      if (winners < MAXPLAYERS)
        {
          for (int i = 0; i < winners; i++)
            {
              core_set_winner (scoreboard[i].p);
            }
        }
    }
}

void
scoreboard_scores_render (void)
{
  const int x = 259;
  int y = 35;
  rdpq_textparms_t textparms
      = { .width = 50, .align = ALIGN_CENTER, .style_id = FONT_STYLE_WHITE };

  rdpq_set_mode_standard ();

  rdpq_mode_push ();
  {
    rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color (BOARD_COLOR);
    rdpq_fill_rectangle (259, 15, 309, 221);
    rdpq_fill_rectangle (259, 15, 309, 50);
  }
  rdpq_mode_pop ();

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, x, y, "SCORE");
  y += 45;

  for (size_t i = 0; i < MAXPLAYERS; i++)
    {
      PlyNum p = scoreboard[i].p;
      int score = scoreboard[i].score;

      rdpq_text_printf (&textparms, FONT_ANITA, x, y, "^%02X%d", p, score);

      y += 40;
    }
}

void
scoreboard_star_render (PlyNum p, int x, int y)
{
  rdpq_mode_push ();
  {
    rdpq_set_mode_standard ();
    rdpq_mode_filter (FILTER_BILINEAR);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner (
        RDPQ_COMBINER1 ((PRIM, ENV, TEX0, ENV), (0, 0, 0, TEX0)));
    rdpq_set_prim_color (PLAYER_COLORS[p]); // fill color
    rdpq_set_env_color (PLAYER_COLORS[p]);  // outline color
    rdpq_sprite_blit (
        star_sprite, x, y,
        &(rdpq_blitparms_t){ .scale_x = 0.67f, .scale_y = 0.67f });
  }
  rdpq_mode_pop ();
}

void
scoreboard_pieces_render (void)
{
  rdpq_set_mode_standard ();

  rdpq_mode_push ();
  {
    rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color (BOARD_COLOR);
    rdpq_fill_rectangle (15, 15, 64, 221);
    rdpq_fill_rectangle (15, 15, 64, 50);
  }
  rdpq_mode_pop ();

  const int x = 16;
  int y = 30;
  rdpq_textparms_t heading_parms = { .width = 50,
                                     .align = ALIGN_CENTER,
                                     .style_id = FONT_STYLE_WHITE,
                                     .line_spacing = -1,
                                     .wrap = WRAP_WORD };

  rdpq_textparms_t digit_parms = { .width = 50,
                                   .align = ALIGN_CENTER,
                                   .style_id = FONT_STYLE_WHITE,
                                   .char_spacing = 1 };

  rdpq_text_print (&heading_parms, FONT_SQUAREWAVE, x, y, "PIECES\nLEFT");
  y += 50;

  PLAYER_FOREACH (p)
  {
    int n = players[p].pieces_left;
    if (players[p].monomino_final_piece)
      {
        scoreboard_star_render (p, x + 4, y - 27);
      }

    rdpq_text_printf (&digit_parms, FONT_ANITA, x, y, "^%02X%d", p, n);
    y += 40;
  }
}
