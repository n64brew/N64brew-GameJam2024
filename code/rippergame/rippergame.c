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
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

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

typedef enum
{
    teamThief,
    teamGuard
} player_team;

typedef struct
{
    int playerNumber;
    player_team playerTeam;
    T3DMat4FP* modelMatFP;
    T3DModel* model;
//    T3DAnim animAttack;
//    T3DAnim animWalk;
    T3DAnim animIdle;
    T3DSkeleton skelBlend;
    T3DSkeleton skel;
    rspq_block_t* dplPlayer;
    T3DVec3 moveDir;
    T3DVec3 playerPos;
    float rotY;
    float currSpeed;
    float animBlend;
    bool isAi;
    //TODO: Remove, temp AI variable
    int framesRemaining;
    T3DVec3 aiDir;
} player_data;

typedef struct
{
    bool isActive;
    T3DMat4FP* modelMatFP;
    T3DModel* model;
    rspq_block_t* dplObjective;
    T3DVec3 objectivePos;
} objective_data;

player_data players[MAXPLAYERS];
objective_data objectives[2];

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
    rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320,  .disable_aa_fix = true, };
    rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, 10, "Test Debug");
    rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, 20, "FPS: %f", /*(int)*/(1.0f/deltaTime));
    rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, 30, "p1 x: %f, p1 y: %f", players[0].playerPos.v[0], players[0].playerPos.v[2]);
}

/*==============================
    player_init
    The player initialization function, determines number of 
    players and assigns AI to players that aren't human
==============================*/
void player_init(int playerNumber)
{
    const color_t colours[] = {
    PLAYERCOLOR_1,
    PLAYERCOLOR_2,
    PLAYERCOLOR_3,
    PLAYERCOLOR_4,
    };

    if(playerNumber < core_get_playercount())
    {
        players[playerNumber].isAi = false;
    }
    else
    {
        players[playerNumber].isAi = true;
    }

    // remember that players are zero indexed
    if(playerNumber == 0 || playerNumber == 2)
    {
        players[playerNumber].playerTeam = teamThief;
        players[playerNumber].model = t3d_model_load("rom:/rippergame/testActor.t3dm");
    }
    else
    {
        players[playerNumber].playerTeam = teamGuard;
        players[playerNumber].model = t3d_model_load("rom:/rippergame/testActorGuard.t3dm");
    }

    switch(playerNumber)
    {
        case 0:
            players[playerNumber].playerPos = (T3DVec3){{-128, 0.0f, 128}};
        break;
        case 1:
            players[playerNumber].playerPos = (T3DVec3){{128, 0.0f, 128}};
        break;
        case 2:
            players[playerNumber].playerPos = (T3DVec3){{128, 0.0f, -128}};
        break;
        case 3:
            players[playerNumber].playerPos = (T3DVec3){{-128, 0.0f, -128}};
        default:
        break;
    }

    players[playerNumber].playerNumber = playerNumber;
    players[playerNumber].playerTeam = teamThief;
    players[playerNumber].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    // Instantiate skeletons, they will be used to draw skinned meshes
    players[playerNumber].skel = t3d_skeleton_create(players[playerNumber].model);
    players[playerNumber].skelBlend = t3d_skeleton_clone(&players[playerNumber].skel, false);
//    players[playerNumber].animAttack = t3d_anim_create(players[playerNumber].model, "Snake_Attack");
//    t3d_anim_set_looping(&players[playerNumber].animAttack, false);
//    t3d_anim_set_playing(&players[playerNumber].animAttack, false);
//    t3d_anim_attach(&players[playerNumber].animAttack, &players[playerNumber].skel);
//    players[playerNumber].animWalk = t3d_anim_create(players[playerNumber].model, "Snake_Walk");
//    t3d_anim_attach(&players[playerNumber].animWalk, &players[playerNumber].skel);
    players[playerNumber].animIdle = t3d_anim_create(players[playerNumber].model, "Actor_Idle");
    t3d_anim_attach(&players[playerNumber].animIdle, &players[playerNumber].skel);
    rspq_block_begin();
        t3d_matrix_push(players[playerNumber].modelMatFP);
        rdpq_set_prim_color(colours[playerNumber]);
        t3d_model_draw_skinned(players[playerNumber].model, &players[playerNumber].skel);
        t3d_matrix_pop(1);
    players[playerNumber].dplPlayer = rspq_block_end();
    players[playerNumber].moveDir = (T3DVec3){{0,0,0}};
    players[playerNumber].rotY = 0.0f;
    players[playerNumber].currSpeed = 0.0f;
    players[playerNumber].animBlend = 0.0f;
    players[playerNumber].framesRemaining = 0;
}

