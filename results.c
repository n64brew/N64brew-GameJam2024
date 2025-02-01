/***************************************************************
                           results.c
                               
The file contains the results screen
***************************************************************/

#include "results.h"
#include "core.h"
#include "menu.h"
#include "savestate.h"
#include <libdragon.h>
#include <limits.h>

#define FONT_TEXT               1
#define FONT_STYLE_DEFAULT      0
#define FONT_STYLE_WHITE        5

#define FADE_IN_DURATION        0.5f
#define BOX_ANIMATION_DELAY     0.2f
#define BOX_ANIMATION_DURATION  0.4f
#define TEXT_DELAY              0.7f
#define POINTS_DELAY            1.5f
#define POINTS_DURATION         0.3f
#define POINTS_SFX_DELAY        1.25f
#define CONFIRM_DELAY           2.0f
#define ANNOUNCE_DELAY          3.0f
#define FADE_OUT_DURATION       0.6f
#define FADE_OUT_POST_DELAY     0.2f

#define ANIM_CHOOSEPLAYER_MOVE     1.0f
#define ANIM_CHOOSEPLAYER_SELECT   4.0f
#define ANIM_CHOOSEPLAYER_SELECTED 5.0f
#define ANIM_CHOOSEPLAYER_DONE     6.0f

static int points_to_win;
static int global_points[MAXPLAYERS];

static bool ending;
static rdpq_font_t *font;

static sprite_t *bg_pattern;
static sprite_t *bg_gradient;
static sprite_t *btn_game;
static sprite_t *icon_player;
static sprite_t *icon_playerselected;
static sprite_t *icon_star;

static wav64_t sfx_point;
static wav64_t sfx_confirm;
static wav64_t sfx_winner;
static wav64_t sfx_roulette1;
static wav64_t sfx_roulette2;
static bool point_sfx_played;
static bool winner_sfx_played;
static bool selected_sfx_played;
static xm64player_t global_music;

static float gtime;

static float iconxpos;
static float confirm_start;
static bool fading_out;
static float fade_out_start;
static bool selectingnext;
static bool canbechosen[MAXPLAYERS];
static int currentlychosen;
static float chooseanim;
static float chooseanim;

static color_t player_colors[MAXPLAYERS];

int results_get_points(PlyNum player)
{
    return global_points[player];
}

void results_set_points(PlyNum player, int points)
{
    global_points[player] = points;
}

void results_reset_points()
{
    for (PlyNum i = 0; i < MAXPLAYERS; i++)
    {
        global_points[i] = 0;
    }
}

int results_get_points_to_win()
{
    return points_to_win;
}

void results_set_points_to_win(int points)
{
    points_to_win = points;
}

inline bool player_has_won(PlyNum player)
{
    return results_get_points(player) >= results_get_points_to_win();
}

static inline float lerp(float from, float to, float frac)
{
    return from + (to - from)*frac;
}

