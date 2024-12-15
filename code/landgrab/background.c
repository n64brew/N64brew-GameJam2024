#include "background.h"
#include "global.h"

static sprite_t *bg_sprite = NULL;
static rspq_block_t *bg_block = NULL;

void
background_init (void)
{
  bg_sprite = sprite_load ("rom:/landgrab/background.ci8.sprite");

  // Create a block for the background, so that we can replay it later.
  rspq_block_begin ();

  rdpq_mode_push ();
  rdpq_set_mode_standard ();
  rdpq_sprite_upload (TILE0, bg_sprite,
                      &(rdpq_texparms_t){
                          .s = { .repeats = REPEAT_INFINITE },
                          .t = { .repeats = REPEAT_INFINITE },
                      });
  rdpq_texture_rectangle (TILE0, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, 0);
  rdpq_mode_pop ();

  bg_block = rspq_block_end ();
}

void
background_cleanup (void)
{
  rspq_block_free (bg_block);
  bg_block = NULL;

  sprite_free (bg_sprite);
  bg_sprite = NULL;
}

void
background_render (void)
{
  rspq_block_run (bg_block);
}
