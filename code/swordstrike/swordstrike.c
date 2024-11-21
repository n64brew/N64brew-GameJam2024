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

#define FONT_TEXT           1
#define TEXT_COLOR          0x6CBB3CFF

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

// init audio files
wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

// number of players specified before loading game
uint32_t numPlayers;

// timer to display winner on screen before exiting game
float game_over_counter;

int winnerIndex;
bool playedWinnerSound;

// keep track of which port paused game + delay for proper response timing
joypad_port_t pausePlayerPort;
float pauseCheckDelay;

// player sprites init
sprite_t *fighter_left_neutral;
sprite_t *fighter_right_neutral;
sprite_t *fighter_left_jump;
sprite_t *fighter_right_jump;

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
sprite_t *fighter_left_attack_11;
sprite_t *fighter_left_attack_12;
sprite_t *fighter_left_attack_13;
sprite_t *fighter_left_attack_14;
sprite_t *fighter_left_attack_15;

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
sprite_t *fighter_right_attack_11;
sprite_t *fighter_right_attack_12;
sprite_t *fighter_right_attack_13;
sprite_t *fighter_right_attack_14;
sprite_t *fighter_right_attack_15;

sprite_t* player_sprites[4];
sprite_t* player_left_attack_anim[20];
sprite_t* player_right_attack_anim[20];
    
