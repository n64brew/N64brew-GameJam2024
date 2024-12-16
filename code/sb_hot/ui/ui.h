#ifndef UI_H
#define UI_H

#include "ui_font.h"
#include "ui_colors.h"
#include "ui_sprite.h"

#ifdef __cplusplus
extern "C"
{
#endif

// @TODO: Recreate button sprite tiling to remove these insanity checks.
#define aPressed 2
#define aIdle 3
#define aHeld 2

#define bPressed 0
#define bIdle 1
#define bHeld 3

#define cIdle 0
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
        "       to Move\n       to Jump",
        "Insert Rumble Pak now!"};

    const char *uiTipStrings[ACTOR_COUNT] = {
        "Platforms will fall when stood on.\nHurry to the closest safe one.",
        "Hold A to extend jump time.\nSee if you can make\na 2 platform gap.",
        "Try to corner opponents\nand reducing their exit routes.",
        "Remember to roll 10d10\nwhen you fall in the lava."};

    const char *uiCharacterSelectStrings[ACTOR_COUNT] = {
        "s4ys",
        "Wolfie",
        "Mewde",
        "Olli"};

    /* Declarations */

    void ui_init(void);
    void ui_syncText(void);
    void ui_fps(float frame_rate, float x, float y);
    void ui_printf(float x, float y, const char *txt, ...);
    void ui_main_menu(ControllerData *control, int diff);
    void ui_input_display(ControllerData *control);
    void ui_textbox(void);
    void ui_cleanup(void);

    /* Definitons */

    void ui_init(void)
    {
        ui_fontRegister();
        ui_spriteLoad();
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

    void ui_print_winner(int winner)
    {
        ui_spriteDrawPanel(TILE1, sprite_gloss, T_BLACK, 96, 102, 210, 130, 0, 0, 64, 64);
        ui_syncText();
        if (winner != 5) // 5 signifies a Draw
        {
            rdpq_textparms_t winnerTextParms = txt_gameParms;
            winnerTextParms.style_id = STYLE_PLAYER + winner - 1;
            rdpq_text_printf(&winnerTextParms, ID_DEFAULT, 110, 120, "Player %d Wins", winner);
        }
        else
        {
            rdpq_text_print(&txt_gameParms, ID_DEFAULT, 132, 120, "DRAW!");
        }
    }

    void ui_print_playerNum(Player *player, Screen *screen)
    {
        Vector3 pos = player_getBillboard(player, &screen->gameplay_viewport);
        ui_syncText();
        rdpq_textparms_t playerTextParms = txt_gameParms;
        playerTextParms.style_id = STYLE_PLAYER + player->id;
        rdpq_text_printf(&playerTextParms, ID_DEFAULT, pos.x, pos.z, "P%d", player->id + 1);
    }

    void ui_countdown(int secondsLeft)
    {
        // Convert secondsLeft integer to a string
        char countdownText[2];
        snprintf(countdownText, sizeof(countdownText), "%d", secondsLeft);

        ui_syncText();
        rdpq_text_printf(&txt_titleParms, ID_TITLE, 150, 110, "%s", countdownText);

        // One of the four tips at random during countdown
        static int randomTip = -1;
        if (randomTip == -1)
            randomTip = rand() % 4;
        txt_titleParms.align = ALIGN_CENTER;
        txt_titleParms.width = 320;
        rdpq_text_printf(&txt_titleParms, ID_DEFAULT, 0, 150, "%s", uiTipStrings[randomTip]);
        txt_titleParms.align = ALIGN_LEFT;
        txt_titleParms.width = 0;
    }

    // Controller data is passed here for visual feedback for the button press.
    void ui_main_menu(ControllerData *control, int diff)
    {
        // Panels
        ui_spriteDrawPanel(TILE2, sprite_gloss, T_RED, 90, 40, 230, 124, 0, 0, 64, 64);
        ui_spriteDrawPanel(TILE3, sprite_tessalate, T_BLACK, 100, 45, 220, 114, 0, 0, 64, 64);

        // Buttons
        if (control->pressed.start || control->held.start)
        {
            ui_spriteDraw(TILE4, sprite_faceButtons0, 1, 170, 90);
        }
        else
        {
            ui_spriteDraw(TILE4, sprite_faceButtons0, 0, 170, 90);
        }
        ui_spriteDraw(TILE5, sprite_controlStick, 0, 92, 170);
        int stickX = 92 + (control->input.stick_x / 15);
        int stickY = 138 + (spriteHeight * 2) - (control->input.stick_y / 15);
        ui_spriteDraw(TILE5, sprite_controlStick, 1, stickX, stickY);
        if (control->pressed.a || control->held.a)
        {
            ui_spriteDraw(TILE6, sprite_faceButtons1, aHeld, 92, 186);
        }
        else
        {
            ui_spriteDraw(TILE6, sprite_faceButtons0, aIdle, 92, 186);
        }

        int count = core_get_playercount();
        static int set = 0;
        if (count == 4)
        {

            if (control->pressed.b || control->held.b)
            {
                ui_spriteDraw(TILE4, sprite_faceButtons1, bHeld, 74, 128);
            }
            else
            {
                ui_spriteDraw(TILE4, sprite_faceButtons1, bIdle, 74, 128);
            }

            set = diff;
        }
        else
        {
            set = core_get_aidifficulty();
        }

        // Text
        ui_syncText();
        rdpq_text_print(&txt_titleParms, ID_TITLE, 106, 64, " Hot Hot\nHexagons");
        rdpq_text_print(&txt_gameParms, ID_DEFAULT, 128, 102, "Press");
        rdpq_text_printf(&txt_gameParms, ID_DEFAULT, 92, 140,
                         "%s %s\n"
                         "%s %d\n"
                         "%s %d\n"
                         "%s\n\n"
                         "%s\n",
                         uiMainMenuStrings[TEXT_DIFF], uiMainMenuStrings[TEXT_DIFF + set + 1],
                         uiMainMenuStrings[TEXT_PLAYERS], count,
                         uiMainMenuStrings[TEXT_BOTS], ACTOR_COUNT - count,
                         uiMainMenuStrings[TEXT_CONTROLS],
                         uiMainMenuStrings[TEXT_RUMBLE]);
    }

    void ui_pause(ControllerData *control)
    {

        ui_spriteDrawPanel(TILE2, sprite_gloss, T_RED, 90, 60, 230, 144, 0, 0, 64, 64);
        ui_spriteDrawPanel(TILE3, sprite_tessalate, T_BLACK, 100, 65, 220, 134, 0, 0, 64, 64);

        if (control->pressed.start || control->held.start)
        {
            ui_spriteDraw(TILE4, sprite_faceButtons0, 1, 170, 110);
        }
        else
        {
            ui_spriteDraw(TILE4, sprite_faceButtons0, 0, 170, 110);
        }

        ui_spriteDraw(TILE5, sprite_dPadTriggers, 5, 160, 180);

        ui_syncText();
        rdpq_text_print(&txt_titleParms, ID_TITLE, 106, 84, " Hot Hot\nHexagons");
        rdpq_text_print(&txt_gameParms, ID_DEFAULT, 128, 122, "Press\n\n\n  PAUSED\n\nHold       to\nQuit Game");
    }

    void ui_character_select(ControllerData *control, uint8_t selectedActor)
    {
        // Buttons
        if (control->pressed.a || control->held.a)
        {
            ui_spriteDraw(TILE2, sprite_faceButtons1, aHeld, 104, 46);
        }
        else
        {
            ui_spriteDraw(TILE2, sprite_faceButtons0, aIdle, 104, 46);
        }

        // Text
        ui_syncText();
        rdpq_text_print(&txt_titleParms, ID_TITLE, 70, 40, "Character Select");
        rdpq_text_print(&txt_gameParms, ID_DEFAULT, 63, 58, "Press        to Confirm Selection");
        rdpq_text_printf(&txt_titleParms, ID_DEFAULT, 84, 76, "Selected Actor: %s", uiCharacterSelectStrings[selectedActor]);
    }

    // Time to crash test the RDP
    void ui_input_display(ControllerData *control)
    {
        int s = 24;
        int t = 164;

        // First row: Triggers
        if (control->pressed.l || control->held.l)
            ui_spriteDraw(TILE0, sprite_dPadTriggers, 6, s, t);
        if (control->pressed.z || control->held.z)
            ui_spriteDraw(TILE1, sprite_dPadTriggers, 5, s + spriteWidth, t);
        if (control->pressed.r || control->held.r)
            ui_spriteDraw(TILE2, sprite_dPadTriggers, 7, s + (spriteWidth * 2), t);

        // Second row: Face Buttons
        if (control->pressed.a || control->held.a)
        {
            ui_spriteDraw(TILE3, sprite_faceButtons1, aHeld, s, t + spriteHeight);
        }
        else
        {
            ui_spriteDraw(TILE3, sprite_faceButtons0, aIdle, s, t + spriteHeight);
        }

        if (control->pressed.b || control->held.b)
        {
            ui_spriteDraw(TILE4, sprite_faceButtons1, bHeld, s + spriteHeight, t + spriteHeight);
        }
        else
        {
            ui_spriteDraw(TILE4, sprite_faceButtons1, bIdle, s + spriteHeight, t + spriteHeight);
        }

        if (control->pressed.start || control->held.start)
        {
            ui_spriteDraw(TILE5, sprite_faceButtons0, 1, s + (spriteHeight * 2), t + spriteHeight);
        }
        else
        {
            ui_spriteDraw(TILE5, sprite_faceButtons0, 0, s + (spriteHeight * 2), t + spriteHeight);
        }

        // Third row: Sticks
        ui_spriteDraw(TILE6, sprite_controlStick, 0, s, t + (spriteHeight * 2));
        int stickX = s + (control->input.stick_x / 15);
        int stickY = t + (spriteHeight * 2) - (control->input.stick_y / 15);
        ui_spriteDraw(TILE6, sprite_controlStick, 1, stickX, stickY);
        if (control->pressed.c_up || control->held.c_up)
        {
            ui_spriteDraw(TILE7, sprite_cButtons1, C_UP, s + (spriteHeight * 2), t + (spriteHeight * 2));
        }
        else if (control->pressed.c_down || control->held.c_down)
        {
            ui_spriteDraw(TILE7, sprite_cButtons1, C_DOWN, s + (spriteHeight * 2), t + (spriteHeight * 2));
        }
        else if (control->pressed.c_left || control->held.c_left)
        {
            ui_spriteDraw(TILE7, sprite_cButtons1, C_LEFT, s + (spriteHeight * 2), t + (spriteHeight * 2));
        }
        else if (control->pressed.c_right || control->held.c_right)
        {
            ui_spriteDraw(TILE7, sprite_cButtons1, C_RIGHT, s + (spriteHeight * 2), t + (spriteHeight * 2));
        }
        else
        {
            ui_spriteDraw(TILE7, sprite_cButtons0, 0, s + (spriteHeight * 2), t + (spriteHeight * 2));
        }
    }

    void ui_intro(ControllerData *control)
    {
        // Basic frame counter for timing
        static uint32_t introTimer = 0;
        const float refreshRate = display_get_fps();
        introTimer++;

        // Animated text positions
        static Vector2 topTextPosition = {93.0f, 0.0f};
        topTextPosition.y = topTextPosition.y + 2.0f;
        if (topTextPosition.y > 56.0f)
            topTextPosition.y = 56.0f;

        // Dynamic alpha from prim colors
        uint32_t dynamicColorsPacked[3] = {0, 0, 0};
        color_t dynamicColors[3];
        dynamicColorsPacked[0] = ui_colorSetAlpha(COLORS[N_RED], introTimer * refreshRate * display_get_delta_time());
        dynamicColorsPacked[1] = ui_colorSetAlpha(COLORS[N_GREEN], introTimer * refreshRate * display_get_delta_time());
        dynamicColorsPacked[2] = ui_colorSetAlpha(COLORS[N_YELLOW], introTimer * refreshRate * display_get_delta_time());
        dynamicColors[0] = color_from_packed32(dynamicColorsPacked[0]);
        dynamicColors[1] = color_from_packed32(dynamicColorsPacked[1]);
        dynamicColors[2] = color_from_packed32(dynamicColorsPacked[2]);

        if (introTimer < refreshRate * 6.0f)
        {

            /* MADE WITH SCREEN */

            // Panels
            ui_spriteDrawDynamic(TILE1, sprite_libdragon, dynamicColors[0], 28, 76, 156, 140, 0, 0, 128, 64);
            ui_spriteDrawDynamic(TILE2, sprite_t3d, dynamicColors[1], 160, 76, 288, 140, 0, 0, 64, 32);
            ui_spriteDrawDynamic(TILE3, sprite_mixamo, dynamicColors[2], 96, 146, 224, 210, 0, 0, 128, 64);

            // Buttons
            if (control->pressed.start || control->held.start)
            {
                ui_spriteDraw(TILE4, sprite_faceButtons0, 1, 132, 214);
            }
            else
            {
                ui_spriteDraw(TILE4, sprite_faceButtons0, 0, 132, 214);
            }

            // Text
            ui_syncText();
            rdpq_text_print(&txt_titleParms, ID_TITLE, topTextPosition.x, topTextPosition.y, "Made with");
            rdpq_text_print(&txt_gameParms, ID_DEFAULT, 90, 226, "Press        to Skip Intro");
        }
        else if (introTimer < refreshRate * 10.0f)
        {

            /* STRAWBERRY SCREEN */

            // Panels
            ui_spriteDrawPanel(TILE1, sprite_strawberryTop, WHITE, 128, 80, 196, 112, 0, 0, 32, 16);
            if (introTimer >= refreshRate * 8.0f)
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
                ui_spriteDraw(TILE3, sprite_faceButtons0, 1, 132, 214);
            }
            else
            {
                ui_spriteDraw(TILE3, sprite_faceButtons0, 0, 132, 214);
            }

            // Text
            ui_syncText();
            rdpq_text_print(&txt_titleParms, ID_TITLE, 68, 56, "Strawberry Byte");
            if (introTimer >= refreshRate * 8.0f)
                rdpq_text_print(&txt_titleParms, ID_TITLE, 110, 190, "Presents");
            rdpq_text_print(&txt_gameParms, ID_DEFAULT, 90, 226, "Press        to Skip Intro");
        }
        else
        {

            // Buttons
            if (control->pressed.start || control->held.start)
            {
                ui_spriteDraw(TILE3, sprite_faceButtons0, 1, 170, 66);
            }
            else
            {
                ui_spriteDraw(TILE3, sprite_faceButtons0, 0, 170, 66);
            }

            // Text
            ui_syncText();
            rdpq_text_print(&txt_titleParms, ID_TITLE, 106, 40, " Hot Hot\nHexagons");
            rdpq_text_print(&txt_gameParms, ID_DEFAULT, 128, 78, "Press");
            rdpq_text_print(&txt_titleParms, ID_DEFAULT, 32, 94, "CREDITS:");

            // @TODO: Probably should make this a rdpq_paragraph
            rdpq_text_print(&txt_gameParms, ID_DEBUG, 32, 114,
                            "Programming: zoncabe, s4ys\n"
                            "Models: zoncabe, mewde, s4ys\n"
                            "- Lava model by HailToDodongo\n"
                            "- Original 'Olli' by FazanaJ\n"
                            "Strawberry Sprite by Sonika Rud\n"
                            "UI Sprites by Kenney\n"
                            "Music by Kaelin Stemmler\n"
                            "SFX obtained from Freesound");
        }
    }

    void ui_cleanup(void)
    {
        ui_spriteCleanup();
        ui_fontUnregister();
        ui_fileCleanup();
    }

#ifdef __cplusplus
}
#endif

#endif // UI_H