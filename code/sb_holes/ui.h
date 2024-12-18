#ifndef UI_H
#define UI_H

#include "ui/ui_font.h"
#include "ui/ui_colors.h"
#include "ui/ui_sprite.h"

enum cStates
{
    C_UP,
    C_DOWN,
    C_LEFT,
    C_RIGHT
};

enum MENU_TEXT
{
    TEXT_DIFF,
    TEXT_PLAYERS = +4,
    TEXT_BOTS,
    TEXT_CONTROLS,
    TEXT_RUMBLE,
    MENU_TEXT_COUNT
};

const char *uiMainMenuStrings[MENU_TEXT_COUNT] = {
    "Difficulty: ",
    "EASY",
    "MEDIUM",
    "HARD",
    "Players: ",
    "Bots: ",
    "       to Move",
    "Insert Rumble Pak now!"};

float textPositions[2];
float panelPositions[4];

void ui_init(void);
void ui_print(game_data *game, float fps);
void ui_cleanup(void);

// Loads, registers and sets a font with style
void ui_init(void)
{
    ui_fontRegister();
    ui_spriteLoad();
    textPositions[0] = SCREEN_WIDTH / 2;   // x
    textPositions[1] = SCREEN_HEIGHT / 2;  // y
    panelPositions[0] = SCREEN_WIDTH / 2;  // x0
    panelPositions[1] = SCREEN_HEIGHT / 2; // y0
    panelPositions[2] = SCREEN_WIDTH;      // x1
    panelPositions[3] = SCREEN_HEIGHT;     // y1
}

// Optional RDPQ sync and set for text, to prevent bleeding if the autosync engine misses something.
void ui_syncText(void)
{
    rdpq_sync_pipe();
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sync_tile();
}

void ui_fps(float frame_rate, float x, float y)
{
    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);
    ui_syncText();
    rdpq_text_printf(&txt_debugParms, ID_DEBUG, x, y, "FPS %.2f Mem: %d KiB", frame_rate, heap_stats.used / 1024);
}

void ui_printf(float x, float y, const char *txt, ...)
{
    ui_syncText();

    va_list args;
    va_start(args, txt);
    rdpq_text_vprintf(&txt_debugParms, ID_DEBUG, x, y, txt, args);
}

void ui_playerScores(player_data *player)
{
    static float base[] = {32, 32};
    float position[] = {base[0], base[1]};
    ui_syncText();
    rdpq_textparms_t playerTextParms = txt_gameParms;
    playerTextParms.style_id = STYLE_PLAYER + player->plynum;
    playerTextParms.align = ALIGN_LEFT;
    rdpq_text_printf(&playerTextParms, ID_DEFAULT, position[0] + 75 * player->plynum, position[1], "P%d : %u", player->plynum + 1, player->score);
}

void ui_playerOut(player_data *player)
{
    ui_syncText();
    rdpq_textparms_t playerTxtParms = txt_gameParms;
    playerTxtParms.style_id = STYLE_PLAYER + player->plynum;
    float xOffset = 80;
    float yOffset = 60;
    float x, y = 0;

    if (player->plynum == 0)
    {
        x = -xOffset;
        y = textPositions[1] - yOffset;
    }
    else if (player->plynum == 1)
    {
        x = xOffset;
        y = textPositions[1] - yOffset;
    }
    else if (player->plynum == 2)
    {
        x = -xOffset;
        y = textPositions[1] + yOffset;
    }
    else // player->plynum == 3
    {
        x = xOffset;
        y = textPositions[1] + yOffset;
    }

    rdpq_text_printf(&playerTxtParms, ID_DEFAULT, x, y, "Player %d Out", player->plynum + 1);
}

void ui_print_winner(int winner)
{
    ui_spriteDrawPanel(TILE1, sprite_gloss, T_BLACK, panelPositions[0] - 64, panelPositions[1] - 18, panelPositions[0] + 60, panelPositions[1] + 10, 0, 0, 64, 64);
    ui_syncText();
    if (winner != 5) // 5 signifies a Draw
    {
        rdpq_textparms_t winnerTextParms = txt_gameParms;
        winnerTextParms.style_id = STYLE_PLAYER + winner;
        winnerTextParms.align = ALIGN_CENTER;
        rdpq_text_printf(&winnerTextParms, ID_DEFAULT, 0, textPositions[1], "Player %d Wins", winner + 1);
    }
    else
    {
        rdpq_text_print(&txt_gameParms, ID_DEFAULT, 0, textPositions[1], "TIE!");
    }
}

// Catch-all print function for Countdown, Score and Winner, with optional FPS print
void ui_print(game_data *game, float fps)
{
    if (game->countDownTimer > 0.0f)
    {
        rdpq_text_printf(&txt_titleParms, ID_TITLE, 0, 126, "%d", (int)ceilf(game->countDownTimer));
    }
    else if (game->countDownTimer > -GO_DELAY)
    {
        rdpq_text_print(&txt_titleParms, ID_TITLE, 0, 126, "GO!");
    }
    else if (game->isEnding && game->endTimer >= WIN_SHOW_DELAY)
    {
        ui_print_winner(game->winner);
    }
    else
    {
        for (size_t scores = 0; scores < MAXPLAYERS; scores++)
            if (players[scores].isAlive)
                ui_playerScores(&players[scores]);
    }

    if (fps)
        ui_fps(display_get_fps(), 20, 20);
}

