/***************************************************************
                         examplegame.c
                               
An example minigame to demonstrate how to use the template for
the game jam.
***************************************************************/

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#define COUNTDOWN_DELAY 4.0f
#define FINISH_DELAY 10.0f

#define DEBUG_FONT 1

/*********************************
             Globals
*********************************/
// Display and T3D math globals
surface_t* depthBuffer;
T3DViewport viewport;
T3DMat4FP* mapMatFP;
T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 lightDirVec;
T3DModel* modelMap;

rspq_block_t* dplMap;
rspq_syncpoint_t syncPoint;

rdpq_font_t* debugFont;

// Sound globals
wav64_t sfx_start;
wav64_t sfx_countdown;

// Gameplay globals
float countdownTimer;
bool gameStarting;
bool gameEnding;

// You need this function defined somewhere in your project
// so that the minigame manager can work
const MinigameDef minigame_def = {
    .gamename = "RiPpEr's Game",
    .developername = "RiPpEr253",
    .description = "This is an example game.",
    .instructions = "Mash A to win."
};


/*==============================
    debugInfoDraw
    draws debug HUD info using the RDP
==============================*/

void debugInfoDraw(float deltaTime)
{
    rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, };
    rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, 10, "Test Debug");
    rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, 20, "FPS: %i", (int)(1.0f/deltaTime));
}

/*==============================
    player_init
    The player initialization function, determines number of 
    players and assigns AI to players that aren't human
==============================*/
void player_init();

/*==============================
    player_loop
    The scans player controllers and updates player entities
    accordingly
==============================*/
void player_loop(float deltaTime, int playerNumber)
{
    joypad_port_t controllerPort = core_get_playercontroller(playerNumber);
    
    if(!joypad_is_connected(controllerPort))
    {
        return;
    }

    joypad_buttons_t btn = joypad_get_buttons_held(controllerPort);

    if(btn.start) minigame_end();

    // move camera with stick test

    // stick left
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_X) == -1) camPos.v[0] += 1.0f;
    // stick right
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_X) == 1) camPos.v[0] -= 1.0f;
    // stick up
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_Y) == 1) camPos.v[1] += 1.0f;
    // stick down
    if(joypad_get_axis_held(controllerPort, JOYPAD_AXIS_STICK_Y) == -1) camPos.v[1] -= 1.0f;

    // rumble pak test code
    if(joypad_get_rumble_supported(controllerPort) && btn.a)
    {
        joypad_set_rumble_active(controllerPort, true);
    }

    if(joypad_get_rumble_supported(controllerPort) && btn.b)
    {
        joypad_set_rumble_active(controllerPort, false);
    }
}

/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    // initialise gameplay variables
    countdownTimer = COUNTDOWN_DELAY;
    gameStarting = true;
    gameEnding = false;

    // initialise the display, setting resolution, colour depth and AA
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    depthBuffer = display_get_zbuf();

    // start tiny3d
    t3d_init((T3DInitParams){});

    // load a font to use for HUD text
    debugFont = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(DEBUG_FONT, debugFont);

    // create the viewport
    viewport = t3d_viewport_create();

    // create a transformation matrix for the map
    mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0,0,-10});

    // set camera position and target vectors
    camPos = (T3DVec3){{0, 125.0f, 100.0f}};
    camTarget = (T3DVec3){{0, 0, 40}};

    // set up a vector for the directional light
    lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    // load a model from ROM for the map
    modelMap = t3d_model_load("rom:/rippergame/map.t3dm");

    // create a command block/display list to optimise drawing the map
    rspq_block_begin();
        t3d_matrix_push(mapMatFP);
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(modelMap);
        t3d_matrix_pop(1);
    dplMap = rspq_block_end();

    // clear the sync point
    syncPoint = 0;

    // load sounds
    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    mixer_ch_set_vol(31, 0.5f, 0.5f);

    return;
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
    if(gameStarting)
    {
        countdownTimer -= deltatime;

        if(countdownTimer < 0.0f)
        {
            wav64_play(&sfx_start, 31);
            gameStarting = false;
        }
    }
    return;
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/

void minigame_loop(float deltatime)
{

    // update the player entities
    for(int i = 0; i < core_get_playercount(); i++)
    {
        player_loop(deltatime, i);
    }

    // set up the ambient light colour and the directional light colour
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4] = {0xFF, 0xAA, 0xAA, 0xFF};

    // set the projection matrix for the viewport
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 260.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // Draw our 3D frame
    // grab the colour and depth buffers and attach
    // the rdp queue in order to dispatch commands to the RDP
    rdpq_attach(display_get(), depthBuffer);

    // tell tiny3d it's time to start a new frame
    t3d_frame_start();
    // attach the desired viewport
    t3d_viewport_attach(&viewport);

    // clear the colour and depth buffers
    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    // set the lighting details
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    // run the displaylist containing the map draw routine
    rspq_block_run(dplMap);

    // set a sync point
    syncPoint = rspq_syncpoint_new();

    // make sure the RDP is sync'd
    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise

    // draws RDP text on top of the scene showing debug info
    debugInfoDraw(deltatime);

    // game starting countdown text draw
    if(gameStarting)
    {
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, };
        rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, 100, "Starting in %i...", (int)countdownTimer);
    }
    
    //detach the queue to flip the buffers and show it on screen
    rdpq_detach_show();

    return;
}


/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    rspq_block_free(dplMap);

    t3d_model_free(modelMap);

    free_uncached(mapMatFP);

    rdpq_text_unregister_font(DEBUG_FONT);
    rdpq_font_free(debugFont);

    t3d_destroy();
    display_close();
    return;
}