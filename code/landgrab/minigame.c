#include <libdragon.h>

#include "../../minigame.h"

#include "background.h"
#include "board.h"
#include "font.h"
#include "global.h"
#include "player.h"

const MinigameDef minigame_def
    = { .gamename = "Land Grab",
        .developername = "Meeq Corporation",
        .description = "Claim as much land as you can!",
        .instructions = "Place pieces at diagonals to claim land. "
                        "The player with the most land when the first player "
                        "runs out of pieces wins!" };

Player players[MAXPLAYERS];
size_t turn_count = 0;

#define BACKGROUND_COLOR RGBA32 (50, 20, 20, 255)

static const char *TURN_MESSAGES[] = {
  "Player 1's Turn!",
  "Player 2's Turn!",
  "Player 3's Turn!",
  "Player 4's Turn!",
};

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

  background_init ();
  board_init ();

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

  board_cleanup ();
  background_cleanup ();

  font_cleanup ();

  rdpq_debug_stop ();
  rdpq_close ();

  display_close ();
}

void
minigame_render (void)
{
  PlyNum active_player = turn_count % MAXPLAYERS;

  // Attach and clear the screen
  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();

  board_render ();

  PLAYER_FOREACH (i) { player_render (&players[i], i == active_player); }

  const int TURN_BOX_TOP = BOARD_TOP - 12;
  const int HINT_BOX_BTM = BOARD_BOTTOM + 12;

  rdpq_set_mode_fill (COLOR_BLACK);
  rdpq_fill_rectangle (BOARD_LEFT, TURN_BOX_TOP, BOARD_RIGHT, BOARD_TOP);
  rdpq_fill_rectangle (BOARD_LEFT, BOARD_BOTTOM, BOARD_RIGHT, HINT_BOX_BTM);

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms
      = { .width = BOARD_RIGHT - BOARD_LEFT, .align = ALIGN_CENTER };

  const int TURN_MSG_Y = TURN_BOX_TOP + 9;
  const int HINT_MSG_Y = HINT_BOX_BTM - 4;

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, TURN_MSG_Y,
                   TURN_MESSAGES[active_player]);

  // TODO: Generalize this for the current context
  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, HINT_MSG_Y,
                   "Place a piece touching a corner!");

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
  PlyNum active_player = turn_count % MAXPLAYERS;

  PLAYER_FOREACH (i)
  {
    if (i < core_get_playercount ())
      {
        player_loop (&players[i], i == active_player);
      }
    else
      {
        player_loop_ai (&players[i], i == active_player);
      }
  }

  minigame_render ();
}
