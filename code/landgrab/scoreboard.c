#include "scoreboard.h"
#include "board.h"
#include "color.h"
#include "font.h"
#include "minigame.h"

static sprite_t *star_sprite;

typedef enum
{
  ICON_A,
  ICON_B,
  ICON_C_DOWN,
  ICON_C_LEFT,
  ICON_C_RIGHT,
  ICON_C_UP,
  ICON_D_DOWN,
  ICON_D_LEFT,
  ICON_D_RIGHT,
  ICON_D_UP,
  ICON_DPAD,
  ICON_L,
  ICON_R,
  ICON_START,
  ICON_STICK,
  ICON_Z,
} Icon;

static const char *icon_paths[] = {
  "rom:/landgrab/joypad_a.rgba16.sprite",
  "rom:/landgrab/joypad_b.rgba16.sprite",
  "rom:/landgrab/joypad_c_down.rgba16.sprite",
  "rom:/landgrab/joypad_c_left.rgba16.sprite",
  "rom:/landgrab/joypad_c_right.rgba16.sprite",
  "rom:/landgrab/joypad_c_up.rgba16.sprite",
  "rom:/landgrab/joypad_d_down.rgba16.sprite",
  "rom:/landgrab/joypad_d_left.rgba16.sprite",
  "rom:/landgrab/joypad_d_right.rgba16.sprite",
  "rom:/landgrab/joypad_d_up.rgba16.sprite",
  "rom:/landgrab/joypad_dpad.rgba16.sprite",
  "rom:/landgrab/joypad_l.rgba16.sprite",
  "rom:/landgrab/joypad_r.rgba16.sprite",
  "rom:/landgrab/joypad_start.rgba16.sprite",
  "rom:/landgrab/joypad_stick.rgba16.sprite",
  "rom:/landgrab/joypad_z.rgba16.sprite",
};

static sprite_t *icons[ARRAY_SIZE (icon_paths)];

static void
scoreboard_icon_render (Icon icon, int x, int y)
{
  rdpq_mode_push ();
  {
    rdpq_set_mode_standard ();
    rdpq_mode_filter (FILTER_BILINEAR);
    rdpq_mode_alphacompare (1);
    rdpq_sprite_blit (icons[icon], x, y, NULL);
  }
  rdpq_mode_pop ();
}

static void
scoreboard_star_render (PlyNum p, int x, int y)
{
  rdpq_mode_push ();
  {
    // Draw the star using the player's color
    rdpq_mode_filter (FILTER_BILINEAR);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner (RDPQ_COMBINER1 ((0, ENV, TEX0, ENV), (0, 0, 0, TEX0)));
    rdpq_set_env_color (PLAYER_COLORS[p]);
    rdpq_sprite_blit (star_sprite, x, y, NULL);
  }
  rdpq_mode_pop ();
}

void
scoreboard_init (void)
{
  star_sprite = sprite_load ("rom:/landgrab/star.ia8.sprite");

  for (size_t i = 0; i < ARRAY_SIZE (icon_paths); i++)
    {
      icons[i] = sprite_load (icon_paths[i]);
    }
}

void
scoreboard_cleanup (void)
{
  sprite_free (star_sprite);

  for (size_t i = 0; i < ARRAY_SIZE (icon_paths); i++)
    {
      sprite_free (icons[i]);
    }
}

void
scoreboard_scores_render (void)
{
  const int x = 259;
  int y = 35;
  rdpq_textparms_t textparms
      = { .width = 50, .align = ALIGN_CENTER, .style_id = FONT_STYLE_WHITE };

  rdpq_mode_push ();
  {
    rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color (BOARD_COLOR);
    // Draw the box for the scores
    rdpq_fill_rectangle (259, 15, 309, 221);
    // Draw the header in a darker shade
    rdpq_fill_rectangle (259, 15, 309, 50);
  }
  rdpq_mode_pop ();

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, x, y, "SCORE");
  y += 45;

  PLAYER_FOREACH (p)
  {
    int score = players[p].score;
    rdpq_text_printf (&textparms, FONT_ANITA, x, y, "^%02X%d", p, score);
    y += 40;
  }
}