/*==============================
    player_cleanup
    The player cleanup function, frees and unloads
    anything that was allocated during the game
==============================*/

void player_cleanup(int playerNumber)
{
    rspq_block_free(players[playerNumber].dplPlayer);

    t3d_skeleton_destroy(&players[playerNumber].skel);
    t3d_skeleton_destroy(&players[playerNumber].skelBlend);

    t3d_anim_destroy(&players[playerNumber].animIdle);
//    t3d_anim_destroy(&players[playerNumber].animWalk);
//    t3d_anim_destroy(&players[playerNumber].animAttack);

    t3d_model_free(players[playerNumber].model);
    free_uncached(players[playerNumber].modelMatFP);
}

/*==============================
    player_fixedloop
    Updates players at the exact tickrate of 30 tps
==============================*/

void player_fixedloop(float deltaTime, int playerNumber)
{
    if(gameStarting)
    {
        return;
    }

    T3DVec3 newDir = {0};
    float speed = 0.0f;

    // only get controller inputs from actual players
    if(!players[playerNumber].isAi)
    {
        joypad_port_t controllerPort = core_get_playercontroller(playerNumber);
        joypad_inputs_t joypad = joypad_get_inputs(controllerPort);

        newDir.v[0] = (float)joypad.stick_x * 0.05f;
        newDir.v[2] = -(float)joypad.stick_y * 0.05f;
        speed = sqrtf(t3d_vec3_len2(&newDir));
    }
    else // is an AI
    {
        // super basic test AI
        if(players[playerNumber].framesRemaining <= 0)
        {
            newDir.v[0] = (rand()%170-85) * 0.05f;
            newDir.v[2] = (rand()%170-85) * 0.05f;

            players[playerNumber].aiDir = newDir;
            
            players[playerNumber].framesRemaining = rand()%600;
        }
        else
        {
            newDir = players[playerNumber].aiDir;
        
            players[playerNumber].framesRemaining--;
        }

        speed = sqrtf(t3d_vec3_len2(&newDir));
    }

    if(speed > 0.15f)
    {
        newDir.v[0] /= speed;
        newDir.v[2] /= speed;
        players[playerNumber].moveDir = newDir;

        float newAngle = atan2f(-players[playerNumber].moveDir.v[0], players[playerNumber].moveDir.v[2]);
        players[playerNumber].rotY = t3d_lerp_angle(players[playerNumber].rotY, newAngle, 0.5f);
        players[playerNumber].currSpeed = t3d_lerp(players[playerNumber].currSpeed, speed * 0.3f, 0.15f);
    }
    else
    {
        players[playerNumber].currSpeed *= 0.64f;
    }
    
    // move the player
    players[playerNumber].playerPos.v[0] += players[playerNumber].moveDir.v[0] * players[playerNumber].currSpeed;
    players[playerNumber].playerPos.v[2] += players[playerNumber].moveDir.v[2] * players[playerNumber].currSpeed;
    // and limit movement inside bounding box
    // do collision checks here
    const float BOX_SIZE = 140.0f;
    if(players[playerNumber].playerPos.v[0] < -BOX_SIZE)players[playerNumber].playerPos.v[0] = -BOX_SIZE;
    if(players[playerNumber].playerPos.v[0] > BOX_SIZE)players[playerNumber].playerPos.v[0] = BOX_SIZE;
    if(players[playerNumber].playerPos.v[2] < -BOX_SIZE)players[playerNumber].playerPos.v[2] = -BOX_SIZE;
    if(players[playerNumber].playerPos.v[2] > BOX_SIZE)players[playerNumber].playerPos.v[2] = BOX_SIZE;

}

