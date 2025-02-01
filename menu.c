/***************************************************************
                             menu.c
                               
This file contains the code for the basic menu
***************************************************************/

#include <libdragon.h>
#include <string.h>
#include "menu.h"
#include "core.h"
#include "minigame.h"
#include "config.h"
#include "results.h"
#include "savestate.h"


/*********************************
           Definitions
*********************************/

#define FONT_TEXT       1
#define FONT_DEBUG      2

#define FADETIME        0.5f

#define DEBUGINFO false

typedef enum
{
    SCREEN_MINIGAME
} menu_screen;

static bool is_first_time = false;

static menu_screen current_screen;  // Current menu screen
static int item_count;              // The number of selection items in the current screen
static int selected;                // The currently selected item
static int yscroll;
static int minigamecount;
static int global_lastplayed = 0;

// Locals that I now made global
static bool menu_done;
static bool menu_quit;
static bool has_moved_selection;
static float yselect;
static float yselect_target;
static color_t BLACK;
static color_t ASH_GRAY;
static color_t MAYA_BLUE;
static color_t GUN_METAL;
static color_t REDWOOD;
static color_t BREWFONT;
static color_t TEXT_COLOR;
static color_t WHITE;
static heap_stats_t heap_stats;
static sprite_t *bg_pattern;
static sprite_t *bg_gradient;
static sprite_t *btn_round;
static sprite_t *btn_wide;
static sprite_t *btn_game;
static sprite_t *slider;
static sprite_t* spr_a;
static int ai_target;
static bool ai_selected;
static float ai_nexttime;
static float roulette;

static rdpq_font_t *font;
static rdpq_font_t *fontdbg;
static int *minigame_indices;

static wav64_t sfx_cursor;
static wav64_t sfx_confirm;
static wav64_t sfx_back;
static wav64_t sfx_drumroll;
static wav64_t sfx_crash;
static xm64player_t global_music;

static float fadeouttime;
static float gtime;


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
    case JOYPAD_8WAY_LEFT:
        return -1;
    case JOYPAD_8WAY_DOWN_LEFT:
    case JOYPAD_8WAY_DOWN:
    case JOYPAD_8WAY_DOWN_RIGHT:
    case JOYPAD_8WAY_RIGHT:
        return 1;
    default:
        return 0;
    }
}

void menu_reset()
{
    is_first_time = true;
}

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
        case SCREEN_MINIGAME:
            item_count = minigamecount;
            break;
    }
}

/**
 * @brief Draws scrolling background
 * @param pattern texture for the checkerboard pattern
 * @param gradient gradient on the Y axis
 * @param offset scroll offset
 */
static void menu_draw_bg(sprite_t* pattern, sprite_t* gradient, float offset)
{
  rdpq_set_mode_standard();
  rdpq_mode_begin();
    rdpq_mode_blender(0);
    rdpq_mode_alphacompare(0);
    rdpq_mode_combiner(RDPQ_COMBINER2(
      (TEX0,0,TEX1,0), (0,0,0,1),
      (COMBINED,0,PRIM,0), (0,0,0,1)
    ));
    rdpq_mode_dithering(DITHER_BAYER_BAYER);
    rdpq_mode_filter(FILTER_BILINEAR);
  rdpq_mode_end();

  float brightness = 0.75f;
  rdpq_set_prim_color((color_t){0xFF*brightness, 0xCC*brightness, 0xAA*brightness, 0xFF});

  offset = fmodf(offset, 64.0f);
  rdpq_texparms_t param_pattern = {
    .s = {.repeats = REPEAT_INFINITE, .mirror = true, .translate = offset, .scale_log = 0},
    .t = {.repeats = REPEAT_INFINITE, .mirror = true, .translate = offset, .scale_log = 0},
  };
  rdpq_texparms_t param_grad = {
    .s = {.repeats = REPEAT_INFINITE},
    .t = {.repeats = 1, .scale_log = 2},
  };
  rdpq_tex_multi_begin();
    rdpq_sprite_upload(TILE0, pattern, &param_pattern);
    rdpq_sprite_upload(TILE1, gradient, &param_grad);
  rdpq_tex_multi_end();

  rdpq_texture_rectangle(TILE0, 0,0, display_get_width(), display_get_height(), 0, 0);
}