void results_init()
{
    ending = false;
    selectingnext = false;
    iconxpos = 30 + 10;
    currentlychosen = -1;
    chooseanim = 0;

    // Award points to winning players
    for (PlyNum i = 0; i < MAXPLAYERS; i++) {

        if (core_get_winner(i)) {
            results_set_points(i, results_get_points(i) + 1);
        }

        if (player_has_won(i)) {
            ending = true;
        }
    }

    // Check which players are eligible to be chosen next, and set them as the next one to choose
    if (core_get_nextround() == NR_LEAST)
    {
        int selected = 0;
        int choicecount = 0;
        int smallest = INT_MAX;
        for (int i=0; i<MAXPLAYERS; i++)
            if (results_get_points(i) < smallest)
                smallest = results_get_points(i);
        for (int i=0; i<MAXPLAYERS; i++)
        {
            bool valid = (results_get_points(i) == smallest);
            canbechosen[i] = valid;
            if (valid)
                choicecount++;
        }
        selected = rand() % choicecount;
        for (int i=0; i<MAXPLAYERS; i++)
        {
            if (canbechosen[i])
            {
                choicecount--;
                if (choicecount == selected)
                {
                    core_set_curchooser(i);
                    break;
                }
            }
        }
    }
    else if (core_get_nextround() == NR_ROBIN)
    {
        int lastchosen = core_get_curchooser();
        int nextchosen = (lastchosen + 1) % MAXPLAYERS;
        for (int i=0; i<MAXPLAYERS; i++)
            canbechosen[i] = (i == nextchosen);
        core_set_curchooser(nextchosen);
    }
    else if (core_get_nextround() == NR_RANDOMPLY)
    {
        for (int i=0; i<MAXPLAYERS; i++)
            canbechosen[i] = true;
        core_set_curchooser(rand() % MAXPLAYERS);
    }

    font = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, FONT_STYLE_DEFAULT, &(rdpq_fontstyle_t){.color = RGBA32(0xFF,0xDD,0xDD,0xFF), .outline_color = RGBA32(0x31,0x39,0x3C,0xFF) });
    rdpq_font_style(font, FONT_STYLE_WHITE, &(rdpq_fontstyle_t){.color = RGBA32(0xFF,0xFF,0xFF,0xFF) });

    player_colors[0] = PLAYERCOLOR_1;
    player_colors[1] = PLAYERCOLOR_2;
    player_colors[2] = PLAYERCOLOR_3;
    player_colors[3] = PLAYERCOLOR_4;

    for (size_t i = 0; i < MAXPLAYERS; i++)
        rdpq_font_style(font, i+1, &(rdpq_fontstyle_t){.color = player_colors[i]});

    bg_pattern = sprite_load("rom:/pattern.i8.sprite");
    bg_gradient = sprite_load("rom:/gradient.i8.sprite");
    btn_game = sprite_load("rom:/btnGame.i4.sprite");
    icon_player = sprite_load("rom:/iconPly.ia8.sprite");
    icon_playerselected = sprite_load("rom:/iconPlySelected.rgba32.sprite");
    icon_star = sprite_load("rom:/iconStar.ia8.sprite");

    wav64_open(&sfx_point, "rom:/core/Point.wav64");
    wav64_open(&sfx_confirm, "rom:/core/menu_confirm.wav64");
    wav64_open(&sfx_winner, "rom:/core/ResultsWinner.wav64");
    wav64_open(&sfx_roulette1, "rom:/core/RoulettePlayer.wav64");
    wav64_open(&sfx_roulette2, "rom:/core/RouletteDone.wav64");
    point_sfx_played = false;
    winner_sfx_played = false;
    selected_sfx_played = false;
    xm64player_open(&global_music, "rom:/core/Menus.xm64");
    xm64player_seek(&global_music, 37, 0, 0);
    xm64player_set_vol(&global_music, 0.0f);
    xm64player_play(&global_music, 0);

    gtime = 0;
    fading_out = false;

    confirm_start = ending ? ANNOUNCE_DELAY : 0.0f;

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
}

/**
 * @brief Draws scrolling background
 * @param pattern texture for the checkerboard pattern
 * @param gradient gradient on the Y axis
 * @param offset scroll offset
 */
static void menu_draw_bg(sprite_t *pattern, sprite_t *gradient, float offset, float fade)
{
    rdpq_set_mode_standard();
    rdpq_mode_begin();
    rdpq_mode_blender(0);
    rdpq_mode_alphacompare(0);
    rdpq_mode_combiner(RDPQ_COMBINER2(
        (TEX0, 0, TEX1, 0), (0, 0, 0, 1),
        (COMBINED, 0, PRIM, 0), (0, 0, 0, 1)
    ));
    rdpq_mode_dithering(DITHER_BAYER_BAYER);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_end();

    float brightness = 0.75f * fade;
    rdpq_set_prim_color((color_t){0xFF * brightness, 0xCC * brightness, 0xAA * brightness, 0xFF});

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

    rdpq_texture_rectangle(TILE0, 0, 0, display_get_width(), display_get_height(), 0, 0);
}

