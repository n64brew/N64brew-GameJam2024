/***************************************************************
                             menu.c
                               
This file contains the code for the basic menu
***************************************************************/

#include <libdragon.h>
#include <string.h>
#include "menu.h"
#include "core.h"
#include "config.h"


/*********************************
           Definitions
*********************************/

#define FONT_TEXT       1
#define FONT_DEBUG      2

typedef enum
{
    SCREEN_PLAYERCOUNT,
    SCREEN_AIDIFFICULTY,
    SCREEN_MINIGAME
} menu_screen;

/*==============================
    minigame_sort
    Sorts two names alphabetically
    @param  The first name
    @param  The second name
    @return -1 if a is less than b, 1 if a is greater than b, and 0 if they are equal
==============================*/

static int minigame_sort(const void *a, const void *b)
{
    int idx1 = *(int*)a, idx2 = *(int*)b;
    return strcasecmp(global_minigame_list[idx1].definition.gamename, global_minigame_list[idx2].definition.gamename);
}

/*==============================
    get_selection_offset
    Converts a joypad 8-way direction into a vertical selection offset
    @param  The joypad direction
    @return The selection offset
==============================*/

int get_selection_offset(joypad_8way_t direction)
{
    switch (direction) {
    case JOYPAD_8WAY_UP_RIGHT:
    case JOYPAD_8WAY_UP:
    case JOYPAD_8WAY_UP_LEFT:
        return -1;
    case JOYPAD_8WAY_DOWN_LEFT:
    case JOYPAD_8WAY_DOWN:
    case JOYPAD_8WAY_DOWN_RIGHT:
        return 1;
    default:
        return 0;
    }
}

/*==============================
    get_difficulty_name
    Gets the display name of an AI difficulty level
    @param  The AI difficulty
    @return The display name
==============================*/

const char *get_difficulty_name(AiDiff difficulty)
{
    switch (difficulty)
    {
    case DIFF_EASY:
        return "Easy";
    case DIFF_MEDIUM:
        return "Medium";
    case DIFF_HARD:
        return "Hard";
    default:
        return "Unknown";
    }
}

static uint32_t max_playercount;
static uint32_t playercount = PLAYER_COUNT;
static AiDiff ai_difficulty = AI_DIFFICULTY;
static bool is_first_time = true;

static menu_screen current_screen;  // Current menu screen
static int item_count;              // The number of selection items in the current screen
static const char *heading;         // The heading of the menu screen
static int select;                  // The currently selected item
static int yscroll;

/*==============================
    set_menu_screen
    Switches the menu to another screen
    @param  The new screen
==============================*/

void set_menu_screen(menu_screen screen)
{
    current_screen = screen;
    yscroll = 0;
    switch (current_screen) {
    case SCREEN_PLAYERCOUNT:
        item_count = max_playercount;
        select = playercount-1;

        if (max_playercount == 0) {
            heading = "No controllers connected!\n";
        } else {
            heading = "How many players?\n";
        }
        break;
    case SCREEN_AIDIFFICULTY:
        item_count = DIFF_HARD+1;
        select = ai_difficulty;
        heading = "AI difficulty?\n";
        break;
    case SCREEN_MINIGAME:
        item_count = global_minigame_count;
        select = 0;
        heading = "Pick a game!\n";
        break;
    }
}

/*==============================
    menu
    Show the minigame selection menu
    @return The internal name of the selected minigame
==============================*/