void menu_init()
{
    int j = 0;
    bool blacklist[global_minigame_count];
    gtime = 0.0f;
    menu_done = false;
    menu_quit = false;
    minigamecount = 0;
    fadeouttime = 0.0f;

    BLACK = RGBA32(0x00,0x00,0x00,0xFF);
    ASH_GRAY = RGBA32(0xAD,0xBA,0xBD,0xFF);
    MAYA_BLUE = RGBA32(0x6C,0xBE,0xED,0xFF);
    GUN_METAL = RGBA32(0x31,0x39,0x3C,0xFF);
    REDWOOD = RGBA32(0xB2,0x3A,0x7A,0xFF);
    BREWFONT = RGBA32(242,209,155,0xFF);
    TEXT_COLOR = RGBA32(0xFF,0xDD,0xDD,0xFF);
    WHITE = RGBA32(0xFF,0xFF,0xFF,0xFF);

    sys_get_heap_stats(&heap_stats);

    yselect = -1;
    yselect_target = -1;
    has_moved_selection = false;

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    bg_pattern = sprite_load("rom:/pattern.i8.sprite");
    bg_gradient = sprite_load("rom:/gradient.i8.sprite");
    btn_round = sprite_load("rom:/btnRound.ia8.sprite");
    btn_wide = sprite_load("rom:/btnWide.ia8.sprite");
    btn_game = sprite_load("rom:/btnGame.i4.sprite");
    slider = sprite_load("rom:/slider.ia4.sprite");
    spr_a = sprite_load("rom:/core/AButton.sprite");

    font = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = TEXT_COLOR, .outline_color = GUN_METAL });
    rdpq_font_style(font, 1, &(rdpq_fontstyle_t){.color = ASH_GRAY,  .outline_color = GUN_METAL });
    rdpq_font_style(font, 2, &(rdpq_fontstyle_t){.color = WHITE, .outline_color = GUN_METAL });
    rdpq_font_style(font, 3, &(rdpq_fontstyle_t){.color = BREWFONT, .outline_color = GUN_METAL });

    fontdbg = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_DEBUG, fontdbg);

    wav64_open(&sfx_cursor, "rom:/core/cursor.wav64");
    wav64_open(&sfx_confirm, "rom:/core/menu_confirm.wav64");
    wav64_open(&sfx_back, "rom:/core/menu_back.wav64");
    wav64_open(&sfx_drumroll, "rom:/core/DrumRoll.wav64");
    wav64_open(&sfx_crash, "rom:/core/Crash.wav64");
    xm64player_open(&global_music, "rom:/core/Menus.xm64");
    xm64player_seek(&global_music, 62, 0, 0);
    xm64player_set_vol(&global_music, 0.0f);
    xm64player_play(&global_music, 0);

    savestate_getblacklist(blacklist);
    for (int i = 0; i < global_minigame_count; i++)
        if (!blacklist[i])
            minigamecount++;
    minigame_indices = malloc(minigamecount * sizeof(int));
    // Minigames are globally sorted by name
    for (int i = 0; i < global_minigame_count; i++)
        if (!blacklist[i])
            minigame_indices[j++] = i;

    selected = global_lastplayed;

    // Handle automatic game selection
    ai_target = -1;
    ai_selected = false;
    roulette = 0.0f;
    if (core_get_nextround() != NR_FREEPLAY)
    {
        if ((core_get_nextround() != NR_ROBIN && (is_first_time || core_get_nextround() == NR_RANDOMGAME)) || core_get_curchooser() >= core_get_playercount())
        {
            if (core_get_nextround() != NR_ROBIN && (is_first_time || core_get_nextround() == NR_RANDOMGAME))
            {
                roulette = 2.0f;
                wav64_play(&sfx_drumroll, 30);
                ai_nexttime = 0.1f;
            }
            else
                ai_nexttime = 1.0f;
            ai_target = rand() % minigamecount;
        }
    }

    // Set the initial menu screen
    set_menu_screen(SCREEN_MINIGAME);
}