void ui_pause(control_data *control)
{

    ui_spriteDrawPanel(TILE2, sprite_gloss, T_BLUE, 40, 60, 280, 144, 0, 0, 64, 64);
    ui_spriteDrawPanel(TILE3, sprite_tessalate, T_BLACK, 50, 65, 270, 134, 0, 0, 64, 64);

    if (control->pressed.start || control->held.start)
    {
        ui_spriteDraw(TILE4, sprite_faceButtons0, 1, 142, 110);
    }
    else
    {
        ui_spriteDraw(TILE4, sprite_faceButtons0, 0, 142, 110);
    }

    ui_spriteDraw(TILE5, sprite_dPadTriggers, 5, 159, 180);

    ui_syncText();
    rdpq_text_print(&txt_titleParms, ID_TITLE, 0, 84, "holes\nA Clone of Hole.io");
    txt_gameParms.align = ALIGN_LEFT;
    rdpq_text_print(&txt_gameParms, ID_DEFAULT, 98, 122, "Press        to Return");
    txt_gameParms.align = ALIGN_CENTER;
    rdpq_text_print(&txt_gameParms, ID_DEFAULT, 0, 164, "PAUSED\n\nHold       to\nQuit Game");
}

void ui_intro(control_data *control)
{
    // Basic frame counter for timing
    static uint32_t introTimer = 0;
    introTimer++;

    // Animated text positions
    static float topTextPosition[] = {93.0f, 0.0f};
    topTextPosition[1] = topTextPosition[1] + 2.0f;
    if (topTextPosition[1] > 56.0f)
        topTextPosition[1] = 56.0f;

    if (introTimer < 100)
    {

        /* STRAWBERRY SCREEN */

        // Panels
        ui_spriteDrawPanel(TILE1, sprite_strawberryTop, WHITE, 128, 80, 196, 112, 0, 0, 32, 16);
        if (introTimer >= 50)
        {
            ui_spriteDrawPanel(TILE2, sprite_strawberry1, WHITE, 128, 112, 196, 144, 0, 0, 32, 16);
        }
        else
        {
            ui_spriteDrawPanel(TILE2, sprite_strawberry0, WHITE, 128, 112, 196, 144, 0, 0, 32, 16);
        }

        // Buttons
        if (control->pressed.start || control->held.start)
        {
            introTimer = 101;
            ui_spriteDraw(TILE3, sprite_faceButtons0, 1, 130, 214);
        }
        else
        {
            ui_spriteDraw(TILE3, sprite_faceButtons0, 0, 130, 214);
        }

        // Text
        ui_syncText();
        rdpq_text_print(&txt_titleParms, ID_TITLE, 0, 56, "Strawberry Byte");
        if (introTimer >= 50)
            rdpq_text_print(&txt_titleParms, ID_TITLE, 0, 190, "Presents");
        rdpq_text_print(&txt_gameParms, ID_DEFAULT, 0, 226, "Press        to Skip Intro");
    }
    else
    {

        // Buttons
        if (control->pressed.start || control->held.start)
        {
            ui_spriteDraw(TILE3, sprite_faceButtons0, 1, 152, 66);
        }
        else
        {
            ui_spriteDraw(TILE3, sprite_faceButtons0, 0, 152, 66);
        }

        joypad_inputs_t joypad = joypad_get_inputs(PLAYER_1);

        ui_spriteDraw(TILE5, sprite_controlStick, 0, 134, 86);
        int stickX = 134 + (joypad.stick_x / 15);
        int stickY = 54 + (spriteHeight * 2) - (joypad.stick_y / 15);
        ui_spriteDraw(TILE5, sprite_controlStick, 1, stickX, stickY);

        ui_spriteDraw(TILE6, sprite_dPadTriggers, 0, 168, 86);

        // Text
        ui_syncText();
        rdpq_text_print(&txt_titleParms, ID_TITLE, 0, 40, "holes\nA Clone of Hole.io");
        txt_gameParms.align = ALIGN_LEFT;
        rdpq_text_print(&txt_gameParms, ID_DEFAULT, 108, 78, "Press         to Play");
        txt_gameParms.align = ALIGN_CENTER;
        rdpq_text_print(&txt_gameParms, ID_DEFAULT, 0, 100, "or\nto Move");
        rdpq_text_print(&txt_titleParms, ID_DEFAULT, 0, 136, "CREDITS:");

        // @TODO: Probably should make this a rdpq_paragraph
        rdpq_text_print(&txt_gameParms, ID_DEBUG, 0, 148,
                        "Programming: s4ys\n"
                        "Models: s4ys\n"
                        "Strawberry Sprite by Sonika Rud\n"
                        "UI Sprites by Kenney\n"
                        "Music: 'The Morning After' by Soft One\n"
                        "SFX obtained from Freesound");
    }
}

void ui_cleanup(void)
{
    ui_spriteCleanup();
    ui_fontUnregister();
    ui_fileCleanup();
}

#endif // UI_H