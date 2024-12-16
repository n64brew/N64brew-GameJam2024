#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "game.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#define ENABLE_MUSIC 1
#define ENABLE_TEXT 1
#define ENABLE_WIREFRAME 0
#define ENABLE_DEBUG_TEXT 0

#if ENABLE_WIREFRAME
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>
#endif

#define COUNTDOWN_DELAY     3.0f
#define GO_DELAY            1.0f
#define WIN_DELAY           5.0f
#define WIN_SHOW_DELAY      2.0f
#define MENU_DELAY          0.2f

#define GAME_BACKGROUND     0x666666ff
#define FONT_DEBUG          1
#define FONT_TEXT           2
#define TEXT_COLOR          0x6CBB3CFF


/*********************************
             Globals
*********************************/

const MinigameDef minigame_def = {
    .gamename = "Le tohu-bohu",
    .developername = "tfmoe__",
    .description = "Find the key and be the first to open the correct safe!",
    .instructions = "A: rummage through the furniture B: steal the key"
};

float countdown_timer;
bool is_ending;
float end_timer;

surface_t* depthBuffer;
T3DViewport viewport;
T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 camUp;
T3DVec3 lightDirVec;
rdpq_font_t* fontdbg;
rdpq_font_t *fonttext;
wav64_t music;
wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

int menu_option;
float menu_time;

#if ENABLE_WIREFRAME
bool show_wireframe = false;
#endif

#if ENABLE_DEBUG_TEXT
bool show_debug_text = false;
#endif


/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    // Init display
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    depthBuffer = display_get_zbuf();

#if ENABLE_WIREFRAME
    // Init OpenGL, for wireframe
    gl_init();
    float aspect_ratio = (float)display_get_width() / (float)display_get_height();
    float near_plane = 20.0f;
    float far_plane = 400.0f;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-near_plane*aspect_ratio, near_plane*aspect_ratio, -near_plane, near_plane, near_plane, far_plane);
#endif

    // Init 3D viewport
    t3d_init((T3DInitParams){});
    viewport = t3d_viewport_create();

    // Init camera and lighting
    camPos = (T3DVec3){{0, 160.0f, 200.0f}};
    camTarget = (T3DVec3){{0, 0, 40.0f}};
    camUp = (T3DVec3){{0, 1, 0}};
    lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

#if ENABLE_TEXT
    // Init fonts
    fontdbg = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_DEBUG, fontdbg);
    fonttext = rdpq_font_load("rom:/tohubohu/thickhead.font64");
    rdpq_text_register_font(FONT_TEXT, fonttext);
    rdpq_font_style(fonttext, 0, &(rdpq_fontstyle_t){.color = color_from_packed32(TEXT_COLOR) });
#endif

#if ENABLE_MUSIC
    // Init and play music
    wav64_open(&music, "rom:/tohubohu/music.wav64");
	wav64_set_loop(&music, true);
    mixer_ch_set_vol(1, 0.75f, 0.75f);
    wav64_play(&music, 1);
#endif

    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");

    game_init();
    
    countdown_timer = COUNTDOWN_DELAY;
    end_timer = 0;
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
    game_logic(deltatime);

    if (!is_paused()) {
        if (countdown_timer > 0) {
            float prev = countdown_timer;
            countdown_timer -= deltatime;
            if ((int)prev != (int)countdown_timer && countdown_timer >= 0) {
                wav64_play(&sfx_countdown, 31);
            }
        }
        if (!is_playing() && !is_ending && countdown_timer < 0) {
            start_game();
            wav64_play(&sfx_start, 31);
        }

        if (!is_ending) {
            if (has_winner()) {
                is_ending = true;
                stop_game();
                wav64_play(&sfx_stop, 31);
            }
        } else {
            float prev = end_timer;
            end_timer += deltatime;
            if ((int)prev != (int)end_timer && (int)end_timer == WIN_SHOW_DELAY) {
                wav64_play(&sfx_winner, 31);
            }
            if (end_timer > WIN_DELAY) {
                core_set_winner(winner());
                minigame_end();
            }
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
    joypad_port_t port = core_get_playercontroller(0);
    joypad_buttons_t pressed = joypad_get_buttons_pressed(port);
    joypad_inputs_t joypad = joypad_get_inputs(port);
#if ENABLE_WIREFRAME
    // Show/hide wireframe by pressing Z
    if (pressed.z) {
        show_wireframe = !show_wireframe;
    }
#endif
#if ENABLE_DEBUG_TEXT
    // Show/hide debug text by pressing R
    if (pressed.r) {
        show_debug_text = !show_debug_text;
    }
#endif

    // Toggle pause menu
    if (pressed.start) {
        menu_option = 0;
        toggle_pause();
    }

    // Handle menu
    if (is_paused()) {
        if (pressed.a) {
            if (menu_option == 0) {
                toggle_pause();
            } else if (menu_option == 1) {
                minigame_end();
            }
        }
        if (menu_time > 0) {
            menu_time -= deltatime;
        } else {
            if (pressed.d_up || joypad.stick_y < -50) {
                menu_option--;
                if (menu_option < 0) {
                    menu_option = 1;
                }
                menu_time = MENU_DELAY;
            }
            if (pressed.d_down || joypad.stick_y > 50) {
                menu_option++;
                if (menu_option > 1) {
                    menu_option = 0;
                }
                menu_time = MENU_DELAY;
            }
        }
    }
    

    // 3D viewport
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 200.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &camUp);

    // Render the UI
    rdpq_attach(display_get(), depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    t3d_screen_clear_color(color_from_packed32(GAME_BACKGROUND));
    t3d_screen_clear_depth();
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    game_render(deltatime, viewport);

#if ENABLE_WIREFRAME
    if (show_wireframe) {
        gl_context_begin();
        // Set the camera's position
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            camPos.v[0], camPos.v[1], camPos.v[2],
            camTarget.v[0], camTarget.v[1], camTarget.v[2],
            camUp.v[0], camUp.v[1], camUp.v[2]
        );
        game_render_gl(deltatime);
        gl_context_end();
    }
#endif

#if ENABLE_TEXT
    rdpq_sync_tile();
    rdpq_sync_pipe();

#if ENABLE_DEBUG_TEXT
    if (show_debug_text) {
        // Display FPS
        rdpq_text_printf(NULL, FONT_DEBUG, 10, 15, "FPS: %d", (int)display_get_fps());

        // Display game data
        rdpq_text_printf(NULL, FONT_DEBUG, 10, 30, "Key: %d", game_key());
        rdpq_text_printf(NULL, FONT_DEBUG, 10, 45, "Vault: %d", game_vault());
    }
#endif

    // Display winner
    if (is_ending && end_timer >= WIN_SHOW_DELAY) {
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, };
        rdpq_text_printf(&textparms, FONT_TEXT, 0, 100, "Player %d wins", winner()+1);
    }

    // Display pause menu
    if (is_paused()) {
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, };
        rdpq_text_printf(&textparms, FONT_TEXT, 0, 80, "%sResume", menu_option == 0 ? "> " : "  ");
        rdpq_text_printf(&textparms, FONT_TEXT, 0, 120, "%sQuit", menu_option == 1 ? "> " : "  ");
    }
#endif

    rdpq_detach_show();
}


/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    game_cleanup();
#if ENABLE_MUSIC
    wav64_close(&music);
#endif
    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);
#if ENABLE_TEXT
    rdpq_text_unregister_font(FONT_DEBUG);
    rdpq_font_free(fontdbg);
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(fonttext);
#endif
    t3d_destroy();
#if ENABLE_WIREFRAME
    gl_close();
#endif
    display_close();
}
