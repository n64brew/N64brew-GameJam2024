#include "color.h"
#include "font.h"
#include "global.h"
#include "minigame.h"

#define LOGO_HOLD_DURATION 4.0f
#define LOGO_FADE_DURATION 2.0f

static sprite_t *logo_sprite;
static float logo_hold_timer;
static float logo_fade_timer;
static float logo_alpha;

void
logo_init (void)
{
  logo_sprite = sprite_load ("rom:/landgrab/logo.ci8.sprite");
  logo_hold_timer = LOGO_HOLD_DURATION;
  logo_fade_timer = LOGO_FADE_DURATION;
}

void
logo_cleanup (void)
{
  sprite_free (logo_sprite);
}

void
logo_loop (float deltatime)
{
  if (logo_hold_timer > 0.0f)
    {
      logo_hold_timer -= deltatime;
      logo_alpha = 1.0f;
    }
  else if (logo_fade_timer > 0.0f)
    {
      logo_fade_timer -= deltatime;
      logo_alpha = logo_fade_timer / LOGO_FADE_DURATION;
    }
  else
    {
      logo_alpha = 0.0f;
    }
}

void
logo_render (void)
{
  if (logo_alpha > 0)
    {
      const int x0 = (DISPLAY_WIDTH - logo_sprite->width) / 2;
      const int y0 = (DISPLAY_HEIGHT - logo_sprite->height) / 2;

      rdpq_mode_push ();
      {
        rdpq_set_mode_standard ();
        rdpq_mode_alphacompare (1);
        rdpq_set_fog_color (RGBA32 (0, 0, 0, logo_alpha * 255));
        rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY_CONST);
        rdpq_sprite_blit (logo_sprite, x0, y0, NULL);
      }
      rdpq_mode_pop ();
    }
}
