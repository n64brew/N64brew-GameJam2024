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

const MinigameDef minigame_def = {
    .gamename = "Sword Strike",
    .developername = "Super Boognish",
    .description = "Free for all battle, but take one hit and you're out! Last player standing wins.",
    .instructions = "DPAD for movement, A to jump, B to attack, Down + A to drop down, L to slide"
};

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

//player 1 test sprite
// sprite_t *player_sprites[4];
sprite_t *player1_sprite;

void minigame_init(){
    // player 1 sprite test
    // char fn[64];
    // sprintf(fn, "rom:/swordstrike/test-sprite.sprite");
    // player_sprites[0] = sprite_load(fn);

    //ness sprite
    // int fp = dfs_open("/earthbound.sprite");
    // sprite_t *ness = malloc( dfs_size( fp ) );
    // dfs_read( ness, 1, dfs_size( fp ), fp );
    // dfs_close( fp );

    // player 1 sprite test
    char fn[64];
    sprintf(fn, "rom:/swordstrike/test-sprite.sprite");
    player1_sprite = sprite_load(fn);


    // default to 0
    pauseCheckDelay = 0.0f;

    //get number of players
    numPlayers = core_get_playercount();

    // default values
    basicSword.id = 0;
    basicSword.xPos = 0;
    basicSword.yPos = 0;
    basicSword.width = 15;
    basicSword.height = 15;
    basicSword.attackTimer = 15;
    basicSword.attackCooldown = 15;

    heavySword.id = 1;
    heavySword.xPos = 0;
    heavySword.yPos = 0;
    heavySword.width = 25;
    heavySword.height = 30;
    heavySword.attackTimer = 20;
    heavySword.attackCooldown = 20;

    weapons[0] = basicSword;
    weapons[1] = heavySword;

    player1.height = 20;
    player1.width = 20;
    player1.xPos = 20;
    player1.yPos = 60;
    player1.color = PLAYERCOLOR_1;
    player1.id = 0;
    initPlayer(&player1, &weapons[0]);
    updatePlayerBoundingBox(&player1);
    updateWeaponHitbox(&player1.weapon);

    player2.height = 20;
    player2.width = 20;
    player2.xPos = 275;
    player2.yPos = 60;
    player2.color = PLAYERCOLOR_2;
    player2.id = 1;
    initPlayer(&player2, &weapons[0]);
    updatePlayerBoundingBox(&player2);
    updateWeaponHitbox(&player2.weapon);

    player3.height = 20;
    player3.width = 20;
    player3.xPos = 20;
    player3.yPos = 140;
    player3.color = PLAYERCOLOR_3;
    player3.id = 2;
    initPlayer(&player3, &weapons[0]);
    updatePlayerBoundingBox(&player3);
    updateWeaponHitbox(&player3.weapon);

    player4.height = 20;
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
                    struct player *target = players[players[i]->ai_target];
                    generateCompInputs(players[i], target, floors, &numFloors);
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
        // UNPAUSE GAME
        joypad_buttons_t joypad_pressed = joypad_get_buttons_pressed(pausePlayerPort);
        if(joypad_pressed.start){
            if(pauseCheckDelay <= 0.0){
                game_state = 1;
                pauseCheckDelay = 0.5f;
            }
        }
    }

    // get display
    surface_t *disp = display_get();
    
    // attach rdp so we can use rdp functions
    rdpq_attach(disp, NULL);

    // draw background
    graphics_fill_screen(disp, graphics_convert_color(DARK_GREY));

    // draw players and floors
    draw_players_and_level(players, floors, &numFloors, WHITE, player1_sprite);

    // draw hitboxs => REMOVE LATER
    if(game_state == 1){
        for(int i = 0; i < 4; i++){
            if(players[i]->isAlive && players[i]->attackTimer > 0){
                rdpq_draw_one_rectangle(&players[i]->weapon.xPos, &players[i]->weapon.yPos, &players[i]->weapon.width, &players[i]->weapon.height, DARK_GREEN);
            }
        }
    }

    // detach rdp before updating display
    rdpq_detach();

    // DRAW DEBUG VALUES -- remove later
    void draw_debug_values(){

        // PLAYER 1 VARS
        char msg1[100];
        graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        snprintf(msg1, sizeof(msg1), "%s: %i, %i", "POS", player1.xPos, player1.yPos);
        graphics_draw_text(disp, 10, 10, msg1);

        char msg2[100];
        graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        snprintf(msg2, sizeof(msg2), "%s: %f, %f", "VEL", player1.horizontalVelocity, player1.verticalVelocity);
        graphics_draw_text(disp, 10, 20, msg2);

        // char msg3[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg3, sizeof(msg3), "%s: %i", "GAME_STATE", game_state);
        // graphics_draw_text(disp, 10, 30, msg3);

        // PLAYER 2 VARS
        // char msg3[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg3, sizeof(msg3), "%s: %i, %i", "POS", player2.xPos, player2.yPos);
        // graphics_draw_text(disp, 10, 30, msg3);

        // char msg4[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg4, sizeof(msg4), "%s: %f, %f", "VEL", player2.horizontalVelocity, player2.verticalVelocity);
        // graphics_draw_text(disp, 10, 40, msg4);

        // PLAYERS ALIVE STATUS
        // char msg3[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg3, sizeof(msg3), "%s: %i", "P1 ALIVE", player1.isAlive);
        // graphics_draw_text(disp, 10, 30, msg3);

        // char msg4[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg4, sizeof(msg4), "%s: %i", "P2 ALIVE", player2.isAlive);
        // graphics_draw_text(disp, 10, 40, msg4);

        // char msg5[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg5, sizeof(msg5), "%s: %i", "P3 ALIVE", player3.isAlive);
        // graphics_draw_text(disp, 10, 50, msg5);

        // char msg6[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg6, sizeof(msg6), "%s: %i", "P4 ALIVE", player4.isAlive);
        // graphics_draw_text(disp, 10, 60, msg6);

        // PLAYER 1 WEAPON
        // char msg5[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg5, sizeof(msg5), "%s: %i, %i", "WEAP POS", player1.weapon.xPos, player1.weapon.yPos);
        // graphics_draw_text(disp, 10, 40, msg5);

        // char msg6[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg6, sizeof(msg6), "%s: %i", "ATT TIMER", player1.attackTimer);
        // graphics_draw_text(disp, 10, 30, msg6);

        // char msg4[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg4, sizeof(msg4), "%s: %i", "PAUSE PRESSED", pausePressed);
        // graphics_draw_text(disp, 10, 40, msg4);
        
        char msg4[100];
        graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        snprintf(msg4, sizeof(msg4), "%s: %i", "P1 onEnemy", player1.onTopOfEnemy);
        graphics_draw_text(disp, 10, 30, msg4);

        // char msg5[100];
        // graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        // snprintf(msg5, sizeof(msg5), "%s: %i", "P1 yPos + 10", (player1.yPos + 10));
        // graphics_draw_text(disp, 10, 50, msg5);

    }
    draw_debug_values();

    // DRAW PLAYER SPRITES
    graphics_draw_sprite(disp, player1->xPos, (player1->yPos + player1->height), player1_sprite);

    // COUNT DOWN
    if(game_state == 0){
        char msg1[100];
        graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        snprintf(msg1, sizeof(msg1), "%i", (int)countdown_timer);
        graphics_draw_text(disp, 155, 130, msg1);
    }

    // DISPLAY WINNER NAME 
    if(game_state == 2 && playedWinnerSound){
        char msg1[100];
        graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        snprintf(msg1, sizeof(msg1), "%s %i %s", "PLAYER", winnerIndex, "WINS");
        graphics_draw_text(disp, 110, 130, msg1);
    }

    // PAUSE
    if(game_state == 3){
        char msg1[100];
        graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
        snprintf(msg1, sizeof(msg1), "%s", "PAUSE");
        graphics_draw_text(disp, 140, 130, msg1);
    }

    // UPDATE DISPLAY WITH CURRENT VIDEO DATA
    display_show(disp);
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
    sprite_free(player_sprites[0]);

    display_close();
}
