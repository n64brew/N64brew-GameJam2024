#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <libdragon.h>
#include <stdbool.h>
#include "globals.h"
#include "types.h"
#include "functions.h"
#include "levels.h"
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

const MinigameDef minigame_def = {
    .gamename = "Sword Strike",
    .developername = "Super Boognish",
    .description = "Free for all battle, but take one hit and you're out! Last player standing wins.",
    .instructions = "DPAD for movement, A to jump, B to attack, Down + A to drop down, L to slide"
};

// font globals
#define FONT_TEXT      1
#define TEXT_COLOR     0x6CBB3CFF

// mixer channel allocation
#define CHANNEL_SFX    31
#define CHANNEL_MUSIC  30

// game state
// COUNT DOWN = 0, IN GAME = 1, GAME END = 2, PAUSE = 3
int game_state;
float countdown_timer;

// weapons init
struct weapon basicSword;
struct weapon heavySword;

struct weapon weapons[2];

// players init
struct player player1;
struct player player2;
struct player player3;
struct player player4;

struct player* players[4];

// level data init
int numFloors;
struct floorPiece **floors;

wav64_t sfx_start, sfx_countdown, sfx_stop, sfx_winner, sfx_scream;
wav64_t music;

// number of players specified before loading game
uint32_t numPlayers;

// timer to display winner on screen before exiting game
float game_over_counter;

// end game vars / sound triggers
int winnerIndex;
bool playedWinnerSound;
bool playDeathSound;

// keep track of which port paused game + delay for proper response timing
joypad_port_t pausePlayerPort;
float pauseCheckDelay;

// player sprites init
sprite_t *fighter_left_neutral;
sprite_t *fighter_right_neutral;
sprite_t *fighter_left_jump;
sprite_t *fighter_right_jump;
sprite_t *fighter_left_slide;
sprite_t *fighter_right_slide;

// left attack animation sprites
sprite_t *fighter_left_attack_1;
sprite_t *fighter_left_attack_2;
sprite_t *fighter_left_attack_3;
sprite_t *fighter_left_attack_4;
sprite_t *fighter_left_attack_5;
sprite_t *fighter_left_attack_6;
sprite_t *fighter_left_attack_7;
sprite_t *fighter_left_attack_8;
sprite_t *fighter_left_attack_9;
sprite_t *fighter_left_attack_10;

// right attack animation sprites
sprite_t *fighter_right_attack_1;
sprite_t *fighter_right_attack_2;
sprite_t *fighter_right_attack_3;
sprite_t *fighter_right_attack_4;
sprite_t *fighter_right_attack_5;
sprite_t *fighter_right_attack_6;
sprite_t *fighter_right_attack_7;
sprite_t *fighter_right_attack_8;
sprite_t *fighter_right_attack_9;
sprite_t *fighter_right_attack_10;

sprite_t* player_sprites[6];
sprite_t* player_left_attack_anim[10];
sprite_t* player_right_attack_anim[10];
    
// font
rdpq_font_t *font;

// lighting vars
// int rotateLightCounter;
// int lightCoordIndex;
// float lightCoordArrayX[4];
// float lightCoordArrayY[4];
// float lightCoordArrayZ[4];
float lightCoordX;
float lightCoordY;
float lightCoordZ;

// T3D stuff
surface_t *depthBuffer;
rspq_block_t *dplMap;
T3DViewport viewport;
T3DMat4FP* mapMatFP;
T3DModel *modelMap;
T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 lightDirVec;

