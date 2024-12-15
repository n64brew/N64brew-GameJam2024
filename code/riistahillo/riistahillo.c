#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

const MinigameDef minigame_def = {
    .gamename = "The Third Arm",
    .developername = "Riistahillo",
    .description = "Twister for your fingers.",
    .instructions = "Hold or smash highlighted buttons. Do not press other buttons."
};

typedef struct
{
    bool currentState;
    bool isSmash;
    bool wasButtonDown;
    float holdDuration;
    float releaseDuration;
} PlayerInput;

typedef struct
{
    bool isCpu;
    float cpuTimer;
    float hp;
    bool isOk;
    unsigned int dLeft;
    unsigned int dDown;
    unsigned int dRight;
    unsigned int dUp;
    int8_t sx;
    int8_t sy;
    PlayerInput a;
    PlayerInput b;
    PlayerInput l;
    PlayerInput z;
    PlayerInput r;
    PlayerInput cLeft;
    PlayerInput cDown;
    PlayerInput cRight;
    PlayerInput cUp;
} Player;
Player players[MAXPLAYERS];

#define TARGET_RELEASE  0
#define TARGET_HOLD     1
#define TARGET_SMASH    2
typedef struct
{
    // 0: Release
    // 1: Hold
    // 2: Smash
    int type;

    // dPad and stick uses variants for directions
    int variant;
} InputTarget;

typedef struct
{
    float timer;
    InputTarget dPad;
    InputTarget stick;
    InputTarget a;
    InputTarget b;
    InputTarget l;
    InputTarget z;
    InputTarget r;
    InputTarget cLeft;
    InputTarget cDown;
    InputTarget cRight;
    InputTarget cUp;
} Target;
Target target;

#define FONT_TEXT           1
#define FONT_P1             2
#define FONT_P2             3
#define FONT_P3             4
#define FONT_P4             5
#define TEXT_COLOR          0xFFFFFFFF
#define TEXT_OUTLINE        0x000000FF
#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240

const float smashMaxTime = 0.5f;
const float maxHp = 100.0f;
const float textGap = 13;
float newTargetRate;
float smashTimer;
bool smashShow;
float animValuePosY;
float animValueRotY;
float animValueRotZ;
float posY;
float rotY;
float rotZ;
float bgColorR;
float bgColorG;
float bgColorB;
bool colorFadeDown;
bool gameEnded;
int holds;
float countDownTimer;
bool playMusic;
uint32_t playercount;
bool fastForward;

surface_t *depthBuffer;
T3DViewport viewport;
rdpq_font_t *font;
rdpq_font_t *fontP1;
rdpq_font_t *fontP2;
rdpq_font_t *fontP3;
rdpq_font_t *fontP4;
T3DMat4FP *controllerMatFP;
rspq_block_t *dplController;
rspq_block_t *dplControllerDPadLeft;
rspq_block_t *dplControllerDPadDown;
rspq_block_t *dplControllerDPadRight;
rspq_block_t *dplControllerDPadUp;
rspq_block_t *dplControllerStickLeft;
rspq_block_t *dplControllerStickDown;
rspq_block_t *dplControllerStickRight;
rspq_block_t *dplControllerStickUp;
rspq_block_t *dplControllerPressA;
rspq_block_t *dplControllerPressB;
rspq_block_t *dplControllerPressL;
rspq_block_t *dplControllerPressZ;
rspq_block_t *dplControllerPressR;
rspq_block_t *dplControllerPressCLeft;
rspq_block_t *dplControllerPressCDown;
rspq_block_t *dplControllerPressCRight;
rspq_block_t *dplControllerPressCUp;
T3DModel *modelController;
T3DModel *modelPressDPadLeft;
T3DModel *modelPressDPadDown;
T3DModel *modelPressDPadRight;
T3DModel *modelPressDPadUp;
T3DModel *modelPressStickLeft;
T3DModel *modelPressStickDown;
T3DModel *modelPressStickRight;
T3DModel *modelPressStickUp;
T3DModel *modelPressA;
T3DModel *modelPressB;
T3DModel *modelPressL;
T3DModel *modelPressZ;
T3DModel *modelPressR;
T3DModel *modelPressCLeft;
T3DModel *modelPressCDown;
T3DModel *modelPressCRight;
T3DModel *modelPressCUp;
T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 lightDirVec;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;
wav64_t sfx_music;

