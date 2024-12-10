#include <libdragon.h>

#include "../../minigame.h"

#include "background.h"
#include "board.h"
#include "color.h"
#include "font.h"
#include "global.h"
#include "player.h"
#include "scoreboard.h"

const MinigameDef minigame_def
    = { .gamename = "Land Grab",
        .developername = "Meeq Corporation",
        .description = "Claim as much land as you can!",
        .instructions = "Place pieces at diagonals to claim land. "
                        "The player with the most land at the end wins!" };

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
  FMT_STYLE_P1 "Player 1's Turn!",
  FMT_STYLE_P2 "Player 2's Turn!",
  FMT_STYLE_P3 "Player 3's Turn!",
  FMT_STYLE_P4 "Player 4's Turn!",
};

static const char *RANDOM_HINTS[] = {
  "Press B to end your turn",
  "Press A to place your piece",
  "Change pieces with C-Left/C-Right",
  "Press L/Z to mirror your piece",
  "Press R to flip your piece",
  "Press C-Down for smaller pieces",
  "Corner connections are key!",
  "Your pieces must all connect",
  "Other colors can block you!",
  "Don't get blocked out!",
  "Expand wisely...",
  "Use all pieces for a bonus!",
  "Use monomino last for a bonus!",
  "Monomino means 'one square'",
  "Each square is worth 1 point",
  "Try to use all of your pieces!",
  "When in doubt, expand out!",
  "Act decisively!",
  "What could possibly go wrong?",
  "Are you sure about that?",
  "Don't forget to guard your flank",
  "Try to block your opponents",
  "Start with larger pieces",
  "Save small pieces for the end",
  "Play defensively",
  "Anticipate their next moves",
  "Adapt, overcome, improvise",
};

static float random_hint_timer = 0.0f;
static const float RANDOM_HINT_DELAY = 10.0f;
static bool random_hint_paused = false;
static const char *hint_msg = NULL;

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
      xm64player_stop (&music);
      menu_input_delay = PAUSE_INPUT_DELAY;
    }

  if (new_state == MINIGAME_STATE_END)
    {
      menu_input_delay = END_INPUT_DELAY;
      scoreboard_calculate (true);
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

  font_init ();

  background_init ();
  board_init ();
  scoreboard_init ();

  PLAYER_FOREACH (p) { player_init (&players[p], p); }

  turn_count = 0;
  hint_msg = NULL;
  random_hint_paused = false;
  random_hint_timer = RANDOM_HINT_DELAY;
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

  scoreboard_cleanup ();
  board_cleanup ();
  background_cleanup ();

  font_cleanup ();

  rdpq_debug_stop ();

  display_close ();
}

void
minigame_set_hint (const char *msg)
{
  hint_msg = msg;
  random_hint_paused = true;
}

static const int UPPER_BOX_TOP = BOARD_TOP - 12;
static const int UPPER_MSG_Y = UPPER_BOX_TOP + 9;
static const int LOWER_BOX_BTM = BOARD_BOTTOM + 13;
static const int LOWER_MSG_Y = LOWER_BOX_BTM - 4;

static void
minigame_upper_msg_print (const char *msg)
{
  rdpq_set_mode_fill (COLOR_BLACK);
  rdpq_fill_rectangle (BOARD_LEFT, UPPER_BOX_TOP, BOARD_RIGHT, BOARD_TOP);

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = FONT_STYLE_WHITE };

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, UPPER_MSG_Y, msg);
}

#define minigame_upper_msg_printf(fmt, ...)                                   \
  {                                                                           \
    rdpq_mode_push ();                                                        \
    rdpq_set_mode_fill (COLOR_BLACK);                                         \
    rdpq_fill_rectangle (BOARD_LEFT, UPPER_BOX_TOP, BOARD_RIGHT, BOARD_TOP);  \
    rdpq_set_mode_standard ();                                                \
    rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,         \
                                   .align = ALIGN_CENTER,                     \
                                   .style_id = FONT_STYLE_WHITE };            \
    rdpq_text_printf (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, UPPER_MSG_Y,   \
                      fmt, __VA_ARGS__);                                      \
    rdpq_mode_pop ();                                                         \
  }

static void
minigame_lower_msg_print (const char *msg)
{
  rdpq_set_mode_fill (COLOR_BLACK);
  rdpq_fill_rectangle (BOARD_LEFT, BOARD_BOTTOM, BOARD_RIGHT, LOWER_BOX_BTM);

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = FONT_STYLE_WHITE };

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

  minigame_upper_msg_print (TURN_MESSAGES[active_player]);

  if (hint_msg != NULL)
    {
      minigame_lower_msg_print (hint_msg);
    }
  else if (players[active_player].pieces_left == PIECE_COUNT)
    {
      minigame_lower_msg_print ("Place a piece touching a corner!");
    }
  else
    {
      minigame_lower_msg_print ("Expand diagonally to win!");
    }

  scoreboard_pieces_render ();

  scoreboard_scores_render ();

  rdpq_detach_show ();
}

static void
minigame_play_loop (float deltatime)
{
  const PlyNum active_player = turn_count % MAXPLAYERS;
  bool turn_ended = false;

  PLAYER_FOREACH (p)
  {
    PlayerTurnResult player_loop_result
        = p < core_get_playercount ()
              ? player_loop (&players[p], p == active_player, deltatime)
              : player_loop_ai (&players[p], p == active_player, deltatime);

    if (player_loop_result == PLAYER_TURN_PAUSE)
      {
        minigame_set_state (MINIGAME_STATE_PAUSE);
        break;
      }
    // Only the active player can end the turn
    if (p == active_player)
      {
        turn_ended = player_loop_result == PLAYER_TURN_END
                     || player_loop_result == PLAYER_TURN_PASS;
        last_active_turn[p] = player_loop_result;
      }
  }

  // Tick the random hint timer
  if (!random_hint_paused)
    {
      random_hint_timer -= deltatime;
      if (random_hint_timer < 0.0f)
        {
          hint_msg = RANDOM_HINTS[rand () % ARRAY_SIZE (RANDOM_HINTS)];
          random_hint_timer = RANDOM_HINT_DELAY;
        }
    }

  scoreboard_calculate (false);
  minigame_play_render ();

  // Wait until after rendering to "end the turn" so the UI is consistent.
  if (turn_ended)
    {
      hint_msg = NULL;
      random_hint_paused = false;
      random_hint_timer = RANDOM_HINT_DELAY;
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

  minigame_upper_msg_print ("Game Paused");
  minigame_lower_msg_print ("Press A + B + Start to exit");

  rdpq_detach_show ();
}

static void
minigame_pause_loop (float deltatime)
{
  joypad_port_t port;
  joypad_buttons_t btn, pressed;

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

  scoreboard_pieces_render ();

  scoreboard_scores_render ();

  if (winners == 1)
    {
      minigame_upper_msg_printf ("Player %d wins!", scoreboard[0].p + 1);
    }
  else if (winners == 2)
    {
      minigame_upper_msg_printf ("Players %d and %d win!", scoreboard[0].p + 1,
                                 scoreboard[1].p + 1);
    }
  else if (winners == 3)
    {
      minigame_upper_msg_printf ("Players %d, %d, and %d win!",
                                 scoreboard[0].p + 1, scoreboard[1].p + 1,
                                 scoreboard[2].p + 1);
    }
  else if (winners == 4)
    {
      minigame_upper_msg_print ("It's a draw!");
    }

  minigame_lower_msg_print ("Press A / B / Start to exit");

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