void minigame_init(){
    // load font for text drawing
    font = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = color_from_packed32(TEXT_COLOR) });

    //player label colors
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    for (size_t i = 0; i < 4; i++)
    {
        rdpq_font_style(font, i+1, &(rdpq_fontstyle_t){ .color = colors[i] });
    }

    // load sprites from rom
    char fn1[64];
    sprintf(fn1, "rom:/swordstrike/fighter_left_neutral.sprite");
    fighter_left_neutral = sprite_load(fn1);

    char fn2[64];
    sprintf(fn2, "rom:/swordstrike/fighter_right_neutral.sprite");
    fighter_right_neutral = sprite_load(fn2);

    char fn3[64];
    sprintf(fn3, "rom:/swordstrike/fighter_jumping_left.sprite");
    fighter_left_jump = sprite_load(fn3);

    char fn4[64];
    sprintf(fn4, "rom:/swordstrike/fighter_left_attack_1.sprite");
    fighter_left_attack_1 = sprite_load(fn4);

    char fn5[64];
    sprintf(fn5, "rom:/swordstrike/fighter_left_attack_2.sprite");
    fighter_left_attack_2 = sprite_load(fn5);

    char fn6[64];
    sprintf(fn6, "rom:/swordstrike/fighter_left_attack_3.sprite");
    fighter_left_attack_3 = sprite_load(fn6);

    char fn7[64];
    sprintf(fn7, "rom:/swordstrike/fighter_left_attack_4.sprite");
    fighter_left_attack_4 = sprite_load(fn7);

    char fn8[64];
    sprintf(fn8, "rom:/swordstrike/fighter_left_attack_5.sprite");
    fighter_left_attack_5 = sprite_load(fn8);

    char fn9[64];
    sprintf(fn9, "rom:/swordstrike/fighter_left_attack_6.sprite");
    fighter_left_attack_6 = sprite_load(fn9);

    char fn10[64];
    sprintf(fn10, "rom:/swordstrike/fighter_left_attack_7.sprite");
    fighter_left_attack_7 = sprite_load(fn10);

    char fn11[64];
    sprintf(fn11, "rom:/swordstrike/fighter_left_attack_8.sprite");
    fighter_left_attack_8 = sprite_load(fn11);

    char fn12[64];
    sprintf(fn12, "rom:/swordstrike/fighter_left_attack_9.sprite");
    fighter_left_attack_9 = sprite_load(fn12);

    char fn13[64];
    sprintf(fn13, "rom:/swordstrike/fighter_left_attack_10.sprite");
    fighter_left_attack_10 = sprite_load(fn13);

    char fn19[64];
    sprintf(fn19, "rom:/swordstrike/fighter_right_attack_1.sprite");
    fighter_right_attack_1 = sprite_load(fn19);

    char fn20[64];
    sprintf(fn20, "rom:/swordstrike/fighter_right_attack_2.sprite");
    fighter_right_attack_2 = sprite_load(fn20);

    char fn21[64];
    sprintf(fn21, "rom:/swordstrike/fighter_right_attack_3.sprite");
    fighter_right_attack_3 = sprite_load(fn21);

    char fn22[64];
    sprintf(fn22, "rom:/swordstrike/fighter_right_attack_4.sprite");
    fighter_right_attack_4 = sprite_load(fn22);

    char fn23[64];
    sprintf(fn23, "rom:/swordstrike/fighter_right_attack_5.sprite");
    fighter_right_attack_5 = sprite_load(fn23);

    char fn24[64];
    sprintf(fn24, "rom:/swordstrike/fighter_right_attack_6.sprite");
    fighter_right_attack_6 = sprite_load(fn24);

    char fn25[64];
    sprintf(fn25, "rom:/swordstrike/fighter_right_attack_7.sprite");
    fighter_right_attack_7 = sprite_load(fn25);

    char fn26[64];
    sprintf(fn26, "rom:/swordstrike/fighter_right_attack_8.sprite");
    fighter_right_attack_8 = sprite_load(fn26);

    char fn27[64];
    sprintf(fn27, "rom:/swordstrike/fighter_right_attack_9.sprite");
    fighter_right_attack_9 = sprite_load(fn27);

    char fn28[64];
    sprintf(fn28, "rom:/swordstrike/fighter_right_attack_10.sprite");
    fighter_right_attack_10 = sprite_load(fn28);

    char fn34[64];
    sprintf(fn34, "rom:/swordstrike/fighter_jumping_right.sprite");
    fighter_right_jump = sprite_load(fn34);

    char fn35[64];
    sprintf(fn35, "rom:/swordstrike/fighter_sliding_left.sprite");
    fighter_left_slide = sprite_load(fn35);

    char fn36[64];
    sprintf(fn36, "rom:/swordstrike/fighter_sliding_right.sprite");
    fighter_right_slide = sprite_load(fn36);

    player_sprites[0] = fighter_left_neutral;
    player_sprites[1] = fighter_right_neutral;
    player_sprites[2] = fighter_left_jump;
    player_sprites[3] = fighter_right_jump;
    player_sprites[4] = fighter_left_slide;
    player_sprites[5] = fighter_right_slide;

    player_left_attack_anim[0] = fighter_left_attack_10;
    player_left_attack_anim[1] = fighter_left_attack_9;
    player_left_attack_anim[2] = fighter_left_attack_8;
    player_left_attack_anim[3] = fighter_left_attack_7;
    player_left_attack_anim[4] = fighter_left_attack_6;
    player_left_attack_anim[5] = fighter_left_attack_5;
    player_left_attack_anim[6] = fighter_left_attack_4;
    player_left_attack_anim[7] = fighter_left_attack_3;
    player_left_attack_anim[8] = fighter_left_attack_2;
    player_left_attack_anim[9] = fighter_left_attack_1;

    player_right_attack_anim[0] = fighter_right_attack_10;
    player_right_attack_anim[1] = fighter_right_attack_9;
    player_right_attack_anim[2] = fighter_right_attack_8;
    player_right_attack_anim[3] = fighter_right_attack_7;
    player_right_attack_anim[4] = fighter_right_attack_6;
    player_right_attack_anim[5] = fighter_right_attack_5;
    player_right_attack_anim[6] = fighter_right_attack_4;
    player_right_attack_anim[7] = fighter_right_attack_3;
    player_right_attack_anim[8] = fighter_right_attack_2;
    player_right_attack_anim[9] = fighter_right_attack_1;

    // default to 0
    pauseCheckDelay = 0.0f;

    //get number of players
    numPlayers = core_get_playercount();

    // default values
    basicSword.id = 0;
    basicSword.xPos = 0;
    basicSword.yPos = 0;
    basicSword.width = 5;
    basicSword.height = 20;
    basicSword.attackTimer = 10;
    basicSword.attackCooldown = 10;

    weapons[0] = basicSword;

    player1.height = 25;
    player1.width = 20;
    player1.xPos = 20;
    player1.yPos = 60;
    player1.color = PLAYERCOLOR_1;
    player1.id = 0;
    initPlayer(&player1, &weapons[0]);
    updatePlayerBoundingBox(&player1);
    updateWeaponHitbox(&player1.weapon);

    player2.height = 25;
    player2.width = 20;
    player2.xPos = 275;
    player2.yPos = 60;
    player2.color = PLAYERCOLOR_2;
    player2.id = 1;
    initPlayer(&player2, &weapons[0]);
    updatePlayerBoundingBox(&player2);
    updateWeaponHitbox(&player2.weapon);

    player3.height = 25;
    player3.width = 20;
    player3.xPos = 20;
    player3.yPos = 140;
    player3.color = PLAYERCOLOR_3;
    player3.id = 2;
    initPlayer(&player3, &weapons[0]);
    updatePlayerBoundingBox(&player3);
    updateWeaponHitbox(&player3.weapon);

    player4.height = 25;
    player4.width = 20;
    player4.xPos = 275;
    player4.yPos = 140;
    player4.color = PLAYERCOLOR_4;
    player4.id = 3;
    initPlayer(&player4, &weapons[0]);
    updatePlayerBoundingBox(&player4);
    updateWeaponHitbox(&player4.weapon);

    players[0] = &player1;
    players[1] = &player2;
    players[2] = &player3;
    players[3] = &player4;

    // load in level data
    floors = loadLevel1(&numFloors);

    // set game state + timer
    game_state = 0;
    countdown_timer = 4.0f;
    game_over_counter = 8.0f;
    
    playedWinnerSound = false;
    playDeathSound = false;

    // load sound files
    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");
    wav64_open(&sfx_scream, "rom:/swordstrike/wilhelm_scream.wav64");
    wav64_open(&music, "rom:/swordstrike/challengers.wav64");
    wav64_set_loop(&music, true);

    mixer_ch_set_vol(CHANNEL_MUSIC, 0.4f, 0.4f);

    // initialize display
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE);

    // lighting animation vars -> can't get working. T3D + blender = hard :(
    // rotateLightCounter = 10;
    // lightCoordIndex = 0;
    // lightCoordArrayX[0] = 5.0f;
    // lightCoordArrayX[1] = 5.0f;
    // lightCoordArrayX[2] = 5.0f;
    // lightCoordArrayX[3] = 5.0f;
    // lightCoordArrayY[0] = 100.0f;
    // lightCoordArrayY[1] = 100.0f;
    // lightCoordArrayY[2] = 100.0f; // setting to 0 changes 2 / 5 face colors? looks bad
    // lightCoordArrayY[3] = 100.0f; // setting to 0 changes 2 / 5 face colors? looks bad
    // lightCoordArrayZ[0] = 5.0f;
    // lightCoordArrayZ[1] = 5.0f;
    // lightCoordArrayZ[2] = 5.0f;
    // lightCoordArrayZ[3] = 5.0f;
    // lightCoordX = lightCoordArrayX[0];
    // lightCoordY = lightCoordArrayY[0];
    // lightCoordZ = lightCoordArrayZ[0];
    lightCoordX = 5.0f;
    lightCoordY = 100.0f;
    lightCoordZ = 0.0f;

    // t3d init
    depthBuffer = display_get_zbuf();
    t3d_init((T3DInitParams){});

    viewport = t3d_viewport_create();
    mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0, 0, 0});
    
    camPos = (T3DVec3){{0, 150.0f, 5.0f}};
    camTarget = (T3DVec3){{0, 0, 0}};

    lightDirVec = (T3DVec3){{lightCoordX, lightCoordY, lightCoordZ}};
    t3d_vec3_norm(&lightDirVec);

    // modelMap = t3d_model_load("rom:/swordstrike/bg_sphere.t3dm");
    modelMap = t3d_model_load("rom:/swordstrike/background_cube_color.t3dm");
    // modelMap = t3d_model_load("rom:/swordstrike/background_cube_sand.t3dm");

    rspq_block_begin();
    t3d_matrix_push(mapMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(modelMap);
    t3d_matrix_pop(1);
    dplMap = rspq_block_end();
}

