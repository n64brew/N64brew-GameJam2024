//#include <D:/projects/libdragon/include/libdragon.h>
#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "lucker.h"
#include "battle.h"
#include "abilities.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#define GAME_BACKGROUND     0x000000FF

#define SLOT_ROTATION_SPEED 500
#define SLOT_TIMER 3
#define SLOT_START_ROT_DEGREES 36
//!!!! change this back to an appropriate number like 5
#define GAME_START_TIMER .25f

//3
#define COMBAT_COUNTDOWN_DELAY .25f

#define GAME_OVER_DELAY 3

#define FONT_TEXT           1
#define FONT_BILLBOARD      2
#define TEXT_COLOR          0x6CBB3CFF
#define TEXT_OUTLINE        0x30521AFF
#define BILLBOARD_YOFFSET 20

const MinigameDef minigame_def = {
    .gamename = "Lucker's Arena",
    .developername = "Tanner P (Ritter7124)",
    .description = "This game is a homage to a mini game of a custom game in Warcraft3 Frozen Throne",
    .instructions = "Spin to win!"
};

surface_t* depthBuffer;
T3DViewport viewport;
rdpq_font_t *font;
rdpq_font_t *fontBillboard;
sprite_t *AButton;
sprite_t *LButton;
sprite_t *RButton;
T3DModel* wheelModel;
T3DModel* fighterModel;
T3DMat4* scaleMat;
T3DMat4* xRMat;
T3DMat4* yRMat;
T3DMat4* zRMat;
//T3DMat4FP* modelMatFP;
T3DVec3 lightDirVec;

T3DVec3 camPos = { {0, 125.0f, 200.0f} };
T3DVec3 camTarget = { {0, 0, 0} };
float gameTimer;
float combatTimer;

rspq_syncpoint_t syncPoint;

player players[4];
PlyNum winner;

wav64_t sfx_slot_start;
wav64_t sfx_slot_run;
wav64_t sfx_slot_win;


const int battleSequence[6][2] = {
    {0,1},
    {2,3},
    {0,2},
    {1,3},
    {0,3},
    {1,2}
};
int tieFighterindex[3];
uint8_t sequenceIndex;
int sequenceLimit;
bool gameOver;

void slot_init (player *player, color_t color, T3DVec3 pos[], T3DVec3 rot[]) {
    player->sl.firstMatFP = malloc_uncached(sizeof(T3DMat4FP));
    player->sl.secondMatFP = malloc_uncached(sizeof(T3DMat4FP));
    player->sl.thirdMatFP = malloc_uncached(sizeof(T3DMat4FP));
    scaleMat = malloc_uncached(sizeof(T3DMat4));
    xRMat = malloc_uncached(sizeof(T3DMat4));
    yRMat = malloc_uncached(sizeof(T3DMat4));
    zRMat = malloc_uncached(sizeof(T3DMat4));

    for (int i = 0; i < 3; i++) {
        player->sl.pos[i] = pos[player->playerNumber * 3 + i];
        player->sl.rot[i] = rot[player->playerNumber * 3 + i];
    }

    player->sl.isSpinning = false;
    player->sl.slotTimer = 0;
    
    player->fighter.lastDamageCrit = 0;

    rspq_block_begin();

    t3d_matrix_push(player->sl.firstMatFP);
    rdpq_set_prim_color(color);
    t3d_model_draw(wheelModel);
    t3d_matrix_pop(1);

    t3d_matrix_push(player->sl.secondMatFP);
    rdpq_set_prim_color(color);
    t3d_model_draw(wheelModel);
    t3d_matrix_pop(1);

    t3d_matrix_push(player->sl.thirdMatFP);
    rdpq_set_prim_color(color);
    t3d_model_draw(wheelModel);
    t3d_matrix_pop(1);

    player->sl.dplWheel = rspq_block_end();

}

void player_init(player *player, bool human) 
{
    player->isHuman = human;
    player->wins = 0;
}