// font
rdpq_font_t *font;

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
    // load font
    font = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = color_from_packed32(TEXT_COLOR) });

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

    char fn14[64];
    sprintf(fn14, "rom:/swordstrike/fighter_left_attack_11.sprite");
    fighter_left_attack_11 = sprite_load(fn14);

    char fn15[64];
    sprintf(fn15, "rom:/swordstrike/fighter_left_attack_12.sprite");
    fighter_left_attack_12 = sprite_load(fn15);

    char fn16[64];
    sprintf(fn16, "rom:/swordstrike/fighter_left_attack_13.sprite");
    fighter_left_attack_13 = sprite_load(fn16);

    char fn17[64];
    sprintf(fn17, "rom:/swordstrike/fighter_left_attack_14.sprite");
    fighter_left_attack_14 = sprite_load(fn17);

    char fn18[64];
    sprintf(fn18, "rom:/swordstrike/fighter_left_attack_15.sprite");
    fighter_left_attack_15 = sprite_load(fn18);

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

    char fn29[64];
    sprintf(fn29, "rom:/swordstrike/fighter_right_attack_11.sprite");
    fighter_right_attack_11 = sprite_load(fn29);

    char fn30[64];
    sprintf(fn30, "rom:/swordstrike/fighter_right_attack_12.sprite");
    fighter_right_attack_12 = sprite_load(fn30);

    char fn31[64];
    sprintf(fn31, "rom:/swordstrike/fighter_right_attack_13.sprite");
    fighter_right_attack_13 = sprite_load(fn31);

    char fn32[64];
    sprintf(fn32, "rom:/swordstrike/fighter_right_attack_14.sprite");
    fighter_right_attack_14 = sprite_load(fn32);

    char fn33[64];
    sprintf(fn33, "rom:/swordstrike/fighter_right_attack_15.sprite");
    fighter_right_attack_15 = sprite_load(fn33);

    char fn34[64];
    sprintf(fn34, "rom:/swordstrike/fighter_jumping_right.sprite");
    fighter_right_jump = sprite_load(fn34);

    player_sprites[0] = fighter_left_neutral;
    player_sprites[1] = fighter_right_neutral;
    player_sprites[2] = fighter_left_jump;
    player_sprites[3] = fighter_right_jump;

    player_left_attack_anim[0] = fighter_left_attack_3;
    player_left_attack_anim[1] = fighter_left_attack_1;
    player_left_attack_anim[2] = fighter_left_attack_2;
    player_left_attack_anim[3] = fighter_left_attack_4;
    player_left_attack_anim[4] = fighter_left_attack_6;
    player_left_attack_anim[5] = fighter_left_attack_8;
    player_left_attack_anim[6] = fighter_left_attack_9;
    player_left_attack_anim[7] = fighter_left_attack_10;
    player_left_attack_anim[8] = fighter_left_attack_11;
    player_left_attack_anim[9] = fighter_left_attack_12;
    player_left_attack_anim[10] = fighter_left_attack_14;
    player_left_attack_anim[11] = fighter_left_attack_15;
    player_left_attack_anim[12] = fighter_left_attack_13;
    player_left_attack_anim[13] = fighter_left_attack_11;
    player_left_attack_anim[14] = fighter_left_attack_10;
    player_left_attack_anim[15] = fighter_left_attack_9;
    player_left_attack_anim[16] = fighter_left_attack_7;
    player_left_attack_anim[17] = fighter_left_attack_5;
    player_left_attack_anim[18] = fighter_left_attack_3;
    player_left_attack_anim[19] = fighter_left_attack_1;

    player_right_attack_anim[0] = fighter_right_attack_3;
    player_right_attack_anim[1] = fighter_right_attack_1;
    player_right_attack_anim[2] = fighter_right_attack_2;
    player_right_attack_anim[3] = fighter_right_attack_4;
    player_right_attack_anim[4] = fighter_right_attack_6;
    player_right_attack_anim[5] = fighter_right_attack_8;
    player_right_attack_anim[6] = fighter_right_attack_9;
    player_right_attack_anim[7] = fighter_right_attack_10;
    player_right_attack_anim[8] = fighter_right_attack_11;
    player_right_attack_anim[9] = fighter_right_attack_12;
    player_right_attack_anim[10] = fighter_right_attack_14;
    player_right_attack_anim[11] = fighter_right_attack_15;
    player_right_attack_anim[12] = fighter_right_attack_13;
    player_right_attack_anim[13] = fighter_right_attack_11;
    player_right_attack_anim[14] = fighter_right_attack_10;
    player_right_attack_anim[15] = fighter_right_attack_9;
    player_right_attack_anim[16] = fighter_right_attack_7;
    player_right_attack_anim[17] = fighter_right_attack_5;
    player_right_attack_anim[18] = fighter_right_attack_3;
    player_right_attack_anim[19] = fighter_right_attack_1;

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
    basicSword.attackTimer = 20;
    basicSword.attackCooldown = 15;

    // heavySword.id = 1;
    // heavySword.xPos = 0;
    // heavySword.yPos = 0;
    // heavySword.width = 25;
    // heavySword.height = 30;
    // heavySword.attackTimer = 25;
    // heavySword.attackCooldown = 20;

    weapons[0] = basicSword;
    // weapons[1] = heavySword;

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

    // floors = loadLevel1(&numFloors);  // load in level 2 data
    floors = loadLevel2(&numFloors); // load in level 2 data

    // set game state + timer
    game_state = 0;
    countdown_timer = 4.0f;
    game_over_counter = 8.0f;
    
    playedWinnerSound = false;

    // load sound files
    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");

    // initialize display
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE);

    // t3d stuff
    depthBuffer = display_get_zbuf();
    t3d_init((T3DInitParams){});

    viewport = t3d_viewport_create();
    mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
    
    camPos = (T3DVec3){{0, 125.0f, 100.0f}};
    camTarget = (T3DVec3){{0, 0, 40}};

    lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    modelMap = t3d_model_load("rom:/snake3d/map.t3dm");

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
            if ((int)prevtime != (int)countdown_timer && countdown_timer >= 0)
                wav64_play(&sfx_countdown, 31);
        } else {
            game_state = 1;
            wav64_play(&sfx_start, 31);
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
            wav64_play(&sfx_stop, 31);
            game_state = 2;
        }
        // shouldn't be possible but adding this just in case
        if(alive == 0){
            minigame_end();
        }

        // PHYSICS
        uint32_t playercount = core_get_playercount();
        for (size_t i = 0; i < MAXPLAYERS; i++)
        {
            bool isHuman = i < playercount;
            if(players[i]->isAlive){
                if(!isHuman){
                    // Generate AI inputs
                    // struct player *target = players[players[i]->ai_target];
                    // generateCompInputs(players[i], target, floors, &numFloors);
                }

                // APPLY PHYSICS UPDATES FROM INPUT
                updatePlayerPos(players[i], floors, &numFloors, players);

                // CHECK ATTACK COLLISIONS WITH OTHER PLAYERS IF ATTACKING
                if(players[i]->attackTimer > 0){
                    for(int j=0; j < MAXPLAYERS; j++){
                        if(players[i]->id != players[j]->id && players[j]->isAlive){
                            checkPlayerWeaponCollision(players[i], players[j]);
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
            wav64_play(&sfx_winner, 31);
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
    // BASIC COLORS
    color_t RED = RGBA16(255, 0, 0, 0);
    color_t WHITE = RGBA16(255, 255, 255, 0);
    color_t BLACK = RGBA16(0, 0, 0, 0);

    color_t GREEN = RGBA16(0, 255, 0, 0);
    color_t BLUE = RGBA16(0, 0, 255, 0);
    color_t PURPLE = RGBA16(128, 0, 128, 0);
    color_t DARK_GREEN = RGBA16(256, 165, 0, 0);
    color_t DARK_GREY = RGBA16(128, 128, 128, 0);

    if(game_state == 1){
        uint32_t playercount = core_get_playercount();
        for (size_t i = 0; i < MAXPLAYERS; i++)
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

             // PLAYER 2 IS DEAD; RESET THEM ============> REMOVE LATER
            if(joypad_pressed.z){
                player2.xPos = 160;
                player2.yPos = 120;
                player2.isAlive = true;
                player2.verticalVelocity = 0.0;
                updatePlayerBoundingBox(&player2);
            }

            // PAUSE GAME
            if(joypad_pressed.start){
                if(pauseCheckDelay <= 0.0){
                    pausePlayerPort = port;
                    pauseCheckDelay = 0.5f;
                    game_state = 3;
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
            }
        }
        
        // QUIT GAME
        if(joypad_held.z && joypad_held.d_up){
            minigame_end();
        }
    }

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 160.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // get display
    surface_t *disp = display_get();
    
    // attach rdp so we can use rdp functions
    // rdpq_attach(disp, NULL);

    // draw background
    // rdpq_clear(DARK_GREY);

    // ======== Draw (3D) ======== //
    rdpq_attach(disp, depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    rspq_block_run(dplMap);

    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise

    // draw player sprites and floors
    draw_players_and_level(players, player_sprites, player_left_attack_anim, player_right_attack_anim, floors, &numFloors, WHITE);

    // draw hitboxs => REMOVE LATER
    // if(game_state == 1){
    //     for(int i = 0; i < 4; i++){
    //         if(players[i]->isAlive && players[i]->attackTimer > 0){
    //             rdpq_draw_one_rectangle(&players[i]->weapon.xPos, &players[i]->weapon.yPos, &players[i]->weapon.width, &players[i]->weapon.height, DARK_GREEN);
    //         }
    //     }
    // }

    // set rdpq for drawing text
    rdpq_set_mode_standard();

    // COUNT DOWN
    if(game_state == 0){
        rdpq_text_printf(NULL, FONT_TEXT, 155, 140, "%i", (int)countdown_timer);
    }

    // DISPLAY WINNER NAME 
    if(game_state == 2 && playedWinnerSound){
        rdpq_text_printf(NULL, FONT_TEXT, 125, 140, "%s %i %s", "PLAYER", winnerIndex, "WINS");
    }

    // PAUSE
    if(game_state == 3){
        rdpq_text_printf(NULL, FONT_TEXT, 145, 140,  "%s", "PAUSE");
        rdpq_text_printf(NULL, FONT_TEXT, 110, 230, "%s", "HOLD Z + UP TO QUIT");
    }

    // DEBUG DATA ---- REMOVE LATER
    // rdpq_text_printf(NULL, FONT_TEXT, 10, 10, "%s, %i", "P1 ATT TIMER: ", players[0]->attackTimer);

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

    // free level data
    free_level_data(floors);

    // free sprites
    sprite_free(fighter_left_neutral);
    sprite_free(fighter_right_neutral);
    sprite_free(fighter_left_jump);
    sprite_free(fighter_right_jump);
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
    sprite_free(fighter_left_attack_11);
    sprite_free(fighter_left_attack_12);
    sprite_free(fighter_left_attack_13);
    sprite_free(fighter_left_attack_14);
    sprite_free(fighter_left_attack_15);
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
    sprite_free(fighter_right_attack_11);
    sprite_free(fighter_right_attack_12);
    sprite_free(fighter_right_attack_13);
    sprite_free(fighter_right_attack_14);
    sprite_free(fighter_right_attack_15);

    // free fonts
    rdpq_font_free(font);
    rdpq_text_unregister_font(FONT_TEXT);

    // t3d cleanup
    rspq_block_free(dplMap);
    t3d_model_free(modelMap);
    free_uncached(mapMatFP);
    t3d_destroy();

    display_close();
}
