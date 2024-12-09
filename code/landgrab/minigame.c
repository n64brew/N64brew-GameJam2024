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

#define PAUSE_INPUT_DELAY 0.5f
#define END_INPUT_DELAY 2.0f

#define MUSIC_PLAY "rom:/landgrab/15yearsb.xm64"
#define MUSIC_END "rom:/landgrab/phekkis-4_weeks_of_hysteria.xm64"

typedef enum
{
  MINIGAME_STATE_INIT = 0,
  MINIGAME_STATE_PLAY,
  MINIGAME_STATE_PAUSE,
  MINIGAME_STATE_END
} MinigameState;

MinigameState minigame_state;
Player players[MAXPLAYERS];
size_t turn_count;
xm64player_t music;

float menu_input_delay;

PlayerTurnResult last_active_turn[MAXPLAYERS];

static const char *TURN_MESSAGES[] = {
  FMT_SQUAREWAVE_P1 "Player 1's Turn!",
  FMT_SQUAREWAVE_P2 "Player 2's Turn!",
  FMT_SQUAREWAVE_P3 "Player 3's Turn!",
  FMT_SQUAREWAVE_P4 "Player 4's Turn!",
};

static void
minigame_set_state (MinigameState new_state)
{
  MinigameState old_state = minigame_state;

  if (old_state == MINIGAME_STATE_INIT && new_state == MINIGAME_STATE_PLAY)
    {
      xm64player_open (&music, MUSIC_PLAY);
      xm64player_set_loop (&music, true);
      xm64player_set_vol (&music, 0.5f);
      xm64player_play (&music, 0);
    }

  if (old_state == MINIGAME_STATE_PLAY && new_state == MINIGAME_STATE_PAUSE)
    {
      xm64player_stop (&music);
    }

  if (old_state == MINIGAME_STATE_PAUSE && new_state == MINIGAME_STATE_PLAY)
    {
      xm64player_play (&music, 0);
    }

  if (old_state == MINIGAME_STATE_PLAY && new_state == MINIGAME_STATE_END)
    {
      xm64player_close (&music);
      xm64player_open (&music, MUSIC_END);
      xm64player_set_vol (&music, 1.0f);
      xm64player_play (&music, 0);
    }

  if (new_state == MINIGAME_STATE_PAUSE)
    {
      menu_input_delay = PAUSE_INPUT_DELAY;
    }
  else if (new_state == MINIGAME_STATE_END)
    {
      menu_input_delay = END_INPUT_DELAY;
    }

  minigame_state = new_state;
}

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void
minigame_init (void)
{
  display_init (RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE,
                FILTERS_RESAMPLE_ANTIALIAS);

  rdpq_init ();
  rdpq_debug_start ();

  font_init ();

  background_init ();
  board_init ();

  PLAYER_FOREACH (p) { player_init (&players[p], p); }

  turn_count = 0;
  minigame_set_state (MINIGAME_STATE_PLAY);
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void
minigame_cleanup (void)
{
  xm64player_stop (&music);
  xm64player_close (&music);

  PLAYER_FOREACH (i) { player_cleanup (&players[i]); }

  board_cleanup ();
  background_cleanup ();

  font_cleanup ();

  rdpq_debug_stop ();

  display_close ();
}

static void
minigame_upper_msg_render (const char *msg)
{
  const int UPPER_BOX_TOP = BOARD_TOP - 12;
  const int UPPER_MSG_Y = UPPER_BOX_TOP + 9;

  rdpq_set_mode_fill (COLOR_BLACK);
  rdpq_fill_rectangle (BOARD_LEFT, UPPER_BOX_TOP, BOARD_RIGHT, BOARD_TOP);

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = STYLE_SQUAREWAVE_WHITE };

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, UPPER_MSG_Y, msg);
}

static void
minigame_lower_msg_render (const char *msg)
{
  const int LOWER_BOX_BTM = BOARD_BOTTOM + 12;
  const int LOWER_MSG_Y = LOWER_BOX_BTM - 4;

  rdpq_set_mode_fill (COLOR_BLACK);
  rdpq_fill_rectangle (BOARD_LEFT, BOARD_BOTTOM, BOARD_RIGHT, LOWER_BOX_BTM);

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = STYLE_SQUAREWAVE_WHITE };

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, LOWER_MSG_Y, msg);
}

static void
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

  minigame_upper_msg_render (TURN_MESSAGES[active_player]);
  minigame_lower_msg_render ("Place a piece touching a corner!");

  rdpq_detach_show ();
}

static void
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
        minigame_set_state (MINIGAME_STATE_PAUSE);
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
          minigame_set_state (MINIGAME_STATE_END);
        }
    }
}

static void
minigame_pause_render (void)
{
  // Attach and clear the screen
  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();

  board_render ();

  PLAYER_FOREACH (p) { player_render (&players[p], false); }

  minigame_upper_msg_render ("Game Paused");
  minigame_lower_msg_render ("Press A + B + Start to exit");

  rdpq_detach_show ();
}

static void
minigame_pause_loop (float deltatime)
{
  if (menu_input_delay > 0.0f)
    {
      menu_input_delay -= deltatime;
    }
  else
    {

      joypad_port_t port;
      joypad_buttons_t btn, pressed;

      PLAYER_FOREACH (p)
      {
        if (p < core_get_playercount ())
          {
            port = core_get_playercontroller (p);
            btn = joypad_get_buttons (port);
            pressed = joypad_get_buttons_pressed (port);

            if (pressed.start)
              {

                if (btn.a && btn.b)
                  {
                    minigame_end ();
                  }
                else
                  {
                    minigame_set_state (MINIGAME_STATE_PLAY);
                  }
              }
          }
      }
    }

  minigame_pause_render ();
}

static void
minigame_end_render (void)
{
  // Attach and clear the screen
  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();

  board_render ();

  rdpq_detach_show ();
}

static void
minigame_end_loop (float deltatime)
{
  joypad_port_t port;
  joypad_buttons_t pressed;

  if (menu_input_delay > 0.0f)
    {
      menu_input_delay -= deltatime;
    }
  else
    {

      PLAYER_FOREACH (p)
      {
        if (p < core_get_playercount ())
          {
            port = core_get_playercontroller (p);
            pressed = joypad_get_buttons_pressed (port);

            if (pressed.a || pressed.b || pressed.start)
              {
                minigame_end ();
              }
          }
      }
    }

  minigame_end_render ();
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

    default:
      minigame_end ();
      break;
    }
}