void minigame_fixedloop(float deltatime){
    if(game_state == 0){
        // COUNTDOWN FROM 3
        if (countdown_timer > 1) {
            float prevtime = countdown_timer;
            countdown_timer -= deltatime;
            if ((int)prevtime != (int)countdown_timer && countdown_timer >= 0) {
                wav64_play(&sfx_countdown, CHANNEL_SFX);
            }
        } else {
            game_state = 1;
            wav64_play(&sfx_start, CHANNEL_SFX);
            wav64_play(&music, CHANNEL_MUSIC);
        }
    }
    
    if(game_state == 1){
        // CHECK IF MORE THAN ONE PLAYER IS ALIVE
        int alive = 0;
        for(int i=0; i < 4; i++){
            if(players[i]->isAlive){
                alive++;
            }
        }
        if(alive == 1){
            for(int i=0; i < 4; i++){
                if(players[i]->isAlive){
                    winnerIndex = i+1;
                }
            }
            mixer_ch_set_vol(CHANNEL_MUSIC, 0, 0);
            wav64_play(&sfx_stop, CHANNEL_SFX);
            game_state = 2;
        }
        // shouldn't be possible but adding this just in case
        if(alive == 0){
            minigame_end();
        }

        // play death sound
        if(alive > 1 && playDeathSound){
            wav64_play(&sfx_scream, CHANNEL_SFX);
            playDeathSound = false;
        }

        // update light animation for bg every 10 ticks
        // if(rotateLightCounter <= 0){
        //     rotateLightCounter = 10;
        //     if(lightCoordIndex > 3){
        //         lightCoordIndex = 0;
        //     }
        //     lightCoordX = lightCoordArrayX[lightCoordIndex];
        //     lightCoordY = lightCoordArrayY[lightCoordIndex];
        //     lightCoordZ = lightCoordArrayZ[lightCoordIndex];
        //     lightCoordIndex++;
        // } else {
        //     rotateLightCounter -= 1;
        // }

        // PHYSICS
        uint32_t playercount = core_get_playercount();
        for (size_t i = 0; i < 4; i++)
        {
            bool isHuman = i < playercount;
            if(players[i]->isAlive){
                if(!isHuman){
                    // Generate AI inputs
                    struct player *target = players[players[i]->ai_target];
                    generateCompInputs(players[i], target, floors, &numFloors);
                }

                // APPLY PHYSICS UPDATES FROM INPUT
                updatePlayerPos(players[i], floors, &numFloors, players);

                // CHECK ATTACK COLLISIONS WITH OTHER PLAYERS IF ATTACKING
                if(players[i]->attackTimer > 0){
                    for(int j=0; j < 4; j++){
                        if(players[i]->id != players[j]->id && players[j]->isAlive){
                            checkPlayerWeaponCollision(players[i], players[j]);
                            if(!players[j]->isAlive && !playDeathSound){
                                playDeathSound = true;
                            }
                        }
                    }
                }
                
                // death by falling off map
                if(players[i]->yPos > 360){
                    players[i]->isAlive = false;
                    players[i]->verticalVelocity = 0.0;
                }
            }
        }

        // DEC PAUSE DELAY BY DT
        if(pauseCheckDelay > 0.0){
            pauseCheckDelay -= deltatime;
        }
    }

    // COUNTDOWN TO END GAME
    if(game_state == 2){
        if(game_over_counter < 5.0 && !playedWinnerSound){
            playedWinnerSound = true;
            wav64_play(&sfx_winner, CHANNEL_SFX);
        }
        game_over_counter -= deltatime;
        if(game_over_counter <= 0){
            core_set_winner(winnerIndex);
            minigame_end();
        }
    }

    // PAUSE - DONT UPDATE PHYSICS
    if(game_state == 3){
        // DEC PAUSE DELAY BY DT
        if(pauseCheckDelay > 0.0){
            pauseCheckDelay -= deltatime;
        }
    }
}