void
scoreboard_pieces_render (void)
{
  rdpq_mode_push ();
  {
    // Blend the scoreboard with the background
    rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color (BOARD_COLOR);
    // Draw the box for the pieces scoreboard
    rdpq_fill_rectangle (15, 15, 64, 221);
    // Draw the header in a darker shade
    rdpq_fill_rectangle (15, 15, 64, 50);
  }
  rdpq_mode_pop ();

  rdpq_textparms_t heading_parms = { .width = 50,
                                     .align = ALIGN_CENTER,
                                     .style_id = FONT_STYLE_WHITE,
                                     .line_spacing = -1,
                                     .wrap = WRAP_WORD };

  rdpq_textparms_t digit_parms = { .width = 50,
                                   .align = ALIGN_CENTER,
                                   .style_id = FONT_STYLE_WHITE,
                                   .char_spacing = 1 };

  const int x = 16;
  int y = 30;

  rdpq_text_print (&heading_parms, FONT_SQUAREWAVE, x, y, "PIECES\nLEFT");
  y += 50;

  PLAYER_FOREACH (p)
  {
    int n = players[p].pieces_left;

    if (players[p].monomino_final_piece)
      {
        scoreboard_star_render (p, x + 4, y - 27);
      }

    digit_parms.style_id = p;
    rdpq_text_printf (&digit_parms, FONT_ANITA, x, y, "%d", n);
    y += 40;
  }
}

void
scoreboard_controls_render (void)
{
  rdpq_mode_push ();
  {
    // Blend with the background
    rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color (BOARD_COLOR);

    rdpq_fill_rectangle (15, 15, 64, 221);
    rdpq_fill_rectangle (259, 15, 309, 221);
  }
  rdpq_mode_pop ();

  const int icon_drop = 24;
  const int text_drop = 24;

  int x, y, icon_center_x, icon_left_x, icon_right_x;
  rdpq_textparms_t text_parms = { .width = 50,
                                  .align = ALIGN_CENTER,
                                  .style_id = FONT_STYLE_WHITE,
                                  .line_spacing = -3,
                                  .wrap = WRAP_WORD };

  x = 16;
  y = 30;

  icon_center_x = x + (50 / 2) - (12 / 2);
  icon_left_x = x + 12;
  icon_right_x = icon_left_x + 16;

  scoreboard_icon_render (ICON_STICK, icon_left_x, y);
  scoreboard_icon_render (ICON_DPAD, icon_right_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Move Cursor");
  y += text_drop;

  scoreboard_icon_render (ICON_A, icon_center_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Place Piece");
  y += text_drop;

  scoreboard_icon_render (ICON_B, icon_center_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Skip Turn");
  y += text_drop;

  scoreboard_icon_render (ICON_START, icon_center_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Pause Game");
  y += text_drop;

  x = 259;
  y = 30;

  icon_center_x = x + (50 / 2) - (12 / 2);
  icon_left_x = x + 12;
  icon_right_x = icon_left_x + 16;

  scoreboard_icon_render (ICON_L, icon_left_x, y);
  scoreboard_icon_render (ICON_Z, icon_right_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Mirror Piece");
  y += text_drop;

  scoreboard_icon_render (ICON_R, icon_center_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Flip Piece");
  y += text_drop;

  scoreboard_icon_render (ICON_C_LEFT, icon_left_x, y);
  scoreboard_icon_render (ICON_C_RIGHT, icon_right_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Change Piece");
  y += text_drop;

  scoreboard_icon_render (ICON_C_UP, icon_left_x, y);
  scoreboard_icon_render (ICON_C_DOWN, icon_right_x, y);
  y += icon_drop;
  rdpq_text_print (&text_parms, FONT_SQUAREWAVE, x, y, "Change Value");
  y += text_drop;
}
