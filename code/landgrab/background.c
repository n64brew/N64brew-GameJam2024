#include "background.h"

static sprite_t *tiles_sprite = NULL;
static rspq_block_t *tiles_block = NULL;

void
background_init (void)
{
  tiles_sprite = sprite_load ("rom:/landgrab/background.ci8.sprite");

  // Create a block for the background, so that we can replay it later.
  rspq_block_begin ();

  rdpq_mode_push ();
  rdpq_set_mode_standard();
  rdpq_sprite_upload (TILE0, tiles_sprite,
                      &(rdpq_texparms_t){
                          .s = { .repeats = REPEAT_INFINITE },
                          .t = { .repeats = REPEAT_INFINITE },
                      });
  rdpq_texture_rectangle (TILE0, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, 0);
  rdpq_mode_pop ();

  tiles_block = rspq_block_end ();
}

void
background_cleanup (void)
{

  if (tiles_block != NULL)
    {
      rspq_block_free (tiles_block);
      tiles_block = NULL;
    }
  if (tiles_sprite != NULL)
    {
      sprite_free (tiles_sprite);
      tiles_sprite = NULL;
    }
}

void
background_render (void)
{
  rspq_block_run (tiles_block);
}