void minigame_loop(float deltatime){
    // floor color
    color_t WHITE = RGBA16(255, 255, 255, 0);

    if(game_state == 1){
        uint32_t playercount = core_get_playercount();
        for (size_t i = 0; i < 4; i++)
        {
            bool isHuman = i < playercount;
            joypad_port_t port = core_get_playercontroller(i);

            joypad_buttons_t joypad_held = joypad_get_buttons_held(port);
            joypad_buttons_t joypad_pressed = joypad_get_buttons_pressed(port);

            if(players[i]->isAlive){
                if(isHuman){
                    // POLL MOVEMENT INPUT
                    pollPlayerInput(players[i], &joypad_held);
                
                    // POLL ATTACK INPUT
                    pollAttackInput(players[i], &joypad_pressed);
                }
            }

            // PAUSE GAME
            if(joypad_pressed.start){
                if(pauseCheckDelay <= 0.0){
                    pausePlayerPort = port;
                    pauseCheckDelay = 0.5f;
                    game_state = 3;
                    mixer_ch_set_vol(CHANNEL_SFX, 0, 0);
                    mixer_ch_set_vol(CHANNEL_MUSIC, 0, 0);
                }
            }
        }
    }

    if(game_state == 3){
        joypad_buttons_t joypad_pressed = joypad_get_buttons_pressed(pausePlayerPort);
        joypad_buttons_t joypad_held = joypad_get_buttons_held(pausePlayerPort);

        // UNPAUSE GAME
        if(joypad_pressed.start){
            if(pauseCheckDelay <= 0.0){
                game_state = 1;
                pauseCheckDelay = 0.5f;
                mixer_ch_set_vol(CHANNEL_SFX, 1, 1);
                mixer_ch_set_vol(CHANNEL_MUSIC, 0.4f, 0.4f);
            }
        }
        
        // QUIT GAME
        if(joypad_held.z && joypad_held.d_up){
            minigame_end();
        }
    }

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 1.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // get display
    surface_t *disp = display_get();

    // draw background
    rdpq_attach(disp, depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    // get lighting animation for frame
    // lightDirVec = (T3DVec3){{lightCoordX, lightCoordY, lightCoordZ}};
    // t3d_vec3_norm(&lightDirVec);

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    rspq_block_run(dplMap);

    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise

    // draw player sprites and floors
    draw_players_and_level(players, player_sprites, player_left_attack_anim, player_right_attack_anim, floors, &numFloors, WHITE);

    // set rdpq for drawing text
    rdpq_set_mode_standard();

    // DRAW PLAYER LABELS -> this crashes on console??? cannot draw player 1 label either
    // for (size_t i = 0; i < 4; i++) {
    //     if(players[i]->isAlive){
    //         rdpq_sync_pipe(); // Hardware crashes otherwise
    //         rdpq_sync_tile(); // Hardware crashes otherwise
    //         rdpq_text_printf(&(rdpq_textparms_t){ .style_id = (i+1)}, FONT_TEXT, 
    //                         players[i]->direction == 0 ? players[i]->xPos+8
    //                                                    : players[i]->xPos+4, 
    //                         players[i]->yPos-5, "P%d", i+1);
    //     }
    // }

    if(game_state == 0){
        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_print(&(rdpq_textparms_t){ .style_id = 1}, FONT_TEXT, 
                        players[0]->direction == 0 ? players[0]->xPos+8
                                                    : players[0]->xPos+4, 
                        players[0]->yPos-5, "P1");

        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_print(&(rdpq_textparms_t){ .style_id = 2}, FONT_TEXT, 
                        players[1]->direction == 0 ? players[1]->xPos+8
                                                    : players[1]->xPos+4, 
                        players[1]->yPos-5, "P2");

        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_print(&(rdpq_textparms_t){ .style_id = 3}, FONT_TEXT, 
                        players[2]->direction == 0 ? players[2]->xPos+8
                                                    : players[2]->xPos+4, 
                        players[2]->yPos-5, "P3");

        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4}, FONT_TEXT, 
                        players[3]->direction == 0 ? players[3]->xPos+8
                                                    : players[3]->xPos+4, 
                        players[3]->yPos-5, "P4");
    }

    // COUNT DOWN
    if(game_state == 0){
        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_printf(NULL, FONT_TEXT, 155, 140, "%i", (int)countdown_timer);
    }

    // DISPLAY WINNER NAME 
    if(game_state == 2 && playedWinnerSound){
        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = (winnerIndex) }, FONT_TEXT, 125, 140, "%s %i %s", "PLAYER", winnerIndex, "WINS");
    }

    // PAUSE
    if(game_state == 3){
        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_printf(NULL, FONT_TEXT, 145, 140,  "%s", "PAUSE");
        rdpq_text_printf(NULL, FONT_TEXT, 110, 230, "%s", "HOLD Z + UP TO QUIT");
    }

    // detach rdp before updating display
    rdpq_detach_show();
}