rspq_syncpoint_t syncPoint;

void displayButtonTarget(const rdpq_textparms_t *parms, int type, float x, float y, const char *button)
{
    rdpq_text_printf(parms, FONT_TEXT, x, y, button);
    if(type == TARGET_RELEASE)
    {
        rdpq_text_printf(parms, FONT_TEXT, x + 28, y, "R");
    }
    else if(type == TARGET_HOLD)
    {
        rdpq_text_printf(parms, FONT_TEXT, x + 28, y, "H");
    }
    else if (type == TARGET_SMASH)
    {
        rdpq_text_printf(parms, FONT_TEXT, x + 28, y, "S");
    }
}

int returnTarget(int currentTarget)
{
    int r = rand() % 2;
    holds++;

    if(r == 0 || holds < 5)
    {
        if(currentTarget == TARGET_HOLD)
        {
            return TARGET_RELEASE;
        }
        return TARGET_HOLD;
    }

    if(currentTarget == TARGET_SMASH)
    {
        return TARGET_RELEASE;
    }
    return TARGET_SMASH;
}

void updatePlayerInput(PlayerInput *playerButton, float deltaTime)
{
    if(playerButton->currentState)
    {
        playerButton->wasButtonDown = true;
        playerButton->holdDuration += deltaTime;
        if(playerButton->holdDuration > smashMaxTime)
        {
            playerButton->isSmash = false;
        }
    }
    else if(playerButton->wasButtonDown && !playerButton->currentState)
    {
        if(playerButton->holdDuration < smashMaxTime)
        {
            playerButton->isSmash = true;
        }
        else
        {
            playerButton->isSmash = false;
        }
        playerButton->wasButtonDown = false;
        playerButton->holdDuration = 0.0f;
        playerButton->releaseDuration = 0.0f;
    }
    
    if(!playerButton->currentState)
    {
        playerButton->releaseDuration += deltaTime;
        if(playerButton->releaseDuration > smashMaxTime)
        {
            playerButton->isSmash = false;
        }
    }
}

void checkPlayerTarget(int targetType, PlayerInput *playerButton, int playerIndex)
{
    if(targetType == TARGET_RELEASE)
    {
        if(playerButton->currentState)
        {
            players[playerIndex].isOk = false;
        }
    }
    else if (targetType == TARGET_HOLD)
    {
        if(!playerButton->currentState)
        {
            players[playerIndex].isOk = false;
        }
    }
    else if (targetType == TARGET_SMASH)
    {
        if(!playerButton->isSmash)
        {
            players[playerIndex].isOk = false;
        }
    }
}

