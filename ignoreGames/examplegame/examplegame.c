/***************************************************************
                         examplegame.c
                               
An example minigame to demonstrate how to use the template for
the game jam.
***************************************************************/

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#define FONT_TEXT           1

#define COUNTDOWN_DELAY     3.0f
#define GO_DELAY            1.0f
#define WIN_DELAY           5.0f
#define WIN_SHOW_DELAY      2.0f

#define POINTS_TO_WIN       150
#define POINTS_PER_PRESS    6

#define POWERBAR_WIDTH      100
#define POWERBAR_HEIGHT     10
#define SCREEN_MARGIN       35
#define PLAYER_HEIGHT       20

#define GAME_BACKGROUND     0x000000FF
#define POWERBAR_BACKGROUND 0x333333FF
#define POWERBAR_FOREGROUND 0xEEEEEEFF


/*********************************
             Globals
*********************************/

// You need this function defined somewhere in your project
// so that the minigame manager can work
const MinigameDef minigame_def = {
    .gamename = "Example Game",
    .developername = "Your Name",
    .description = "This is an example game.",
    .instructions = "Mash A to win."
};

rdpq_font_t *font;

uint32_t player_points[MAXPLAYERS];
uint32_t ai_press_timer[MAXPLAYERS];

float countdown_timer;
bool is_ending;
float end_timer;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

bool has_player_won(PlyNum player)
{
    return player_points[player] >= POINTS_TO_WIN;
}

bool is_countdown()
{
    return countdown_timer > 0.0f;
}

bool can_control()
{
    return !(is_ending || is_countdown());
}

uint32_t ai_get_ticks_until_next_press()
{
    const float min_ticks[] = { 2.8f, 2.4f, 2.0f };   // Minimum delay in ticks by difficulty
    const float max_ticks[] = { 8.4f, 7.7f, 7.0f };   // Maximum delay in ticks by difficulty

    float random = (float)rand() / RAND_MAX;

    int diff = core_get_aidifficulty();
    return min_ticks[diff] + random * (max_ticks[diff] - min_ticks[diff]);
}

/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_TEXT, font);

    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        player_points[i] = 0;
        ai_press_timer[i] = ai_get_ticks_until_next_press();
    }

    countdown_timer = COUNTDOWN_DELAY;
    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");
}


/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is 
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/

void minigame_fixedloop(float deltatime)
{
    bool couldcontrol = can_control();
    if (countdown_timer > -GO_DELAY)
    {
        float prevtime = countdown_timer;
        countdown_timer -= deltatime;
        if ((int)prevtime != (int)countdown_timer && countdown_timer >= 0)
            wav64_play(&sfx_countdown, 31);
    }

    if (is_ending) {
        float prevendtime = end_timer;
        end_timer += deltatime;
        if ((int)prevendtime != (int)end_timer && (int)end_timer == WIN_SHOW_DELAY)
            wav64_play(&sfx_winner, 31);
        if (end_timer > WIN_DELAY) minigame_end();
    }

    if (!can_control()) return;
    if (!couldcontrol && can_control())
        wav64_play(&sfx_start, 31);

    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        // Subtract "point drain" for all players at fixed rate
        if (player_points[i] > 0) player_points[i] -= 1;

        if (i < core_get_playercount()) continue;

        // For AI players, wait for a random number of ticks until the next A press
        ai_press_timer[i] -= 1;
        if (ai_press_timer[i] == 0) {
            player_points[i] += POINTS_PER_PRESS;
            ai_press_timer[i] = ai_get_ticks_until_next_press();
        }
    }

    // Check if anyone has won
    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        if (has_player_won(i)) {
            core_set_winner(i);
            is_ending = true;
            wav64_play(&sfx_stop, 31);
        }
    }
}


/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/

void minigame_loop(float deltatime)
{
    if (can_control()) {
        // Handle button presses of human players in variable step loop so the input feels more responsive
        for (size_t i = 0; i < core_get_playercount(); i++)
        {
            // For human players, check if the physical A button on the controller was pressed
            joypad_buttons_t btn = joypad_get_buttons_pressed(core_get_playercontroller(i));
            if (btn.a) player_points[i] += POINTS_PER_PRESS;
        }
    }

    // Render the UI
    rdpq_attach(display_get(), NULL);
    rdpq_clear(color_from_packed32(GAME_BACKGROUND));

    const int player_screen_coords[MAXPLAYERS][2] = {
        {SCREEN_MARGIN, SCREEN_MARGIN},
        {320-SCREEN_MARGIN-POWERBAR_WIDTH, SCREEN_MARGIN},
        {320-SCREEN_MARGIN-POWERBAR_WIDTH, 240-SCREEN_MARGIN-PLAYER_HEIGHT},
        {SCREEN_MARGIN, 240-SCREEN_MARGIN-PLAYER_HEIGHT}
    };

    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        int xcur = player_screen_coords[i][0];
        int ycur = player_screen_coords[i][1];

        // Draw player names
        rdpq_set_mode_standard();
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, xcur, ycur, "Player %d", i+1);
        ycur += 4;
        
        // Draw power bars
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(color_from_packed32(POWERBAR_BACKGROUND));
        rdpq_fill_rectangle(xcur, ycur, xcur+POWERBAR_WIDTH, ycur+POWERBAR_HEIGHT);
        rdpq_set_prim_color(color_from_packed32(POWERBAR_FOREGROUND));
        int width = (player_points[i]*POWERBAR_WIDTH)/POINTS_TO_WIN;
        if (width > POWERBAR_WIDTH) width = POWERBAR_WIDTH;
        rdpq_fill_rectangle(xcur, ycur, xcur+width, ycur+POWERBAR_HEIGHT);
    }

    rdpq_set_mode_standard();    

    if (is_countdown()) {
        // Draw countdown
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 155, 100, "%d", (int)ceilf(countdown_timer));
    } else if (countdown_timer > -GO_DELAY) {
        // For a short time after countdown is over, draw "GO!"
        rdpq_text_print(NULL, FONT_BUILTIN_DEBUG_MONO, 150, 100, "GO!");
    } else if (is_ending && end_timer >= WIN_SHOW_DELAY) {
        // Draw winner announcement (There might be multiple winners)
        int ycur = 100;
        for (size_t i = 0; i < MAXPLAYERS; i++)
        {
            if (!has_player_won(i)) continue;
            ycur += rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 120, ycur, "Player %d wins!\n", i+1).advance_y;
        }
    }

    rdpq_detach_show();
}


/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);
    display_close();
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
}