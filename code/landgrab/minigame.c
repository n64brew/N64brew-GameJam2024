#include "minigame.h"
#include "global.h"

#include "../../minigame.h"

#include "ai.h"
#include "background.h"
#include "board.h"
#include "color.h"
#include "font.h"
#include "logo.h"
#include "player.h"
#include "scoreboard.h"
#include "sfx.h"

const MinigameDef minigame_def
    = { .gamename = "Land Grab",
        .developername = "Meeq Corporation",
        .description = "Claim as much land as you can!",
        .instructions = "Place pieces at diagonals to claim land. "
                        "The player with the most land at the end wins!" };

#define PAUSE_INPUT_DELAY 0.5f
#define END_INPUT_DELAY 5.0f
#define RANDOM_HINT_DELAY 10.0f

#define MUSIC_PLAY "rom:/landgrab/15yearsb.xm64"
#define MUSIC_END "rom:/landgrab/phekkis-4_weeks_of_hysteria.xm64"

#define COLOR_MSG_BG RGBA32 (0x00, 0x00, 0x00, 0x80)

#define SHOW_STATS_FPS 0
#define SHOW_STATS_MEM 0

#if SHOW_STATS_MEM
static heap_stats_t heap_stats;
#endif

MinigameState minigame_state;
Player players[MAXPLAYERS];

static PlayerTurnResult last_active_turn[MAXPLAYERS];
static Player *winners[MAXPLAYERS];
static size_t winner_count;
static size_t turn_count;
static float menu_input_delay;
static xm64player_t music;
static bool show_controls;
static float random_hint_timer = 0.0f;
static bool random_hint_paused = false;
static const char *hint_msg = NULL;

static const char *TURN_MESSAGES[] = {
  FMT_STYLE_P1 "Player 1's Turn",
  FMT_STYLE_P2 "Player 2's Turn",
  FMT_STYLE_P3 "Player 3's Turn",
  FMT_STYLE_P4 "Player 4's Turn",
};

static const char *RANDOM_HINTS[] = {
  "Press B to skip your turn",
  "Press B to skip your turn",
  "Change pieces with C-Left/C-Right",
  "Change pieces with C-Left/C-Right",
  "Press L/Z to mirror your piece",
  "Press L/Z to mirror your piece",
  "Press R to flip your piece",
  "Press R to flip your piece",
  "Press C-Down for smaller pieces",
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
  "Don't forget to guard your flank",
  "Try to block your opponents",
  "Use larger pieces early",
  "Save small pieces for the end",
  "Play defensively",
  "Anticipate their next moves",
  "Adapt - Improvise - Overcome",
  "Don't let them corner you!",
};

static void
minigame_crown_winners (void)
{
  int high_score = players[0].score;
  PLAYER_FOREACH (p)
  {
    if (players[p].score > high_score)
      {
        high_score = players[p].score;
      }
  }

  winner_count = 0;
  PLAYER_FOREACH (p)
  {
    if (players[p].score == high_score)
      {
        winners[winner_count++] = &players[p];
      }
  }

  if (winner_count < MAXPLAYERS)
    {
      for (size_t i = 0; i < winner_count; i++)
        {
          winners[i]->winner = true;
          core_set_winner (winners[i]->plynum);
        }
    }
}

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
      show_controls = true;
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
      minigame_crown_winners ();
    }

  minigame_state = new_state;
}

static void
minigame_random_hint (void)
{
  hint_msg = RANDOM_HINTS[rand () % ARRAY_SIZE (RANDOM_HINTS)];
  random_hint_timer = RANDOM_HINT_DELAY;
}

static const int UPPER_BOX_TOP = BOARD_TOP - 12;
static const int UPPER_MSG_Y = UPPER_BOX_TOP + 9;
static const int LOWER_BOX_BTM = BOARD_BOTTOM + 13;
static const int LOWER_MSG_Y = LOWER_BOX_BTM - 4;