rspq_block_t* AddModelToBlock(T3DModel *model)
{
    rspq_block_begin();
    t3d_matrix_push(controllerMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(model);
    t3d_matrix_pop(1);
    return rspq_block_end();
}

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    smashTimer = 0.0f;
    smashShow = false;
    animValuePosY = 0.0f;
    animValueRotY = 0.0f;
    animValueRotZ = 0.0f;
    posY = 0.0f;
    rotY = 0.0f;
    rotZ = 0.0f;
    bgColorR = 0.0f;
    bgColorG = 0.0f;
    bgColorB = 0.0f;
    colorFadeDown = true;
    gameEnded = false;
    holds = 0;
    countDownTimer = 1.0f;
    playMusic = false;
    playercount = 0;
    fastForward = false;

    playercount = core_get_playercount();
    for(int i = 0; i < MAXPLAYERS; ++i)
    {
        players[i].hp = maxHp;
        players[i].isCpu = i >= (playercount);
        if(core_get_aidifficulty() == DIFF_EASY)
        {
            players[i].cpuTimer = 15.0f + (float)(rand() % 15);
        }
        else if (core_get_aidifficulty() == DIFF_MEDIUM)
        {
            players[i].cpuTimer = 20.0f + (float)(rand() % 20);
        }
        else if (core_get_aidifficulty() == DIFF_HARD)
        {
            players[i].cpuTimer = 40.0f + (float)(rand() % 40);
        }
    }

    target.timer = 0.0f;
    target.dPad.type = TARGET_RELEASE;
    target.stick.type = TARGET_RELEASE;
    target.a.type = TARGET_RELEASE;
    target.b.type = TARGET_RELEASE;
    target.l.type = TARGET_RELEASE;
    target.z.type = TARGET_RELEASE;
    target.r.type = TARGET_RELEASE;
    target.cLeft.type = TARGET_RELEASE;
    target.cDown.type = TARGET_RELEASE;
    target.cRight.type = TARGET_RELEASE;
    target.cUp.type = TARGET_RELEASE;

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    depthBuffer = display_get_zbuf();

    t3d_init((T3DInitParams){});

    font = rdpq_font_load("rom:/riistahillo/ComicNeue-Regular.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = color_from_packed32(TEXT_COLOR)});

    fontP1 = rdpq_font_load("rom:/riistahillo/ComicNeue-Regular.font64");
    rdpq_text_register_font(FONT_P1, fontP1);
    rdpq_font_style(fontP1, 0, &(rdpq_fontstyle_t){.color = PLAYERCOLOR_1});

    fontP2 = rdpq_font_load("rom:/riistahillo/ComicNeue-Regular.font64");
    rdpq_text_register_font(FONT_P2, fontP2);
    rdpq_font_style(fontP2, 0, &(rdpq_fontstyle_t){.color = PLAYERCOLOR_2});

    fontP3 = rdpq_font_load("rom:/riistahillo/ComicNeue-Regular.font64");
    rdpq_text_register_font(FONT_P3, fontP3);
    rdpq_font_style(fontP3, 0, &(rdpq_fontstyle_t){.color = PLAYERCOLOR_3});

    fontP4 = rdpq_font_load("rom:/riistahillo/ComicNeue-Regular.font64");
    rdpq_text_register_font(FONT_P4, fontP4);
    rdpq_font_style(fontP4, 0, &(rdpq_fontstyle_t){.color = PLAYERCOLOR_4});

    modelController = t3d_model_load("rom:/riistahillo/controller.t3dm");
    modelPressDPadLeft = t3d_model_load("rom:/riistahillo/pressdpadl.t3dm");
    modelPressDPadDown = t3d_model_load("rom:/riistahillo/pressdpadd.t3dm");
    modelPressDPadRight = t3d_model_load("rom:/riistahillo/pressdpadr.t3dm");
    modelPressDPadUp = t3d_model_load("rom:/riistahillo/pressdpadu.t3dm");
    modelPressStickLeft = t3d_model_load("rom:/riistahillo/presssl.t3dm");
    modelPressStickDown = t3d_model_load("rom:/riistahillo/presssd.t3dm");
    modelPressStickRight = t3d_model_load("rom:/riistahillo/presssr.t3dm");
    modelPressStickUp = t3d_model_load("rom:/riistahillo/presssu.t3dm");
    modelPressA = t3d_model_load("rom:/riistahillo/pressa.t3dm");
    modelPressB = t3d_model_load("rom:/riistahillo/pressb.t3dm");
    modelPressL = t3d_model_load("rom:/riistahillo/pressl.t3dm");
    modelPressZ = t3d_model_load("rom:/riistahillo/pressz.t3dm");
    modelPressR = t3d_model_load("rom:/riistahillo/pressr.t3dm");
    modelPressCLeft = t3d_model_load("rom:/riistahillo/presscl.t3dm");
    modelPressCDown = t3d_model_load("rom:/riistahillo/presscd.t3dm");
    modelPressCRight = t3d_model_load("rom:/riistahillo/presscr.t3dm");
    modelPressCUp = t3d_model_load("rom:/riistahillo/presscu.t3dm");

    viewport = t3d_viewport_create();

    controllerMatFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(controllerMatFP, (float[3]){0.0f, 0.0f, 0.0f}, (float[3]){0.0f, 0.0f, 0.0f}, (float[3]){0.0f, 0.0f, 0.0f});

    dplController = AddModelToBlock(modelController);
    dplControllerDPadLeft = AddModelToBlock(modelPressDPadLeft);
    dplControllerDPadDown = AddModelToBlock(modelPressDPadDown);
    dplControllerDPadRight = AddModelToBlock(modelPressDPadRight);
    dplControllerDPadUp = AddModelToBlock(modelPressDPadUp);
    dplControllerStickLeft = AddModelToBlock(modelPressStickLeft);
    dplControllerStickDown = AddModelToBlock(modelPressStickDown);
    dplControllerStickRight = AddModelToBlock(modelPressStickRight);
    dplControllerStickUp = AddModelToBlock(modelPressStickUp);
    dplControllerPressA = AddModelToBlock(modelPressA);
    dplControllerPressB = AddModelToBlock(modelPressB);
    dplControllerPressL = AddModelToBlock(modelPressL);
    dplControllerPressZ = AddModelToBlock(modelPressZ);
    dplControllerPressR = AddModelToBlock(modelPressR);
    dplControllerPressCLeft = AddModelToBlock(modelPressCLeft);
    dplControllerPressCDown = AddModelToBlock(modelPressCDown);
    dplControllerPressCRight = AddModelToBlock(modelPressCRight);
    dplControllerPressCUp = AddModelToBlock(modelPressCUp);

    camPos = (T3DVec3){{0.0f, 0.0f, 0.0f}};
    camTarget = (T3DVec3){{0.0f, 0.0f, 1.0f}};

    lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    syncPoint = 0;
    wav64_open(&sfx_start, "rom:/riistahillo/bop.wav64");
    wav64_open(&sfx_countdown, "rom:/riistahillo/bap.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");
    wav64_open(&sfx_music, "rom:/riistahillo/jam.wav64");
    mixer_ch_set_vol(31, 0.5f, 0.5f);
}

void RenderPressEffect(int type, rspq_block_t *block)
{
    if(type == TARGET_HOLD)
    {
        rspq_block_run(block);
    }
    else if(type == TARGET_SMASH && smashShow)
    {
        rspq_block_run(block);
    }
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

}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 50.0f, 150.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    rdpq_attach(display_get(), depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    if(!gameEnded)
    {
        const float colorFadeSpeed = 500.0f;
        bgColorR = 255.0f;
        if(colorFadeDown)
        {
            bgColorG += deltatime * colorFadeSpeed;
            bgColorB -= deltatime * colorFadeSpeed;
            if(bgColorG > 255.0f)
            {
                colorFadeDown = false;
                bgColorG = 255.0f;
                bgColorB = 0.0f;
            }
        }
        else
        {
            bgColorG -= deltatime * colorFadeSpeed;
            bgColorB += deltatime * colorFadeSpeed;
            if(bgColorG < 0.0f)
            {
                colorFadeDown = true;
                bgColorG = 0.0f;
                bgColorB = 255.0f;
            }
        }

        if(fastForward)
        {
            deltatime *= 5.0f;
        }
    }
    
    t3d_screen_clear_color(RGBA32((int)bgColorR, (int)bgColorG, (int)bgColorB, 0xFF));
    t3d_screen_clear_depth();

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    animValuePosY += deltatime;
    animValueRotY += deltatime * 0.66f;
    animValueRotZ += deltatime * 0.75f;
    posY = fm_sinf(animValuePosY) * 10.0f;
    rotY = fm_sinf(animValueRotY) * 0.1f;
    rotZ = fm_sinf(animValueRotZ) * 0.1f;
    t3d_mat4fp_from_srt_euler(controllerMatFP, (float[3]){0.9f, 0.9f, 0.9f}, (float[3]){0, 0.15f + rotY, rotZ}, (float[3]){-23.0f, -20.0f - posY, 100.0f});

    rspq_block_run(dplController);

    smashTimer += deltatime;
    if(smashTimer > 0.15f)
    {
        smashTimer = 0.0f;
        smashShow = !smashShow;
    }

    if(target.dPad.variant == 0) RenderPressEffect(target.dPad.type, dplControllerDPadLeft);
    else if (target.dPad.variant == 1) RenderPressEffect(target.dPad.type, dplControllerDPadDown);
    else if (target.dPad.variant == 2) RenderPressEffect(target.dPad.type, dplControllerDPadRight);
    else if (target.dPad.variant == 3) RenderPressEffect(target.dPad.type, dplControllerDPadUp);

    if(target.stick.variant == 0) RenderPressEffect(target.stick.type, dplControllerStickLeft);
    else if (target.stick.variant == 1) RenderPressEffect(target.stick.type, dplControllerStickDown);
    else if (target.stick.variant == 2) RenderPressEffect(target.stick.type, dplControllerStickRight);
    else if (target.stick.variant == 3) RenderPressEffect(target.stick.type, dplControllerStickUp);

    RenderPressEffect(target.a.type, dplControllerPressA);
    RenderPressEffect(target.b.type, dplControllerPressB);
    RenderPressEffect(target.l.type, dplControllerPressL);
    RenderPressEffect(target.z.type, dplControllerPressZ);
    RenderPressEffect(target.r.type, dplControllerPressR);

    RenderPressEffect(target.cLeft.type, dplControllerPressCLeft);
    RenderPressEffect(target.cDown.type, dplControllerPressCDown);
    RenderPressEffect(target.cRight.type, dplControllerPressCRight);
    RenderPressEffect(target.cUp.type, dplControllerPressCUp);

    syncPoint = rspq_syncpoint_new();

    rdpq_sync_pipe(); 

    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise

    rdpq_textparms_t textparms = {.align = ALIGN_LEFT, .width = SCREEN_WIDTH};

    const int offsetX = 32;
    const int offsetY = 32;
    const int plaeryUiGap = 44;

    int playerAlive = MAXPLAYERS;
    int nonAiAlive = playercount;
    for(int i = 0; i < MAXPLAYERS; ++i)
    {
        if(players[i].isCpu)
        {
            rdpq_text_printf(&textparms, FONT_TEXT + (i + 1), offsetX, offsetY + i * plaeryUiGap, "Player %d CPU", i + 1);
        }
        else
        {
            rdpq_text_printf(&textparms, FONT_TEXT + (i + 1), offsetX, offsetY + i * plaeryUiGap, "Player %d", i + 1);
        }
        
        // Get player inputs
        joypad_inputs_t joypad = joypad_get_inputs(core_get_playercontroller(i));

        players[i].dLeft = joypad.btn.d_left;
        players[i].dDown = joypad.btn.d_down;
        players[i].dRight = joypad.btn.d_right;
        players[i].dUp = joypad.btn.d_up; 

        players[i].sx = joypad.stick_x;
        players[i].sy = joypad.stick_y;

        players[i].a.currentState = joypad.btn.a;
        players[i].b.currentState = joypad.btn.b;
        players[i].l.currentState = joypad.btn.l;
        players[i].z.currentState = joypad.btn.z;
        players[i].r.currentState = joypad.btn.r;

        players[i].cLeft.currentState = joypad.btn.c_left;
        players[i].cUp.currentState = joypad.btn.c_up;
        players[i].cRight.currentState = joypad.btn.c_right;
        players[i].cDown.currentState = joypad.btn.c_down;

        updatePlayerInput(&players[i].a, deltatime);
        updatePlayerInput(&players[i].b, deltatime);
        updatePlayerInput(&players[i].l, deltatime);
        updatePlayerInput(&players[i].z, deltatime);
        updatePlayerInput(&players[i].r, deltatime);

        updatePlayerInput(&players[i].cLeft, deltatime);
        updatePlayerInput(&players[i].cUp, deltatime);
        updatePlayerInput(&players[i].cRight, deltatime);
        updatePlayerInput(&players[i].cDown, deltatime);

        // Check player input against target
        players[i].isOk = true;

        if(target.dPad.type == TARGET_RELEASE)
        {
            if(players[i].dLeft || players[i].dDown || players[i].dRight || players[i].dUp)
            {
                players[i].isOk = false;
            }
        }
        else if (target.dPad.type == TARGET_HOLD)
        {
            if(target.dPad.variant == 0 && !players[i].dLeft)
            {
                players[i].isOk = false;
            }
            else if(target.dPad.variant == 1 && !players[i].dDown)
            {   
                players[i].isOk = false;
            }
            else if(target.dPad.variant == 2 && !players[i].dRight)
            {
                players[i].isOk = false;
            }
            else if(target.dPad.variant == 3 && !players[i].dUp)
            {
                players[i].isOk = false;
            }
        }

        if(target.stick.type == TARGET_RELEASE)
        {
            const int deadZone = 40;
            if(players[i].sx > deadZone || players[i].sx < -deadZone || players[i].sy > deadZone || players[i].sy < -deadZone)
            {
                players[i].isOk = false;
            }
        }
        else if (target.stick.type == TARGET_HOLD)
        {
            const int requirement = 40;
            if(target.stick.variant == 0 && players[i].sx > -requirement)
            {
                players[i].isOk = false;
            }
            else if(target.stick.variant == 1 && players[i].sy > -requirement)
            {   
                players[i].isOk = false;
            }
            else if(target.stick.variant == 2 && players[i].sx < requirement)
            {
                players[i].isOk = false;
            }
            else if(target.stick.variant == 3 && players[i].sy < requirement)
            {
                players[i].isOk = false;
            }
        }

        checkPlayerTarget(target.a.type, &players[i].a, i);
        checkPlayerTarget(target.b.type, &players[i].b, i);
        checkPlayerTarget(target.l.type, &players[i].l, i);
        checkPlayerTarget(target.z.type, &players[i].z, i);
        checkPlayerTarget(target.r.type, &players[i].r, i);

        checkPlayerTarget(target.cLeft.type, &players[i].cLeft, i);
        checkPlayerTarget(target.cDown.type, &players[i].cDown, i);
        checkPlayerTarget(target.cRight.type, &players[i].cRight, i);
        checkPlayerTarget(target.cUp.type, &players[i].cUp, i);

        // Update player alive status
        if(gameEnded)
        {
            if(players[i].hp > 0.0f)
            {
                rdpq_text_printf(&textparms, FONT_TEXT, offsetX, offsetY + i * plaeryUiGap + textGap, "WINNER!");
            }
            else
            {
                rdpq_text_printf(&textparms, FONT_TEXT, offsetX, offsetY + i * plaeryUiGap + textGap, "OUT!");
            }
            
            if(joypad.btn.start)
            {
                minigame_end();
            }
        }
        else
        {
            if(players[i].hp > 0.0f)
            {
                if(players[i].isCpu)
                {
                    players[i].cpuTimer -= deltatime;
                    
                    if(players[i].cpuTimer < 0.0f)
                    {
                        players[i].isOk = false;
                    }
                    else
                    {
                        players[i].isOk = true;
                    }
                }

                if(players[i].isOk)
                {
                    rdpq_text_printf(&textparms, FONT_TEXT, offsetX, offsetY + i * plaeryUiGap + textGap * 2, "OK");
                }
                else 
                {
                    rdpq_text_printf(&textparms, FONT_TEXT, offsetX, offsetY + i * plaeryUiGap + textGap * 2, "Not OK!");
                }

                if(target.timer > 1.0f)
                {
                    if(!players[i].isOk)
                    {
                        players[i].hp -= deltatime * 6.0f;
                    }
                }

                rdpq_text_printf(&textparms, FONT_TEXT, offsetX, offsetY + i * plaeryUiGap + textGap, "HP %i", (int)players[i].hp);
            }
            else
            {
                rdpq_text_printf(&textparms, FONT_TEXT, offsetX, offsetY + i * plaeryUiGap + textGap, "OUT!");
                playerAlive--;
                if(!players[i].isCpu)
                {
                    nonAiAlive--;
                }
            }
        }
    }

    if(nonAiAlive == 0)
    {
        fastForward = true;
    }

    if(playerAlive > 3)
    {
        newTargetRate = 5.0f;
    }
    else if(playerAlive > 2)
    {
        newTargetRate = 4.0f;
    }
    else
    {
        newTargetRate = 3.0f;
    }
    
    // Check game end
    if(playerAlive <= 1 && !gameEnded)
    {
        gameEnded = true;
        wav64_close(&sfx_music);

        if(playerAlive > 0)
        {
            wav64_play(&sfx_winner, 31);
        }
        else
        {
            wav64_play(&sfx_stop, 31);
        }
        
        for(int i = 0; i < MAXPLAYERS; ++i)
        {
            if(players[i].hp > 0.0f)
            {
                core_set_winner(i);
            }
        }
    }

    // Update target
    if(gameEnded)
    {
        rdpq_text_printf(&textparms, FONT_TEXT, offsetX, 206, "Press START to return");
    }
    else
    {
        rdpq_text_printf(&textparms, FONT_TEXT, offsetX, 206, "Next button in %i!", (int)newTargetRate - (int)target.timer);
        target.timer += deltatime;

        if(target.timer > newTargetRate)
        {
            wav64_play(&sfx_start, 31);
            countDownTimer = 1.0f;
            //printf("Add new target"); 
            target.timer = 0.0f;
            int r = rand() % 13;
            //rdpq_text_printf(&textparms, FONT_TEXT, 200, 20, "%d", r);

            if(r == 0)
            {
                target.a.type = returnTarget(target.a.type);
            }
            else if(r == 1)
            {
                target.b.type = returnTarget(target.b.type);
            }
            else if(r == 2)
            {
                target.l.type = returnTarget(target.l.type);
            }
            else if(r == 3)
            {
                target.z.type = returnTarget(target.z.type);
            }
            else if(r == 4)
            {
                target.r.type = returnTarget(target.r.type);
            }
            else if(r == 5)
            {
                target.cLeft.type = returnTarget(target.cLeft.type);
            }
            else if(r == 6)
            {
                target.cDown.type = returnTarget(target.cDown.type);
            }
            else if(r == 7)
            {
                target.cRight.type = returnTarget(target.cRight.type);
            }
            else if(r == 8)
            {
                target.cUp.type = returnTarget(target.cUp.type);
            }
            else if(r < 11)
            {
                target.dPad.type = TARGET_HOLD;
                target.dPad.variant = rand() % 4;
                holds++;
            }
            else if(r < 13)
            {
                target.stick.type = TARGET_HOLD;
                target.stick.variant = rand() % 4;
                holds++;
            }
        }
        else
        {
            countDownTimer -= deltatime;
            if (countDownTimer < 0.0f)
            {
                wav64_play(&sfx_countdown, 31);
                countDownTimer = 1.0f;
            }
        }
    }

    if(!playMusic)
    {
        playMusic = true;
        wav64_set_loop(&sfx_music, true);
        wav64_play(&sfx_music, 30);
    }
    
    // Debug display target
    /*
    if(target.dPad.type == TARGET_RELEASE)
    {
        rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D   R");
    }
    else if(target.dPad.type == TARGET_HOLD)
    {
        if(target.dPad.variant == 0) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-L  H");
        if(target.dPad.variant == 1) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-U  H");
        if(target.dPad.variant == 2) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-R  H");
        if(target.dPad.variant == 3) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-D  H");
    }
    else 
    {
        if(target.dPad.variant == 0) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-L  S");
        if(target.dPad.variant == 1) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-U  S");
        if(target.dPad.variant == 2) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-R  S");
        if(target.dPad.variant == 3) rdpq_text_printf(&textparms, FONT_TEXT, 100, 200, "D-D  S");
    }

    if(target.stick.type == TARGET_RELEASE)
    {
        rdpq_text_printf(&textparms, FONT_TEXT, 100, 210, "S   R");
    }
    else if(target.stick.type == TARGET_HOLD)
    {
        if(target.stick.variant == 0) rdpq_text_printf(&textparms, FONT_TEXT, 100, 210, "S-L  H");
        if(target.stick.variant == 1) rdpq_text_printf(&textparms, FONT_TEXT, 100, 210, "S-U  H");
        if(target.stick.variant == 2) rdpq_text_printf(&textparms, FONT_TEXT, 100, 210, "S-R  H");
        if(target.stick.variant == 3) rdpq_text_printf(&textparms, FONT_TEXT, 100, 210, "S-D  H");
    }

    displayButtonTarget(&textparms, target.a.type, 160, 170, "A");
    displayButtonTarget(&textparms, target.b.type, 160, 180, "B");
    displayButtonTarget(&textparms, target.l.type, 160, 190, "L");
    displayButtonTarget(&textparms, target.z.type, 160, 200, "Z");
    displayButtonTarget(&textparms, target.r.type, 160, 210, "R");

    displayButtonTarget(&textparms, target.cLeft.type, 220, 180, "C-L");
    displayButtonTarget(&textparms, target.cUp.type, 220, 190, "C-U");
    displayButtonTarget(&textparms, target.cRight.type, 220, 200, "C-R");
    displayButtonTarget(&textparms, target.cUp.type, 220, 210, "C-D");
    */
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

    rspq_block_free(dplController);
    rspq_block_free(dplControllerDPadLeft);
    rspq_block_free(dplControllerDPadDown);
    rspq_block_free(dplControllerDPadRight);
    rspq_block_free(dplControllerDPadUp);
    rspq_block_free(dplControllerStickLeft);
    rspq_block_free(dplControllerStickDown);
    rspq_block_free(dplControllerStickRight);
    rspq_block_free(dplControllerStickUp);
    rspq_block_free(dplControllerPressA);
    rspq_block_free(dplControllerPressB);
    rspq_block_free(dplControllerPressL);
    rspq_block_free(dplControllerPressZ);
    rspq_block_free(dplControllerPressR);
    rspq_block_free(dplControllerPressCLeft);
    rspq_block_free(dplControllerPressCDown);
    rspq_block_free(dplControllerPressCRight);
    rspq_block_free(dplControllerPressCUp);

    t3d_model_free(modelController);
    t3d_model_free(modelPressDPadLeft);
    t3d_model_free(modelPressDPadDown);
    t3d_model_free(modelPressDPadRight);
    t3d_model_free(modelPressDPadUp);
    t3d_model_free(modelPressStickLeft);
    t3d_model_free(modelPressStickDown);
    t3d_model_free(modelPressStickRight);
    t3d_model_free(modelPressStickUp);
    t3d_model_free(modelPressA);
    t3d_model_free(modelPressB);
    t3d_model_free(modelPressL);
    t3d_model_free(modelPressZ);
    t3d_model_free(modelPressR);
    t3d_model_free(modelPressCLeft);
    t3d_model_free(modelPressCDown);
    t3d_model_free(modelPressCRight);
    t3d_model_free(modelPressCUp);

    free_uncached(controllerMatFP);

    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
    rdpq_text_unregister_font(FONT_P1);
    rdpq_font_free(fontP1);
    rdpq_text_unregister_font(FONT_P2);
    rdpq_font_free(fontP2);
    rdpq_text_unregister_font(FONT_P3);
    rdpq_font_free(fontP3);
    rdpq_text_unregister_font(FONT_P4);
    rdpq_font_free(fontP4);

    t3d_destroy();

    display_close();
}