void menu_loop(float deltatime)
{
    int selection_offset = 0;
    bool a_pressed = false;
    bool b_pressed = false;

    // Handle controls
    if (!menu_done)
    {
        if (ai_target == -1)
        {
            for (int i=0; i<MAXPLAYERS; i++) {
                if (core_get_nextround() != NR_FREEPLAY && i != core_get_curchooser())
                    continue;
                joypad_buttons_t btn = joypad_get_buttons_pressed(core_get_playercontroller(PLAYER_1+i));
                if (btn.a) a_pressed = true;
                if (btn.b) b_pressed = true;
                selection_offset = get_selection_offset(joypad_get_direction(core_get_playercontroller(PLAYER_1+i), JOYPAD_2D_ANY));
                if (selection_offset != 0) break;
            }
        }
        else
        {
            ai_nexttime -= deltatime;
            if (roulette > 0)
            {
                roulette -= deltatime;
                if (roulette <= 0)
                {
                    wav64_play(&sfx_crash, 30);
                    selected = ai_target;
                    ai_nexttime = 2.0f;
                }
                else if (ai_nexttime <= 0)
                {
                    ai_nexttime = 0.1f;
                    selected = rand() % minigamecount;
                }
                yscroll = selected-1;
                if (selected == 0)
                    yscroll = 0;
                else if (selected == minigamecount-1)
                    yscroll = minigamecount - 3;
            }
            else if (ai_nexttime <= 0)
            {
                if (selected != ai_target)
                {
                    if (selected < ai_target)
                        selected++;
                    else
                        selected--;
                    wav64_play(&sfx_cursor, 31);
                    ai_nexttime = 0.5f;
                    if (selected == ai_target)
                        ai_nexttime = 2.0f;
                }
                else if (!ai_selected)
                {
                    wav64_play(&sfx_confirm, 30);
                    ai_selected = true;
                }
            }
        }

        if (selection_offset != 0) {
            if (!has_moved_selection) {
                selected += selection_offset;
                has_moved_selection = true;
                wav64_play(&sfx_cursor, 31);
            }
        } else {
            has_moved_selection = false;
        }

        if (selected < 0) selected = item_count-1;
        if (selected > item_count-1) selected = 0;

        if (roulette <= 0)
        {
            if (selected < yscroll) {
                yscroll -= 1;
            }
            else if (selected > yscroll+2) {
                yscroll += 1;
            }
        }

        if (ai_selected)
        {
            for (int i=0; i<MAXPLAYERS; i++) {
                joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1+i);
                if (btn.a)
                    a_pressed = true;
            }
        }

        if (a_pressed) {
            menu_done = true;
            fadeouttime = FADETIME;
            wav64_play(&sfx_confirm, 30);
        } else if (b_pressed && core_get_nextround() == NR_FREEPLAY) {
            menu_done = true;
            menu_quit = true;
            fadeouttime = FADETIME;
            wav64_play(&sfx_back, 30);
        }
    }

    gtime += deltatime;
    if (fadeouttime > 0)
    {
        fadeouttime -= deltatime;
        if (fadeouttime < 0)
            fadeouttime = 0;
    }
    surface_t *disp = display_get();

    rdpq_attach(disp, NULL);
    menu_draw_bg(bg_pattern, bg_gradient, gtime * 12.0f);

    rdpq_textparms_t textparmsCenter = {
        .align = ALIGN_CENTER,
        .width = 200,
    };

    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,ENV,TEX0,ENV), (0,0,0,TEX0)));
    rdpq_set_prim_color(BREWFONT);  // fill color
    rdpq_set_env_color(BLACK);      // outline color
    rdpq_mode_filter(FILTER_BILINEAR);

    int y0 = 38;
    if (core_get_nextround() != NR_FREEPLAY)
        y0 = 48;

    if(current_screen == SCREEN_MINIGAME) {
      int ycur = y0;

      rdpq_set_mode_standard();
      rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
      rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
      rdpq_mode_filter(FILTER_BILINEAR);

      rdpq_tex_multi_begin();
        rdpq_sprite_upload(TILE0, btn_game, &(rdpq_texparms_t){
          .s.repeats = 1, .t.repeats = 1,
        });
        rdpq_tex_reuse_sub(TILE1, &(rdpq_texparms_t){
          .s.repeats = 1, .t.repeats = 1,
        }, 0, 0, 16, 16);
      rdpq_tex_multi_end();
      
      int text_i = yscroll;
      int text_count = 0;
      float pos_x = display_get_width() / 2;

      for (int i = yscroll; i < item_count; i++) {
            if (ycur > 100) {
                if (selected == i) {
                    yscroll += 1;
                }
                break;
            }

            ++text_count;
            if (selected == i) yselect_target = ycur;

            rdpq_set_prim_color(selected == i ? TEXT_COLOR : ASH_GRAY);

            float btnScale = 1.0f;
            if(selected == i) {
              btnScale = 1.0f + (fm_sinf(gtime * 4.0f) * 0.04f);
            }

            int button_width = 80;
            float half_size_x = button_width * btnScale;
            float half_size_y = btn_game->height * 0.5f * btnScale;

            // left side
            rdpq_texture_rectangle_scaled(TILE0, 
              pos_x - half_size_x + 0, ycur - half_size_y, 
              pos_x + half_size_x - 8, ycur + half_size_y,
              0, 0, button_width * 2 - 8, btn_game->height
            );
            // end piece (flipped on X)
            rdpq_texture_rectangle_scaled(TILE0, 
              pos_x + half_size_x - 8, ycur - half_size_y, 
              pos_x + half_size_x + 0, ycur + half_size_y,
              8, 0, 0, btn_game->height
            );

            ycur += 24;
        }

        // Description box background
        ycur = y0 + 64;
        int rect_width = 260;
        int rect_height = 100;
        pos_x = display_get_width() / 2 - (rect_width/2)-8;

        rdpq_set_prim_color(ASH_GRAY);
        rdpq_texture_rectangle(TILE1, // left, top
          pos_x, ycur, 
          pos_x + rect_width, ycur+rect_height, 0, 0);
        rdpq_texture_rectangle(TILE1, // right, top
          pos_x + rect_width + 14, ycur, 
          pos_x + rect_width -  1, ycur+rect_height, 0, 0);
        rdpq_texture_rectangle(TILE1, // left, bottom
          pos_x,              ycur + rect_height + 14, 
          pos_x + rect_width, ycur + rect_height - 1, 0, 0);
        rdpq_texture_rectangle(TILE1, // right, bottom
          pos_x + rect_width + 14, ycur + rect_height + 14, 
          pos_x + rect_width -  1, ycur + rect_height - 1, 0, 0);
      
        
        for(int s=0; s<2; ++s) 
        {
          // Slider
          float slider_y = y0-10;
          float slider_x = s == 0 ? 60 : (display_get_width() - 60 - slider->width);
          rdpq_sprite_upload(TILE0, slider, &(rdpq_texparms_t){
            .s.repeats = 1, .t.repeats = 1,
          });
          rdpq_set_prim_color((color_t){0xAA, 0xAA, 0xAA, 0x99});
          rdpq_texture_rectangle(TILE0, 
            slider_x, slider_y, slider_x + slider->width, slider_y + slider->height, 0, 0
          );
          // point
          rdpq_set_prim_color(WHITE);
          slider_y += (float)selected / item_count * (slider->height);
          rdpq_texture_rectangle(TILE0, 
            slider_x, slider_y, slider_x + slider->width, slider_y + 3.5f, 0, 0 
          );
        }

        // Show the description of the selected minigame
        Minigame *cur = &global_minigame_list[minigame_indices[selected]];
        rdpq_textparms_t parms = {
            .width = rect_width + 10, 
            .wrap = WRAP_WORD,
            .align = ALIGN_RIGHT,
        };

        rdpq_set_mode_standard();

        rdpq_text_printf(&parms, FONT_TEXT, pos_x-4, ycur+rect_height+2, "^03© %s\n", cur->definition.developername);
        parms.align = ALIGN_LEFT;
        parms.width = rect_width;

        ycur += 16;
        ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x+10, ycur, "%s\n", cur->definition.description).advance_y;
        ycur += 6;
        ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x+10, ycur, "^02%s\n", cur->definition.instructions).advance_y;

      // Minigame names in the list
        pos_x = display_get_width() / 2;
        ycur = y0;
        for(int i=0; i<text_count; ++i) {
          int global_i = text_i+i;
          rdpq_text_printf(&textparmsCenter, FONT_TEXT, 
            pos_x-100, ycur+3, 
            selected == global_i ? "^00%s" : "^01%s",
            global_minigame_list[minigame_indices[global_i]].definition.gamename
          );
          ycur += 24;
        }
    }

    if (DEBUGINFO) {
        rdpq_text_printf(NULL, FONT_DEBUG, 10, 15, 
            "Mem: %d KiB", heap_stats.used/1024);
    }
    if (core_get_nextround() != NR_FREEPLAY && !menu_done)
    {
        if (ai_selected)
        {
            if (((int)(gtime*4))%2 == 0)
            {
                rdpq_set_mode_standard();
                rdpq_mode_combiner(RDPQ_COMBINER1((TEX0,0,PRIM,0), (TEX0,0,PRIM,0)));
                rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                rdpq_sprite_blit(spr_a, 185, 24-11, NULL);
                rdpq_text_print(&(rdpq_textparms_t){.width = 320, .align = ALIGN_CENTER}, FONT_TEXT, 0, 24, "Game selected, press      to begin");
            }
        }
        else if (core_get_nextround() != NR_ROBIN && (is_first_time || core_get_nextround() == NR_RANDOMGAME))
            rdpq_text_print(&(rdpq_textparms_t){.width = 320, .align = ALIGN_CENTER}, FONT_TEXT, 0, 24, "Random game being selected");
        else
            rdpq_text_printf(&(rdpq_textparms_t){.width = 320, .align = ALIGN_CENTER}, FONT_TEXT, 0, 24, "Player %d selecting game", core_get_curchooser()+1);
    }

    // Fade in
    if (gtime < FADETIME)
    {
        xm64player_set_vol(&global_music, (gtime/FADETIME));
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_set_prim_color(RGBA32(0, 0, 0, 255*(1-(gtime/FADETIME))));
        rdpq_fill_rectangle(0, 0, 320, 240);
    }

    // Fade out
    if (menu_done)
    {
        xm64player_set_vol(&global_music, fadeouttime/FADETIME);
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_set_prim_color(RGBA32(0, 0, 0, 255*(1-(fadeouttime/FADETIME))));
        rdpq_fill_rectangle(0, 0, 320, 240);
    }    
        
    rdpq_detach_show();

    if (menu_done && fadeouttime <= 0)
    {
        if (!menu_quit)
        {
            is_first_time = false;
            global_lastplayed = selected;
            minigame_loadnext(global_minigame_list[minigame_indices[selected]].internalname);
            if (core_get_nextround() != NR_FREEPLAY)
                savestate_save(false);
            core_level_changeto(LEVEL_MINIGAME);
        }
        else
            core_level_changeto(LEVEL_MAINMENU);
    }
}

void menu_cleanup()
{
    free(minigame_indices);
    rspq_wait();
    
    sprite_free(bg_pattern);
    sprite_free(bg_gradient);
    sprite_free(btn_round);
    sprite_free(btn_wide);
    sprite_free(btn_game);
    sprite_free(slider);
    sprite_free(spr_a);

    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_text_unregister_font(FONT_DEBUG);
    rdpq_font_free(font);
    rdpq_font_free(fontdbg);

    wav64_close(&sfx_cursor);
    wav64_close(&sfx_confirm);
    wav64_close(&sfx_back);
    wav64_close(&sfx_drumroll);
    wav64_close(&sfx_crash);
    xm64player_stop(&global_music);
    xm64player_close(&global_music);

    display_close();
}