void minigame_init()
{
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };
    T3DVec3 start_positions[] = {
        (T3DVec3){{-98,30,159}}, //first
        (T3DVec3){{-84,30,157}},
        (T3DVec3){{-71,30,154}},
        (T3DVec3){{-38,31,152}},   //second
        (T3DVec3){{-26,31,151}},
        (T3DVec3){{-14,31,150}},
        (T3DVec3){{18,31,150}},  //third
        (T3DVec3){{29,31,151}},
        (T3DVec3){{41,31,152}},
        (T3DVec3){{73,30,153}}, //last
        (T3DVec3){{85,30,156}},
        (T3DVec3){{98,30,158}},
    };
    float x = T3D_DEG_TO_RAD(SLOT_START_ROT_DEGREES);
    T3DVec3 wheel_rot_sets[] = {
        (T3DVec3){{x, -.56f, .4f}},
        (T3DVec3){{x, -.555f, .4f}},
        (T3DVec3){{x, -.55f, .4f}},
        (T3DVec3){{x,-.2f,.2f}},
        (T3DVec3){{x,-.2f,.2f}},
        (T3DVec3){{x,-.2f,.2f}},
        (T3DVec3){{x,.2f,-.2f}},
        (T3DVec3){{x,.2f,-.2f}},
        (T3DVec3){{x,.2f,-.175f}},
        (T3DVec3){{x, .5f, -.4f}},
        (T3DVec3){{x, .5f, -.4f}},
        (T3DVec3){{x, .5f, -.4f}},
    };
    
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    t3d_init((T3DInitParams) {});
    
    //rdpq_debug_start();

    //replace font stuff if I had more time
    font = rdpq_font_load("rom:/lucker/m6x11plus.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = color_from_packed32(TEXT_COLOR) });

    fontBillboard = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_BILLBOARD, fontBillboard);
    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        rdpq_font_style(fontBillboard, i, &(rdpq_fontstyle_t){ .color = colors[i] });
    }

    lightDirVec = (T3DVec3){ {1.0f, 1.0f, 1.0f} };
    t3d_vec3_norm(&lightDirVec);

    depthBuffer = display_get_zbuf();
    viewport = t3d_viewport_create();
    wheelModel = t3d_model_load("rom:/lucker/wheel.t3dm");
    fighterModel = t3d_model_load("rom:/lucker/snake.t3dm");
    AButton = sprite_load("rom:/lucker/A_Button.sprite");
    LButton = sprite_load("rom:/lucker/L_Button.sprite");
    RButton = sprite_load("rom:/lucker/R_Button.sprite");


    sequenceIndex = 0;
    sequenceLimit = 6;
    
    winner = -1;
    gameTimer = 0;
    combatTimer = 0;
    gameOver = false;

    uint32_t playercount = core_get_playercount();
    for (int i = 0; i < 4; i++) 
    {
        player_init(&players[i], i < playercount);
        players[i].playerNumber = i;
        slot_init(&players[i], colors[i], start_positions, wheel_rot_sets);
        battle_player_init(&players[i], colors[i], fighterModel);
    }
    
    battle_init();

    syncPoint = 0;
    wav64_open(&sfx_slot_start, "rom:/lucker/Slot_Start.wav64");
    wav64_open(&sfx_slot_run, "rom:/lucker/Slot_Run.wav64");
    wav64_open(&sfx_slot_win, "rom:/lucker/Slot_Win.wav64");
    
    mixer_ch_set_vol(31, 0.5f, 0.5f); //slot start
    mixer_ch_set_vol(30, 0.15f, 0.15f); //slot run
    mixer_ch_set_vol(29, 0.3f, 0.3f); //slot win
    mixer_ch_set_vol(28, 0.3f, 0.3f); //bites
    mixer_ch_set_vol(27, 0.3f, 0.3f); //crits and cheers
    mixer_ch_set_vol(26, 0.3f, 0.3f);
}
void wheel_matrix(player *plyr, T3DVec3 pos, T3DVec3 rot, T3DMat4FP *out) 
{
    float rads0 = rot.v[0];
    float rads1 = rot.v[1];
    float rads2 = rot.v[2];
    float s = (plyr->playerNumber == 1 || plyr->playerNumber == 2) ? .175f : .25f;
    *scaleMat = (T3DMat4){{
        {.175f, 0, 0, 0},
        {0, s, 0, 0},
        {0, 0, s, 0},
        {0, 0, 0, 1}
    }};
    *xRMat = (T3DMat4){{
        {1, 0, 0, 0},
        {0, cos(rads0), -sin(rads0), 0},
        {0, sin(rads0), cos(rads0), 0},
        {0, 0, 0, 1}
    }};
    *yRMat = (T3DMat4){{
        {cos(rads1), 0, sin(rads1), 0},
        {0, 1, 0, 0},
        {-sin(rads1), 0, cos(rads1), 0},
        {0, 0, 0, 1}
    }};
    *zRMat = (T3DMat4){{
        {cos(rads2), -sin(rads2), 0, 0},
        {sin(rads2), cos(rads2), 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};
    //this funciton seems to multiply B*A meaning, that since our rots are non commutative
    //that we should have xRMat be the first 'b' matrix
    t3d_mat4_mul(xRMat, xRMat, scaleMat);
    t3d_mat4_mul(xRMat, yRMat, xRMat);
    t3d_mat4_mul(xRMat, zRMat, xRMat);
    xRMat->m[3][0] = pos.v[0];
    xRMat->m[3][1] = pos.v[1];
    xRMat->m[3][2] = pos.v[2];
    t3d_mat4_to_fixed_3x4(out, xRMat);
}
void spin_slot(player *plyr, float dt) 
{
    plyr->sl.slotTimer += dt;
    //slotTimer[plyr->playerNumber] += dt;
    
    for (int i = 0; i < 3; i++) 
    {
        if (plyr->sl.slotTimer >= (SLOT_TIMER - (1 + (.5f * i))) && !plyr->sl.finished[i]) 
        {
            //float rad = T3D_DEG_TO_RAD(deca(plyr->sl.currentSelection[i]));
            int deg = rad_to_deg(plyr->sl.rot[i].v[0]);
            int final = deca(plyr->sl.currentSelection[i])%360;
            if (deg >= (final - 16) && deg <= (final + 16))
            {
                plyr->sl.rot[i].v[0] = T3D_DEG_TO_RAD(final);
                plyr->sl.finished[i] = true;
                continue;
            }
        }
        if (!plyr->sl.finished[i]) 
        {
            plyr->sl.rot[i].v[0] += T3D_DEG_TO_RAD(SLOT_ROTATION_SPEED * dt);
        }
        
    }
    if (plyr->sl.finished[0] && plyr->sl.finished[1] && plyr->sl.finished[2]) 
    {
        plyr->sl.isSpinning = false;
        //if all slots are the same
        if (plyr->sl.currentSelection[0] == plyr->sl.currentSelection[1] && plyr->sl.currentSelection[1] == plyr->sl.currentSelection[2])
        {
            player* target = get_current_player(plyr->sl.left);
            player* other = get_current_player(!plyr->sl.left);
            activate_ability(plyr->sl.currentSelection[0], target, other);
            if (plyr->sl.currentSelection[0] != BOMB)
            {
                wav64_play(&sfx_slot_win, 29);
            }
            //activate_ability(HEART, target, other);
        }
    }
}
void slot_settle(player *plyr, int value) 
{
    wav64_play(&sfx_slot_start, 31);
    wav64_play(&sfx_slot_run, 30);
    for(int i = 0; i < 3; i++) 
    {
        plyr->sl.finished[i] = false;
        if (value == -1) 
        {
            if (i == 1) 
            {
                plyr->sl.currentSelection[1] = (plyr->sl.currentSelection[0] + 2)%10;
            } else 
            {
                plyr->sl.currentSelection[i] = rand()%10;
            }
        } else
        {
            plyr->sl.currentSelection[i] = value;
        }
    }
}
void player_loop(player *plyr, float deltaTime, joypad_port_t port) 
{
    if (isBattle && !isDead) 
    {
        if (plyr->isHuman) {
        joypad_buttons_t btn = joypad_get_buttons_pressed(port);
        if (!plyr->sl.isSpinning) 
        {
            if (btn.a) 
            {
                //start spinning
                plyr->sl.isSpinning = true;
                plyr->sl.slotTimer = 0;
                int value = randomSelection();
                //value = LIGHTNING; //for testing purposes
                //value = HEART;
                slot_settle(plyr, value);
            }
        } else 
        {
            spin_slot(plyr, deltaTime);
        }
        if (btn.l && !plyr->sl.left)
        {
            plyr->sl.left = true;
        }
        if (btn.r && plyr->sl.left)
        {
            plyr->sl.left = false;
        }
        } else 
        {
            if (!plyr->sl.isSpinning) 
            {
                int r = rand()%100;
                if (r < 2) //low odds but runs every frame
                {
                    plyr->sl.isSpinning = true;
                    plyr->sl.slotTimer = 0;
                    plyr->sl.left = rand()%2;
                    int value = randomSelection();
                    slot_settle(plyr, value);
                }
            } else 
            {
                spin_slot(plyr, deltaTime);
            }
        }
    }

    wheel_matrix(plyr, plyr->sl.pos[0], plyr->sl.rot[0], plyr->sl.firstMatFP);
    wheel_matrix(plyr, plyr->sl.pos[1], plyr->sl.rot[1], plyr->sl.secondMatFP);
    wheel_matrix(plyr, plyr->sl.pos[2], plyr->sl.rot[2], plyr->sl.thirdMatFP);
}


void player_fixedloop(player *player, float deltaTime, joypad_port_t port)
{

}
bool check_game_end()
{
    //first round robin
    if (sequenceLimit == 6 && sequenceIndex == 6)
    {
        int zeroWinIndex = -1;
        for (int i = 0; i < 4; i++)
        {
            //if a player has more than 3 wins the game is over,
            if (players[i].wins == 3)
            {
                core_set_winner(players[i].playerNumber);
                winner = players[i].playerNumber;
                gameOver = true;
                return true;
            }
            if (players[i].wins == 0)
            {
                zeroWinIndex = i;
            }
        }
        if (zeroWinIndex != -1)
        {
            sequenceLimit = 3;
            sequenceIndex = 0;
            //for(int i = zeroWinIndex; )
            //we have a three way tie
            tieFighterindex[0] = (zeroWinIndex + 1 )%4;
            tieFighterindex[1] = (zeroWinIndex + 2 )%4;
            tieFighterindex[2] = (zeroWinIndex + 3 )%4;

            battle_start(&players[tieFighterindex[0]],
            &players[tieFighterindex[1]]);
            return true;
        } else 
        {
            int firstIndex = -1;
            int secondIndex = 0;
            for (int i = 0; i < 4; i++)
            {
                if (players[i].wins == 2)
                {
                    if (firstIndex == -1)
                    {
                        firstIndex = i;
                        continue;
                    }
                    secondIndex = i;
                }
            }
            battle_start(&players[firstIndex], &players[secondIndex]);
            //after this final battle, it will roll back around and find a player with 3 wins
            return true;
            //we have a two way tie
        }
    }
    if (sequenceLimit == 3)  
    {
        if (sequenceIndex < 3)
        {
            
            int secondIndex = (sequenceIndex == 2) ? 2 : sequenceIndex + 1;
            battle_start(&players[tieFighterindex[sequenceIndex>>1]],
            &players[tieFighterindex[secondIndex]]);
            return true;
        } else 
        {
            //another tie
            if (players[tieFighterindex[0]].wins == players[tieFighterindex[1]].wins
                && players[tieFighterindex[1]].wins == players[tieFighterindex[2]].wins)
            {
                sequenceIndex = 0;
                battle_start(&players[tieFighterindex[0]], &players[tieFighterindex[1]]);
                return true;
            } else //we have a winner thank god
            {
                player lastWinner = players[tieFighterindex[0]];
                for (int i = 1; i < 3; i++)
                {
                    if (players[tieFighterindex[i]].wins > lastWinner.wins)
                    {
                        lastWinner = players[tieFighterindex[i]];
                    }
                }
                core_set_winner(lastWinner.playerNumber);
                winner = lastWinner.playerNumber;
                gameOver = true;
                return true;
            }
        }
    }
    return false;
}
void minigame_fixedloop(float deltaTime)
{
    gameTimer += deltaTime;
    if (gameTimer < GAME_START_TIMER) 
    {
        //!!!! display splash screen
        //do I have to display in minigame loop?
        //I think so
        //make it look like wc3 load screen
        return;
    }
    
    if (!isBattle)
    {
        combatTimer += deltaTime;
        if (combatTimer >= COMBAT_COUNTDOWN_DELAY && !gameOver)
        {
            if (check_game_end()) 
            {
                //if game over we set flags in function
                //if tie we started a batte_start function inside check_game_end
            } else 
            {
                battle_start(&players[battleSequence[sequenceIndex][0]], 
                &players[battleSequence[sequenceIndex][1]]);
            }
            
            combatTimer = 0;
            if (sequenceIndex < 6)
            {
                sequenceIndex++;
            }
        }
    } else 
    {
        for (int i = 0; i < 4; i++) 
        {
            player_fixedloop(&players[i], deltaTime, core_get_playercontroller(i));
        }
        battle_fixedLoop(deltaTime);
    }
}

void draw_slot_ui() 
{
    PlyNum fighterNumbers[2] =
    {
        get_current_player(true)->playerNumber,
        get_current_player(false)->playerNumber
    };
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            T3DVec3 billboardPos = (T3DVec3){{
                players[i].sl.pos[j].v[0],
                players[i].sl.pos[1].v[1] + BILLBOARD_YOFFSET,
                players[i].sl.pos[1].v[2]
            }};

            T3DVec3 billboardScreenPos;
            t3d_viewport_calc_viewspace_pos(&viewport, &billboardScreenPos, &billboardPos);

            int x = floorf(billboardScreenPos.v[0]);
            int y = floorf(billboardScreenPos.v[1]);

            x -= (1 - i)*-10;
            y -= 20;
            
            //the outer and middle slots need a few adjustments
            if (i%3 == 0)
            {
                y -= 5;
            } 

            rdpq_sync_pipe(); // Hardware crashes otherwise
            rdpq_sync_tile(); // Hardware crashes otherwise

            rdpq_set_mode_standard();
            rdpq_mode_alphacompare(1);

            switch (j)
            {
                case 0:
                    if (players[i].sl.left)
                    {
                        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = fighterNumbers[j] },
                        FONT_BILLBOARD, x, y, "P%d", fighterNumbers[j]+1);
                    } else
                    {
                        rdpq_sprite_blit(LButton,x-5,y-10,NULL);
                    }
                
                    break;
                case 1:
                    if (!players[i].sl.left)
                    {
                        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = fighterNumbers[j] },
                        FONT_BILLBOARD, x, y, "P%d", fighterNumbers[j]+1);
                    } else
                    {
                        rdpq_sprite_blit(RButton,x-5,y-10,NULL);
                    }
                    break;
                case 2:
                    if (!players[i].sl.isSpinning)
                    {
                        rdpq_sprite_blit(AButton,x,y-10,NULL);
                    }
                    break;
            }
        }
    }
}

