#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "game.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#define COUNTDOWN_DELAY     3.0f
#define GAME_BACKGROUND     0xffff00ff
#define FONT_DEBUG          1


/*********************************
             Globals
*********************************/

const MinigameDef minigame_def = {
    .gamename = "Rummage",
    .developername = "tfmoe__",
    .description = "Find the key and be the first to open the safe!",
    .instructions = "Press A to rummage through the furniture or to steal the key."
};

float countdown_timer;

surface_t* depthBuffer;
T3DViewport viewport;
T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 lightDirVec;


/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    // Init display and 3D viewport
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    depthBuffer = display_get_zbuf();
    t3d_init((T3DInitParams){});
    viewport = t3d_viewport_create();

    // Init camera and lighting
    camPos = (T3DVec3){{0, 200.0f, 80.0f}};
    camTarget = (T3DVec3){{0, 0, 0}};
    lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    // Init fonts
    rdpq_font_t *fontdbg = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_DEBUG, fontdbg);

    game_init();
    
    countdown_timer = COUNTDOWN_DELAY;
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

    if (countdown_timer > 0)
    {
        countdown_timer -= deltatime;
    }
}


/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/

void minigame_loop(float deltatime)
{
    // 3D viewport
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 160.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // Render the UI
    rdpq_attach(display_get(), depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    t3d_screen_clear_color(color_from_packed32(GAME_BACKGROUND));
    t3d_screen_clear_depth();
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    game_render(deltatime);

    // Display FPS
    rdpq_text_printf(NULL, FONT_DEBUG, 10, 15, "FPS: %d", (int)display_get_fps());

    rdpq_detach_show();
}


/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    game_cleanup();
    t3d_destroy();
    display_close();
}