char* menu(void)
{
    const color_t BLACK = RGBA32(0x00,0x00,0x00,0xFF);
    const color_t ASH_GRAY = RGBA32(0xAD,0xBA,0xBD,0xFF);
    const color_t MAYA_BLUE = RGBA32(0x6C,0xBE,0xED,0xFF);
    const color_t GUN_METAL = RGBA32(0x31,0x39,0x3C,0xFF);
    const color_t REDWOOD = RGBA32(0xB2,0x3A,0x7A,0xFF);
    const color_t BREWFONT = RGBA32(242,209,155,0xFF);

    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    sprite_t *logo = sprite_load("rom:/n64brew.ia8.sprite");
    sprite_t *jam = sprite_load("rom:/jam.rgba32.sprite");
    
    rdpq_font_t *font = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = MAYA_BLUE, .outline_color = GUN_METAL });

    rdpq_font_t *fontdbg = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_DEBUG, fontdbg);

    max_playercount = 0;
    for (int i = 0; i < MAXPLAYERS; i++) {
        if (joypad_is_connected(i)) max_playercount++;
    }

    bool has_moved_selection = false;
    bool menu_done = false;

    float yselect = -1;
    float yselect_target = -1;

    int sorted_indices[global_minigame_count];
    for (int i = 0; i < global_minigame_count; i++) sorted_indices[i] = i;
    qsort(sorted_indices, global_minigame_count, sizeof(int), minigame_sort);

    int selected_minigame = -1;
    if (SKIP_MINIGAMESELECTION) {
        for (int i = 0; i < global_minigame_count; i++) {
            if (!strcasecmp(global_minigame_list[sorted_indices[i]].internalname, MINIGAME_TO_TEST)) {
                selected_minigame = i;
                break;
            }
        }
    }

    // Set the initial menu screen
    menu_screen targetscreen = SCREEN_MINIGAME;
    if (is_first_time)
        targetscreen = SCREEN_PLAYERCOUNT;
    if (targetscreen == SCREEN_PLAYERCOUNT && SKIP_PLAYERSELECTION)
        targetscreen = SCREEN_AIDIFFICULTY;
    if (targetscreen == SCREEN_AIDIFFICULTY && (SKIP_DIFFICULTYSELECTION || playercount == MAXPLAYERS))
        targetscreen = SCREEN_MINIGAME;
    if (targetscreen == SCREEN_MINIGAME && SKIP_MINIGAMESELECTION)
        menu_done = true;
    set_menu_screen(targetscreen);

    while (!menu_done) {
        joypad_poll();

        int selection_offset = get_selection_offset(joypad_get_direction(JOYPAD_PORT_1, JOYPAD_2D_ANY));
        if (selection_offset != 0) {
            if (!has_moved_selection) select += selection_offset;
            has_moved_selection = true;
        } else {
            has_moved_selection = false;
        }

        if (select < 0) select = 0;
        if (select > item_count-1) select = item_count-1;

        if (select < yscroll) {
            yscroll -= 1;
        }

        joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        if (btn.a) {
            switch (current_screen) {
                case SCREEN_PLAYERCOUNT:
                    playercount = select+1;
                    targetscreen = SCREEN_AIDIFFICULTY;
                    if (targetscreen == SCREEN_AIDIFFICULTY && (SKIP_DIFFICULTYSELECTION || playercount == MAXPLAYERS))
                        targetscreen = SCREEN_MINIGAME;
                    if (targetscreen == SCREEN_MINIGAME && SKIP_MINIGAMESELECTION)
                        menu_done = true;
                    set_menu_screen(targetscreen);
                    break;
                case SCREEN_AIDIFFICULTY:
                    ai_difficulty = select;
                    if (SKIP_MINIGAMESELECTION)
                        menu_done = true;
                    else
                        set_menu_screen(SCREEN_MINIGAME);
                    break;
                case SCREEN_MINIGAME:
                    selected_minigame = select;
                    menu_done = true;
                    break;
            }
        } else if (btn.b) {
            switch (current_screen) {
                case SCREEN_AIDIFFICULTY:
                    set_menu_screen(SCREEN_PLAYERCOUNT);
                    break;
                case SCREEN_MINIGAME:
                    if (playercount == MAXPLAYERS) {
                        set_menu_screen(SCREEN_PLAYERCOUNT);
                    } else {
                        set_menu_screen(SCREEN_AIDIFFICULTY);
                    }
                    break;
                default:
                    break;
            }
        }

        surface_t *disp = display_get();

        rdpq_attach(disp, NULL);
        rdpq_clear(ASH_GRAY);

        rdpq_textparms_t textparms = {
            .width = 200, .tabstops = (int16_t[]){ 15 },
        };

        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,ENV,TEX0,ENV), (0,0,0,TEX0)));
        rdpq_set_prim_color(BREWFONT);  // fill color
        rdpq_set_env_color(BLACK);      // outline color
        rdpq_sprite_blit(logo, 35, 20, NULL);

        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_sprite_blit(jam, 35+190, 10, NULL);

        int x0 = 80;
        int y0 = 20 + logo->height + 20;

        if (yselect_target >= 0) {
            if (yselect < 0) yselect = yselect_target;
            yselect = yselect * 0.7 + yselect_target * 0.3;
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_set_prim_color(REDWOOD);
            rdpq_fill_rectangle(x0-10, yselect-12, x0+150, yselect+5);
        }

        rdpq_set_mode_standard();

        int ycur = y0;
        ycur += rdpq_text_print(&textparms, FONT_TEXT, x0-20, ycur, heading).advance_y;
        ycur += 4;

        for (int i = yscroll; i < item_count; i++) {
            if (ycur > 160) {
                if (select == i) {
                    yscroll += 1;                    
                }
                break;
            }

            if (select == i) yselect_target = ycur;

            switch (current_screen) {
            case SCREEN_PLAYERCOUNT:
                ycur += rdpq_text_printf(&textparms, FONT_TEXT, x0, ycur, "%d\n", i+1).advance_y;
                break;
            case SCREEN_AIDIFFICULTY:
                ycur += rdpq_text_printf(&textparms, FONT_TEXT, x0, ycur, "%s\n", get_difficulty_name(i)).advance_y;
                break;
            case SCREEN_MINIGAME:
                ycur += rdpq_text_printf(&textparms, FONT_TEXT, x0, ycur, "%d.\t%s\n", i+1, global_minigame_list[sorted_indices[i]].definition.gamename).advance_y;
                break;
            }
        }

        if (current_screen == SCREEN_MINIGAME) {
            // Show the description of the selected minigame
            rdpq_textparms_t parms = {
                .width = 300, .wrap = WRAP_WORD,
            };

            Minigame *cur = &global_minigame_list[sorted_indices[select]];

            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_set_prim_color(color_from_packed32(0x000000C7));
            rdpq_fill_rectangle(5, 158, 315, 237);

            rdpq_set_mode_standard();
            int y0 = 171;
            y0 += rdpq_text_printf(&parms, FONT_TEXT, 10, y0, "%s\n\n", cur->definition.description).advance_y;
            y0 += rdpq_text_printf(&parms, FONT_TEXT, 10, y0, "%s\n", cur->definition.instructions).advance_y;
        }

        if (true) {
            rdpq_text_printf(NULL, FONT_DEBUG, 10, 15, 
                "Mem: %d KiB", heap_stats.used/1024);
        }
        rdpq_detach_show();
    }

    is_first_time = false;

    rspq_wait();
    sprite_free(jam);
    sprite_free(logo);
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_text_unregister_font(FONT_DEBUG);
    rdpq_font_free(font);
    rdpq_font_free(fontdbg);
    display_close();
    core_set_playercount(playercount);
    core_set_aidifficulty(ai_difficulty);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Warray-bounds"
    return global_minigame_list[sorted_indices[selected_minigame]].internalname;
    #pragma GCC diagnostic pop
}