void minigame_loop(float deltaTime)
{
    uint8_t colorAmbient[4] = { 0xAA, 0xAA, 0xAA, 0xFF };
    uint8_t colorDir[4] = { 0xFF, 0xAA, 0xAA, 0xFF };

    //viewport far plane was 160, but our camera is 235 units away, we want to capture origin
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 250);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){ {0, 1, 0}});

    
    for (int i = 0; i < 4; i++) 
    {
        player_loop(&players[i], deltaTime, core_get_playercontroller(i));
    }
    battle_loop(deltaTime, syncPoint);

    // ======== Draw (3D) ======== //
    rdpq_attach(display_get(), depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    if (syncPoint)rspq_syncpoint_wait(syncPoint);

    if (gameTimer < GAME_START_TIMER)
    {
        //!!!! add splash screen here!!!!
    }
    if (isBattle) {
        for (int i = 0; i < 4; i++) 
        {
            rspq_block_run(players[i].sl.dplWheel);
            //rspq_block_run(players[i])
        }
    }
    
    if (isBattle || isDead) 
    {
        battle_draw();
    }
    syncPoint = rspq_syncpoint_new();
    
    if (isBattle)
    {
        for (int i = 0; i < 4; i++)
        {
            draw_slot_ui();
        }
        battle_ui(FONT_BILLBOARD, viewport);
    }
    if (gameOver) 
    {
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, };
        rdpq_text_printf(&textparms, FONT_TEXT, 0, 100, "Player %d wins!", winner+1);
        if (combatTimer >= GAME_OVER_DELAY)
        {
            minigame_end();
        }
    }

    rdpq_sync_tile();
    rdpq_sync_pipe();

    rdpq_detach_show();
}
void player_cleanup(player *player) {
    rspq_block_free(player->sl.dplWheel);

    free_uncached(player->sl.firstMatFP);
    free_uncached(player->sl.secondMatFP);
    free_uncached(player->sl.thirdMatFP);
}

void minigame_cleanup()
{
    for (int i = 0; i < 4; i++) {
        player_cleanup(&players[i]);
        fighter_cleanup(&players[i]);
    }
    battle_cleanup();
    wav64_close(&sfx_slot_start);
    wav64_close(&sfx_slot_run);
    wav64_close(&sfx_slot_win);

    //free_uncached(modelMatFP);
    free_uncached(scaleMat);
    free_uncached(xRMat);
    free_uncached(yRMat);
    free_uncached(zRMat);
    t3d_model_free(wheelModel);
    t3d_model_free(fighterModel);
    sprite_free(AButton);
    sprite_free(LButton);
    sprite_free(RButton);
    rdpq_text_unregister_font(FONT_BILLBOARD);
    rdpq_font_free(fontBillboard);
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
    t3d_destroy();
    display_close();
}