/*==============================
    player_loop
    Updates players in any way that is not required to be
    on a fixed timebase
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

    t3d_anim_update(&players[playerNumber].animIdle, deltaTime);
    t3d_anim_set_speed(&players[playerNumber].animIdle, 1.0f);
//    t3d_anim_set_speed(&players[playerNumber].animWalk, players[playerNumber].animBlend + 0.15f);
//    t3d_anim_update(&players[playerNumber].animWalk, deltaTime);

    t3d_skeleton_blend(&players[playerNumber].skel, &players[playerNumber].skel, &players[playerNumber].skelBlend, players[playerNumber].animBlend);

    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

    t3d_skeleton_update(&players[playerNumber].skel);
    //update matrix
    t3d_mat4fp_from_srt_euler(players[playerNumber].modelMatFP,
        (float[3]){0.125f, 0.125f, 0.125f},
        (float[3]){0.0f, players[playerNumber].rotY, 0},
        players[playerNumber].playerPos.v);

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

void player_draw(int playerNumber)
{
    rspq_block_run(players[playerNumber].dplPlayer);
}

void objective_init()
{
    objectives[0].isActive = true;
    objectives[0].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    objectives[0].model = t3d_model_load("rom:/rippergame/testObjective.t3dm");
    rspq_block_begin();
        t3d_matrix_push(objectives[0].modelMatFP);
        rdpq_set_prim_color(RGBA32(0,255,0,255));
        t3d_model_draw(objectives[0].model);
        t3d_matrix_pop(1);
    objectives[0].dplObjective = rspq_block_end();
    objectives[0].objectivePos = (T3DVec3){{-64, 0.0f, 64}};
    
    objectives[1].isActive = true;
    objectives[1].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    objectives[1].model = t3d_model_load("rom:/rippergame/testObjective.t3dm");
    rspq_block_begin();
        t3d_matrix_push(objectives[1].modelMatFP);
        rdpq_set_prim_color(RGBA32(255,0,0,255));
        t3d_model_draw(objectives[1].model);
        t3d_matrix_pop(1);
    objectives[1].dplObjective = rspq_block_end();
    objectives[1].objectivePos = (T3DVec3){{64, 0.0f, -64}};
}

void objective_draw()
{
    //update matricies
    t3d_mat4fp_from_srt_euler(objectives[0].modelMatFP,
        (float[3]){0.125f, 0.125f, 0.125f},
        (float[3]){0.0f, 0.0f, 0.0f},
        objectives[0].objectivePos.v);
        t3d_mat4fp_from_srt_euler(objectives[1].modelMatFP,
        (float[3]){0.125f, 0.125f, 0.125f},
        (float[3]){0.0f, 0.0f, 0.0f},
        objectives[1].objectivePos.v);
    rspq_block_run(objectives[0].dplObjective);
    rspq_block_run(objectives[1].dplObjective);
}

void objective_cleanup()
{
    rspq_block_free(objectives[0].dplObjective);
    t3d_model_free(objectives[0].model);
    free_uncached(objectives[0].modelMatFP);

    rspq_block_free(objectives[1].dplObjective);
    t3d_model_free(objectives[1].model);
    free_uncached(objectives[1].modelMatFP);
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
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
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
    camPos = (T3DVec3){{0, 175.0f, 100.0f}};
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

    // load players
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_init(i);
    }

    // set up objectives
    objective_init();

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
    // update the player entities
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_fixedloop(deltatime, i);
    }

    // process pre-game countdown timer
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
    for(int i = 0; i < MAXPLAYERS; i++)
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

    // draw the player entities
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_draw(i);
    }

    // run the displaylist containing the map draw routine
    rspq_block_run(dplMap);

    // draw the objectives
    objective_draw();

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
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, .disable_aa_fix = true, };
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
    // cleanup players
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_cleanup(i);
    }

    // clenaup objectives
    objective_cleanup();

    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);

    rspq_block_free(dplMap);

    t3d_model_free(modelMap);

    free_uncached(mapMatFP);

    rdpq_text_unregister_font(DEBUG_FONT);
    rdpq_font_free(debugFont);

    t3d_destroy();
    display_close();
    return;
}