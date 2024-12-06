#include <libdragon.h>

#include "../../minigame.h"

#include "board.h"
#include "constants.h"
#include "font.h"
#include "player.h"

const MinigameDef minigame_def
    = { .gamename = "Land Grab",
        .developername = "Meeq Corporation",
        .description = "Claim as much land as you can!",
        .instructions = "Place pieces at diagonals to claim land. "
                        "The player with the most land when the first player "
                        "runs out of pieces wins!" };

BoardState board;
PlayerState players[MAXPLAYERS];
PlyNum player_turn = PLAYER_1;

#define BACKGROUND_COLOR RGBA32 (50, 20, 20, 255)

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void
minigame_init (void)
{
  display_init (DISPLAY_RESOLUTION, DISPLAY_BITDEPTH, DISPLAY_BUFFERS,
                DISPLAY_GAMMA, DISPLAY_FILTERS);

  rdpq_init ();
  rdpq_debug_start ();

  font_init ();

  board_init (&board);

  PLAYER_FOREACH (i) { player_init (&players[i], i); }
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void
minigame_cleanup (void)
{
  PLAYER_FOREACH (i) { player_cleanup (&players[i]); }

  board_cleanup (&board);

  font_cleanup ();

  rdpq_debug_stop ();
  rdpq_close ();

  display_close ();
}

void
minigame_draw (void)
{
  // Attach and clear the screen
  surface_t *disp = display_get ();
  rdpq_attach_clear (disp, NULL);

  rdpq_set_mode_fill (PLAYER_COLORS[player_turn]);
  rdpq_fill_rectangle (0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

  board_draw (&board);

  PLAYER_FOREACH (i) { player_draw (&players[i]); }

  rdpq_detach_show ();
}

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/
void
minigame_fixedloop (float deltatime)
{
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void
minigame_loop (float deltatime)
{
  minigame_draw ();
}