float ease_cubic_out(float t)
{
    float f;
    f = t - 1.0f;
    return f * f * f + 1.0f;
}


float ease_back_out(float t)
{
  float o, z, n;

  o = 1.70158f;
  n = t - 1.0f;
  z = (o + 1.0f) * n + o;

  return n * n * z + 1.0f;
}

float get_point_scale(PlyNum player, int index)
{
    int points = global_points[player];
    if (index >= points) return 0.0f;

    if (core_get_winner(player) && index == points-1) {
        if (gtime < POINTS_DELAY) return 0.0f;
        if (gtime > POINTS_DELAY + POINTS_DURATION) return 1.0f;
        float point_time = gtime - POINTS_DELAY;
        return ease_back_out(point_time/POINTS_DURATION);
    }

    return 1.0f;
}

void results_loop(float deltatime)
{
    gtime += deltatime;

    joypad_buttons_t btn[4] = {
        joypad_get_buttons_pressed(JOYPAD_PORT_1),
        joypad_get_buttons_pressed(JOYPAD_PORT_2),
        joypad_get_buttons_pressed(JOYPAD_PORT_3),
        joypad_get_buttons_pressed(JOYPAD_PORT_4),
    };

    surface_t *disp = display_get();

    rdpq_attach(disp, NULL);
    
    float bg_fade = gtime < FADE_IN_DURATION ? (gtime/FADE_IN_DURATION) : 1.0f;
    menu_draw_bg(bg_pattern, bg_gradient, gtime * 12.0f, bg_fade);
    if (fading_out)
        xm64player_set_vol(&global_music, (((1-(gtime-fade_out_start)/FADE_OUT_DURATION) < 0) ? 0 : (1-(gtime-fade_out_start)/FADE_OUT_DURATION)));
    else
        xm64player_set_vol(&global_music, ((gtime/FADE_IN_DURATION > 1) ? 1 : gtime/FADE_IN_DURATION));

    bool can_confirm = gtime - confirm_start > CONFIRM_DELAY;
    bool is_announcing = ending && gtime > ANNOUNCE_DELAY;

    if (gtime > POINTS_SFX_DELAY && !point_sfx_played) {
        bool someonescored = false;
        for (PlyNum i = 0; i < MAXPLAYERS; i++)
        {
            if (core_get_winner(i))
            {
                someonescored = true;
                break;
            }
        }
        if (someonescored)
            wav64_play(&sfx_point, 16);
        point_sfx_played = true;
    }

    if (is_announcing && gtime > ANNOUNCE_DELAY && !winner_sfx_played) {
        wav64_play(&sfx_winner, 16);
        xm64player_stop(&global_music);
        winner_sfx_played = true;
    }

    // Selection animation
    if (selectingnext)
    {
        int choosecount = 0;
        chooseanim += deltatime;
        iconxpos = lerp(iconxpos, (320/2) - 12, 4*deltatime);
        for (int i=0; i<MAXPLAYERS; i++)
        {
            if (!canbechosen[i])
                player_colors[i].a = lerp(player_colors[i].a, 32, 2*deltatime);
            else
                choosecount++;
        }

        // If only one player is selectible, skip the selection animation
        if (choosecount == 1 && chooseanim >= ANIM_CHOOSEPLAYER_MOVE && chooseanim < ANIM_CHOOSEPLAYER_SELECTED)
        {
            chooseanim = ANIM_CHOOSEPLAYER_SELECTED;
            currentlychosen = core_get_curchooser();
        }

        if (is_announcing)
            chooseanim = ANIM_CHOOSEPLAYER_DONE;

        // Do the selection animation
        if (chooseanim >= ANIM_CHOOSEPLAYER_MOVE && chooseanim < ANIM_CHOOSEPLAYER_SELECT) {
            int j = 0;
            int curchoice = ((int)((chooseanim - ANIM_CHOOSEPLAYER_MOVE)*(choosecount*3))) % (choosecount*3);
            int possible[choosecount];
            int finalchoice;
            for (int i=0; i<MAXPLAYERS; i++)
                if (canbechosen[i])
                    possible[j++] = i;
            finalchoice = possible[curchoice % choosecount];
            if (currentlychosen != finalchoice)
                wav64_play(&sfx_roulette1, 16);
            currentlychosen = finalchoice;
        } else if (chooseanim >= ANIM_CHOOSEPLAYER_SELECT && chooseanim < ANIM_CHOOSEPLAYER_SELECTED) {
            currentlychosen = core_get_curchooser();
            if (!selected_sfx_played)
            {
                wav64_play(&sfx_roulette2, 16);
                selected_sfx_played = true;
            }
        } else if (!fading_out && chooseanim >= ANIM_CHOOSEPLAYER_DONE) {
            fading_out = true;
            fade_out_start = gtime;
        }
    }

    // Box background
    int rect_width = 260;
    int rect_height = 180;

    float box_time = gtime - BOX_ANIMATION_DELAY;
    if (box_time < BOX_ANIMATION_DURATION) {
        float box_factor = ease_cubic_out(box_time/BOX_ANIMATION_DURATION);
        rect_width *= box_factor;
        rect_height *= box_factor;
    }

    int pos_x = display_get_width() / 2 - (rect_width / 2) - 8;
    int ycur = display_get_height() / 2 - (rect_height / 2) - 8;

    if (gtime > BOX_ANIMATION_DELAY) {
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

        rdpq_set_prim_color(RGBA32(0xAD,0xBA,0xBD,0xFF));
        rdpq_texture_rectangle(TILE1, // left, top
                            pos_x, ycur,
                            pos_x + rect_width, ycur + rect_height, 0, 0);
        rdpq_texture_rectangle(TILE1, // right, top
                            pos_x + rect_width + 14, ycur,
                            pos_x + rect_width - 1, ycur + rect_height, 0, 0);
        rdpq_texture_rectangle(TILE1, // left, bottom
                            pos_x, ycur + rect_height + 14,
                            pos_x + rect_width, ycur + rect_height - 1, 0, 0);
        rdpq_texture_rectangle(TILE1, // right, bottom
                            pos_x + rect_width + 14, ycur + rect_height + 14,
                            pos_x + rect_width - 1, ycur + rect_height - 1, 0, 0);
    }

    pos_x = 30;

    if (gtime > TEXT_DELAY) {
        ycur += 20;

        rdpq_textparms_t parms = {
            .align = ALIGN_CENTER,
            .width = 260
        };

        if (is_announcing) {
            int ytmp = ycur;
            ycur += 34;
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_blitparms_t blitparms = {
                .cx = 10,
                .cy = 10,
                .theta = fmodf(gtime*2.4f, FM_PI*2)
            };
            for (PlyNum i = 0; i < MAXPLAYERS; i++)
            {
                if (!player_has_won(i)) continue;
                rdpq_set_prim_color(player_colors[i]);
                rdpq_sprite_blit(icon_star, display_get_width()/2 - 50, ycur - 3, &blitparms);
                rdpq_sprite_blit(icon_star, display_get_width()/2 + 50, ycur - 3, &blitparms);
                ycur += 22;
            }

            ycur = ytmp;
            rdpq_set_mode_standard();
            rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "CONGRATULATIONS!");
            ycur += 34;
            for (PlyNum i = 0; i < MAXPLAYERS; i++)
            {
                if (!player_has_won(i)) continue;
                rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "Player %d", i+1);
                ycur += 22;
            }
            rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "won!");
        } else {
            int ytmp = ycur;

            ycur += 15;

            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            for (PlyNum i = 0; i < MAXPLAYERS; i++)
            {
                if (selectingnext && i == currentlychosen && chooseanim >= ANIM_CHOOSEPLAYER_MOVE)
                {
                    if (chooseanim >= ANIM_CHOOSEPLAYER_SELECTED)
                        rdpq_set_prim_color(RGBA32(255, 255, 255, (((int)((chooseanim-ANIM_CHOOSEPLAYER_SELECT)*4))%2)*255));
                    else
                        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                    rdpq_sprite_blit(icon_playerselected, iconxpos-4, ycur-4, NULL);
                }
                rdpq_mode_combiner(RDPQ_COMBINER1((TEX0,0,PRIM,0), (TEX0,0,PRIM,0)));
                rdpq_set_prim_color(player_colors[i]);
                rdpq_sprite_blit(icon_player, iconxpos, ycur, NULL);

                if (!selectingnext)
                {
                    for (int j = 0; j < points_to_win; j++)
                    {
                        rdpq_set_prim_color(RGBA32(0x20, 0x20, 0x20, 0xFF));
                        rdpq_sprite_blit(icon_player, pos_x + 10 + (j+1) * 30, ycur, NULL);

                        float point_scale = get_point_scale(i, j);
                        if (point_scale > 0.0f) {
                            rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
                            rdpq_sprite_blit(icon_star, pos_x + 22 + (j+1) * 30, ycur+12, &(rdpq_blitparms_t){
                                .cx = 10,
                                .cy = 10,
                                .scale_x = point_scale, 
                                .scale_y = point_scale,
                                .theta = fmodf(gtime*2.4f, FM_PI*2)
                            });
                        }
                    }
                }

                ycur += 30;
            }

            ycur = ytmp;
            rdpq_set_mode_standard();

            if (!selectingnext)
                ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "RESULTS\n").advance_y;
            else
                ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "SELECTING NEXT\n").advance_y;

            rdpq_textparms_t plyparms = {
                .style_id = FONT_STYLE_WHITE,
                .width = 24,
                .align = ALIGN_CENTER,
                .char_spacing = 1
            };

            for (PlyNum i = 0; i < MAXPLAYERS; i++)
            {
                rdpq_text_printf(&plyparms, FONT_TEXT, iconxpos, ycur + 16, "P%d", i+1);

                if (core_get_winner(i) && !selectingnext) {
                    rdpq_text_printf(&plyparms, FONT_TEXT, pos_x + (points_to_win+1) * 30 + 5, ycur + 16, "+1");
                }
                ycur += 30;
            }
        }

        if (can_confirm && !selectingnext) {
            rdpq_text_printf(&parms, FONT_TEXT, pos_x, 200, "Press A to continue");
        }
    }

    // Fade out
    if (fading_out) {
        float fade_time = gtime - fade_out_start;
        float fade_out_factor = (fade_time/FADE_OUT_DURATION);
        if (fade_out_factor > 1.0f) fade_out_factor = 1.0f;
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(RGBA32(0, 0, 0, fade_out_factor*0xFF));
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
    }

    rdpq_detach_show();

    bool confirm_pressed = btn[0].a || btn[1].a || btn[2].a || btn[3].a;
    if (can_confirm && !fading_out && confirm_pressed) {
        wav64_play(&sfx_confirm, 31);
        if (core_get_nextround() == NR_RANDOMGAME) {
            fading_out = true;
            fade_out_start = gtime;
        } else
            selectingnext = true;
    }

    if (fading_out && gtime > fade_out_start + FADE_OUT_DURATION + FADE_OUT_POST_DELAY) {
        if (ending) {
            results_reset_points();
            savestate_clear();
            core_level_changeto(LEVEL_MAINMENU);
        }
        else
            core_level_changeto(LEVEL_MINIGAMESELECT);
    }
}

void results_cleanup()
{
    rspq_wait();
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
    sprite_free(bg_pattern);
    sprite_free(bg_gradient);
    sprite_free(btn_game);
    sprite_free(icon_player);
    sprite_free(icon_playerselected);
    sprite_free(icon_star);
    wav64_close(&sfx_point);
    wav64_close(&sfx_confirm);
    wav64_close(&sfx_winner);
    wav64_close(&sfx_roulette1);
    wav64_close(&sfx_roulette2);
    xm64player_stop(&global_music);
    xm64player_close(&global_music);
    display_close();
}