void free_level_data(struct floorPiece **floors){
    for(int i = 0; i < numFloors; i++){
        free(floors[i]);
    }
    free(floors);
}

void minigame_cleanup(){
    // close audio file streams
    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);
    wav64_close(&sfx_scream);
    wav64_close(&music);

    // free level data
    free_level_data(floors);

    // free sprites
    sprite_free(fighter_left_neutral);
    sprite_free(fighter_right_neutral);
    sprite_free(fighter_left_jump);
    sprite_free(fighter_right_jump);
    sprite_free(fighter_left_slide);
    sprite_free(fighter_right_slide);
    sprite_free(fighter_left_attack_1);
    sprite_free(fighter_left_attack_2);
    sprite_free(fighter_left_attack_3);
    sprite_free(fighter_left_attack_4);
    sprite_free(fighter_left_attack_5);
    sprite_free(fighter_left_attack_6);
    sprite_free(fighter_left_attack_7);
    sprite_free(fighter_left_attack_8);
    sprite_free(fighter_left_attack_9);
    sprite_free(fighter_left_attack_10);
    sprite_free(fighter_right_attack_1);
    sprite_free(fighter_right_attack_2);
    sprite_free(fighter_right_attack_3);
    sprite_free(fighter_right_attack_4);
    sprite_free(fighter_right_attack_5);
    sprite_free(fighter_right_attack_6);
    sprite_free(fighter_right_attack_7);
    sprite_free(fighter_right_attack_8);
    sprite_free(fighter_right_attack_9);
    sprite_free(fighter_right_attack_10);

    // free fonts
    rdpq_font_free(font);
    rdpq_text_unregister_font(FONT_TEXT);

    // t3d cleanup
    rspq_block_free(dplMap);
    t3d_model_free(modelMap);
    free_uncached(mapMatFP);
    t3d_destroy();

    // reset mixer channels to default volume
    mixer_ch_set_vol(CHANNEL_SFX, 1, 1);
    mixer_ch_set_vol(CHANNEL_MUSIC, 1, 1);

    display_close();
}
