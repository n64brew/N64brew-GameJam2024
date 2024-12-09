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

typedef enum
{
  MINIGAME_STATE_PLAY = 0,
  MINIGAME_STATE_PAUSE,
  MINIGAME_STATE_END
} MinigameState;

MinigameState minigame_state = MINIGAME_STATE_PLAY;
Player players[MAXPLAYERS];
size_t turn_count = 0;

#define BACKGROUND_COLOR RGBA32 (50, 20, 20, 255)

PlayerTurnResult last_active_turn[MAXPLAYERS];

static const char *TURN_MESSAGES[] = {
  FMT_SQUAREWAVE_P1 "Player 1's Turn!",
  FMT_SQUAREWAVE_P2 "Player 2's Turn!",
  FMT_SQUAREWAVE_P3 "Player 3's Turn!",
  FMT_SQUAREWAVE_P4 "Player 4's Turn!",
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
minigame_play_render (void)
{
  PlyNum active_player = turn_count % MAXPLAYERS;

  // Attach and clear the screen
  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();

  board_render ();

  PLAYER_FOREACH (p)
  {
    if (p != active_player)
      {
        player_render (&players[p], false);
      }
  }

  player_render (&players[active_player], true);

  const int TURN_BOX_TOP = BOARD_TOP - 12;
  const int HINT_BOX_BTM = BOARD_BOTTOM + 12;

  rdpq_set_mode_fill (COLOR_BLACK);
  rdpq_fill_rectangle (BOARD_LEFT, TURN_BOX_TOP, BOARD_RIGHT, BOARD_TOP);
  rdpq_fill_rectangle (BOARD_LEFT, BOARD_BOTTOM, BOARD_RIGHT, HINT_BOX_BTM);

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = STYLE_SQUAREWAVE_WHITE };

  const int TURN_MSG_Y = TURN_BOX_TOP + 9;
  const int HINT_MSG_Y = HINT_BOX_BTM - 4;

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, TURN_MSG_Y,
                   TURN_MESSAGES[active_player]);

  // TODO: Generalize this for the current context
  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, HINT_MSG_Y,
                   "Place a piece touching a corner!");

  rdpq_detach_show ();
}

void
minigame_play_loop (float deltatime)
{
  const PlyNum active_player = turn_count % MAXPLAYERS;
  bool turn_ended = false;

  PLAYER_FOREACH (p)
  {
    const bool player_active = p == active_player;

    PlayerTurnResult player_loop_result
        = p < core_get_playercount ()
              ? player_loop (&players[p], player_active, deltatime)
              : player_loop_ai (&players[p], player_active, deltatime);

    if (player_loop_result == PLAYER_TURN_PAUSE)
      {
        minigame_state = MINIGAME_STATE_PAUSE;
        break;
      }
    // Only the active player can end the turn
    if (player_active)
      {
        turn_ended = player_loop_result == PLAYER_TURN_END
                     || player_loop_result == PLAYER_TURN_PASS;
        last_active_turn[p] = player_loop_result;
      }
  }

  minigame_play_render ();

  // Wait until after rendering to "end the turn" so the UI is consistent.
  if (turn_ended)
    {
      turn_count++;

      bool all_players_passed = true;
      PLAYER_FOREACH (p)
      {
        if (last_active_turn[p] != PLAYER_TURN_PASS)
          {
            all_players_passed = false;
            break;
          }
      }
      if (all_players_passed)
        {
          minigame_state = MINIGAME_STATE_END;
        }
    }
}

void
minigame_pause_loop (float deltatime)
{
  PLAYER_FOREACH (p)
  {
    if (p < core_get_playercount ())
      {
        joypad_port_t port = core_get_playercontroller (p);
        joypad_buttons_t pressed = joypad_get_buttons_pressed (port);
        joypad_buttons_t held = joypad_get_buttons_held (port);

        if (pressed.start)
          {
            if (held.l && held.r)
              {
                minigame_end();
              }
            else
              {
                minigame_state = MINIGAME_STATE_PLAY;
              }
          }
      }
  }

  // Attach and clear the screen
  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();

  board_render ();

  PLAYER_FOREACH (p) { player_render (&players[p], false); }

  const int TURN_BOX_TOP = BOARD_TOP - 12;
  const int HINT_BOX_BTM = BOARD_BOTTOM + 12;

  rdpq_set_mode_fill (COLOR_BLACK);
  rdpq_fill_rectangle (BOARD_LEFT, TURN_BOX_TOP, BOARD_RIGHT, BOARD_TOP);
  rdpq_fill_rectangle (BOARD_LEFT, BOARD_BOTTOM, BOARD_RIGHT, HINT_BOX_BTM);

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = STYLE_SQUAREWAVE_WHITE };

  const int TURN_MSG_Y = TURN_BOX_TOP + 9;
  const int HINT_MSG_Y = HINT_BOX_BTM - 4;

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, TURN_MSG_Y,
                   "Game Paused");
  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, HINT_MSG_Y,
                   "Press L + R + Start to exit");

  rdpq_detach_show ();
}

void
minigame_end_loop (float deltatime)
{
  // Attach and clear the screen
  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();

  board_render ();

  rdpq_detach_show ();
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void
minigame_loop (float deltatime)
{
  switch (minigame_state)
    {
    case MINIGAME_STATE_PLAY:
      minigame_play_loop (deltatime);
      break;

    case MINIGAME_STATE_PAUSE:
      minigame_pause_loop (deltatime);
      break;

    case MINIGAME_STATE_END:
      minigame_end_loop (deltatime);
      break;
    }
}