static void
minigame_upper_msg_print (const char *msg)
{
  rdpq_set_mode_standard ();

  rdpq_mode_push ();
  {
    rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color (COLOR_MSG_BG);
    rdpq_fill_rectangle (BOARD_LEFT, UPPER_BOX_TOP, BOARD_RIGHT, BOARD_TOP);
  }
  rdpq_mode_pop ();

  rdpq_set_mode_standard ();
  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = FONT_STYLE_WHITE };

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, UPPER_MSG_Y, msg);
}

static void
minigame_lower_msg_print (const char *msg)
{
  rdpq_set_mode_standard ();

  rdpq_mode_push ();
  {
    rdpq_mode_combiner (RDPQ_COMBINER_FLAT);
    rdpq_mode_blender (RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color (COLOR_MSG_BG);
    rdpq_fill_rectangle (BOARD_LEFT, BOARD_BOTTOM, BOARD_RIGHT, LOWER_BOX_BTM);
  }
  rdpq_mode_pop ();

  rdpq_textparms_t textparms = { .width = BOARD_RIGHT - BOARD_LEFT,
                                 .align = ALIGN_CENTER,
                                 .style_id = FONT_STYLE_WHITE };

  rdpq_text_print (&textparms, FONT_SQUAREWAVE, BOARD_LEFT, LOWER_MSG_Y, msg);
}

static void
minigame_play_render (void)
{
  PlyNum active_plynum = turn_count % MAXPLAYERS;

  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();
  board_render ();

  // Render the inactive players under the active player
  PLAYER_FOREACH (p)
  {
    if (p != active_plynum && last_active_turn[p] != PLAYER_TURN_PASS)
      {
        player_render (&players[p], false);
      }
  }

  player_render (&players[active_plynum], true);

  if (show_controls)
    {
      scoreboard_controls_render ();
    }
  else
    {
      scoreboard_pieces_render ();
      scoreboard_scores_render ();
    }

  logo_render ();

  minigame_upper_msg_print (TURN_MESSAGES[active_plynum]);

  if (plynum_is_ai (active_plynum))
    {
      minigame_lower_msg_print ("AI is thinking...");
    }
  else if (hint_msg != NULL)
    {
      minigame_lower_msg_print (hint_msg);
    }
  else if (player_is_first_turn (&players[active_plynum]))
    {
      // Show a specific hint on the player's first turn
      minigame_lower_msg_print ("Place a piece touching a corner!");
      // The random hint will replace this message eventually
    }
  else
    {
      // Show a specific hint for the player's second turn
      minigame_lower_msg_print ("Expand diagonally to win!");
      // The random hint will replace this message eventually
    }

#if SHOW_STATS_MEM
  int mem = heap_stats.used / 1024;
  rdpq_text_printf (NULL, FONT_SQUAREWAVE, 10, 15, "Mem: %d KiB", mem);
#endif

#if SHOW_STATS_FPS
  float fps = display_get_fps ();
  rdpq_text_printf (NULL, FONT_SQUAREWAVE, 10, 25, "FPS: %.2f", fps);
#endif

  rdpq_detach_show ();
}

static void
minigame_play_loop (float deltatime)
{
  const PlyNum active_plynum = turn_count % MAXPLAYERS;
  bool turn_ended = false;

  logo_loop (deltatime);

  PLAYER_FOREACH (p)
  {
    bool active = p == active_plynum;
    PlayerTurnResult turn_result
        = plynum_is_ai (p) ? player_loop_ai (&players[p], active, deltatime)
                           : player_loop (&players[p], active, deltatime);

    if (turn_result == PLAYER_TURN_PAUSE)
      {
        minigame_set_state (MINIGAME_STATE_PAUSE);
        break;
      }
    // Only the active player can end the turn
    if (active)
      {
        turn_ended = turn_result == PLAYER_TURN_END
                     || turn_result == PLAYER_TURN_PASS;
        last_active_turn[p] = turn_result;
      }
  }

  // Tick the random hint timer
  if (!random_hint_paused)
    {
      random_hint_timer -= deltatime;
      if (random_hint_timer < 0.0f)
        {
          minigame_random_hint ();
        }
    }

  minigame_play_render ();

  // Wait until after rendering to "end the turn" so the UI is consistent.
  if (turn_ended)
    {
      random_hint_paused = false;
      random_hint_timer = RANDOM_HINT_DELAY;
      turn_count++;

      PlyNum next_player = turn_count % MAXPLAYERS;

      if (!plynum_is_ai (next_player)
          && !player_is_first_turn (&players[next_player]))
        {
          minigame_random_hint ();
        }
      else
        {
          minigame_set_hint (NULL);
        }

      if (plynum_is_ai (next_player))
        {
          if (last_active_turn[next_player] == PLAYER_TURN_PASS)
            {
              ai_reset (NULL);
            }
          else
            {
              ai_reset (&players[next_player]);
            }

          show_controls = false;
        }
      else
        {
          show_controls = player_is_first_turn (&players[next_player]);
        }

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

  scoreboard_controls_render ();

  rdpq_detach_show ();
}

static void
minigame_pause_loop (float deltatime)
{
  joypad_port_t port;
  joypad_buttons_t btn, pressed;

  // Swallow inputs for a moment to prevent accidental input
  if (menu_input_delay > 0.0f)
    {
      menu_input_delay -= deltatime;
    }
  else
    {
      // Any player can unpause or quit the game
      PLAYER_FOREACH (p)
      {
        if (!plynum_is_ai (p))
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
  surface_t *disp = display_get ();
  rdpq_attach (disp, NULL);

  background_render ();
  board_render ();
  scoreboard_pieces_render ();
  scoreboard_scores_render ();

  char msg[sizeof ("Players A, B, and C win!")];

  if (winner_count == 1)
    {
      sprintf (msg, "Player %d wins!", winners[0]->plynum + 1);
      minigame_upper_msg_print (msg);
    }
  else if (winner_count == 2)
    {
      sprintf (msg, "Players %d and %d win!", winners[0]->plynum + 1,
               winners[1]->plynum + 1);
      minigame_upper_msg_print (msg);
    }
  else if (winner_count == 3)
    {
      sprintf (msg, "Players %d, %d, and %d win!", winners[0]->plynum + 1,
               winners[1]->plynum + 1, winners[2]->plynum + 1);
      minigame_upper_msg_print (msg);
    }
  else
    {
      minigame_upper_msg_print ("It's a draw!");
    }

  //minigame_lower_msg_print ("Press A / B / Start to exit");

  rdpq_detach_show ();
}

static void
minigame_end_loop (float deltatime)
{
  //joypad_port_t port;
  //joypad_buttons_t pressed;

  // Swallow inputs for a moment to prevent accidental input.
  // We want the players to actually see the victory screen.
  if (menu_input_delay > 0.0f)
    {
      menu_input_delay -= deltatime;
    }
  else
    {
      /* Game should exit automatically and not require inputs
      PLAYER_FOREACH (p)
      {
        if (!plynum_is_ai (p))
          {
            port = core_get_playercontroller (p);
            pressed = joypad_get_buttons_pressed (port);

            if (pressed.a || pressed.b || pressed.start)
              {
                minigame_end ();
              }
          }
      }
      */
      minigame_end ();
    }

  minigame_end_render ();
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
  sfx_init ();
  logo_init ();
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
  logo_cleanup ();
  sfx_cleanup ();
  font_cleanup ();

  display_close ();
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void
minigame_loop (float deltatime)
{
#if SHOW_STATS_MEM
  sys_get_heap_stats (&heap_stats);
#endif

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

void
minigame_set_hint (const char *msg)
{
  hint_msg = msg;
  random_hint_paused = msg != NULL;
}
