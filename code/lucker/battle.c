#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
//#include "lucker.h"
#include "battle.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

//all chances are out of 100, so that we don't have to deal with floats
//when we gen random numbers we don't want to convert floats to values
#define DEFAULT_STUN_DURATION 1.5f
#define DEFAULT_ATTACK_CD .35f
#define DEFAULT_HP 425
#define DEFAULT_CRIT_CHANCE 15
#define DEFAULT_CRIT_MULT 2
#define DEFAULT_EVADE_CHANCE 15
#define DEFAULT_BASH_CHANCE 12
#define DEFAULT_BASH_DAMAGE 20
#define DEFAULT_DAMAGE 7

#define COMBAT_DEATH_DELAY 5

#define POWERBAR_WIDTH 50
#define POWERBAR_HEIGHT 6
#define POWERBAR_YOFFSET 110

player* currentBattlers[2];

float abilityCoolDowns[] = 
    {
        2,
        0,
        0,
        0,
        3,
        2,
        3,
        1.5f,
        3,
        3
    };
const char messages[4][5] =
{
    " ",
    "Crit!",
    "Miss!",
    "Stun!"
};
bool isBattle;
bool isDead;
player* battleVictor;
float deathTimer;

T3DVec3 fighter_start_positions[2] = 
{
    (T3DVec3){{-100,-25,25}},
    (T3DVec3){{100,-25,25}},
};
T3DVec3 fighter_start_rotations[2] = 
{
    (T3DVec3){{0,-1.5f,0}},
    (T3DVec3){{0,1.5f,0}},
};

wav64_t sfx_bite;
wav64_t sfx_cheer;
wav64_t sfx_boom;

void battle_init() 
{
    wav64_open(&sfx_bite, "rom:/lucker/Bite.wav64");
    wav64_open(&sfx_cheer, "rom:/lucker/Cheer.wav64");
    wav64_open(&sfx_boom, "rom:/lucker/Periander.wav64");
}
void battle_player_init (player *player, color_t color, T3DModel* model) 
{
    player->fighter.fighterMatFP = malloc_uncached(sizeof(T3DMat4FP));

    player->fighter.skel = t3d_skeleton_create(model);
    player->fighter.color = color;
    player->fighter.model = model;
    //player->fighter.skelBlend = t3d_skeleton_clone(&player->fighter.skel, false); // optimized for blending, has no matrices

    // Now create animation instances (by name), the data in 'model' is fixed,
    // whereas 'anim' contains all the runtime data.
    // Note that tiny3d internally keeps no track of animations, it's up to the user to manage and play them.
    player->fighter.animIdle = t3d_anim_create(model, "Snake_Idle");
    t3d_anim_attach(&player->fighter.animIdle, &player->fighter.skel); // tells the animation which skeleton to modify

    player->fighter.animWalk = t3d_anim_create(model, "Snake_Walk");
    t3d_anim_attach(&player->fighter.animWalk, &player->fighter.skel);

    player->fighter.animDeath = t3d_anim_create(model, "Snake_Death");
    t3d_anim_set_looping(&player->fighter.animDeath, false); // don't loop this animation
    t3d_anim_set_playing(&player->fighter.animDeath, false); // start in a paused state
    t3d_anim_set_speed(&player->fighter.animDeath, .65f);
    t3d_anim_attach(&player->fighter.animDeath, &player->fighter.skel);

    player->fighter.animJump = t3d_anim_create(model, "Snake_Jump");
    t3d_anim_set_looping(&player->fighter.animJump, true); // don't loop this animation
    t3d_anim_set_playing(&player->fighter.animJump, false); // start in a paused state
    t3d_anim_attach(&player->fighter.animJump, &player->fighter.skel);

    // multiple animations can attach to the same skeleton, this will NOT perform any blending
    // rather the last animation that updates "wins", this can be useful if multiple animations touch different bones
    player->fighter.animAttack = t3d_anim_create(model, "Snake_Attack");
    t3d_anim_set_speed(&player->fighter.animAttack, 2.0f);
    t3d_anim_set_looping(&player->fighter.animAttack, false); // don't loop this animation
    t3d_anim_set_playing(&player->fighter.animAttack, false); // start in a paused state
    t3d_anim_attach(&player->fighter.animAttack, &player->fighter.skel);

    //not using rspq block cuz we can't change color during block
    /*rspq_block_begin();
    t3d_matrix_push(player->fighter.fighterMatFP);
    rdpq_set_prim_color(b);
    t3d_model_draw_skinned(model, &player->fighter.skel); // as in the last example, draw skinned with the main skeleton

    t3d_matrix_pop(1);
    player->fighter.dplFighter = rspq_block_end();*/

}
void fighter_start (player* player, T3DVec3 pos, T3DVec3 rot) 
{
    //hate this but needs to be done
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };
    player->fighter.pos = pos;
    player->fighter.rot = rot;

    player->fighter.isAttack = false;
    player->fighter.isStunned = false;
    player->fighter.boom = false;
    player->fighter.color = colors[player->playerNumber];
    player->fighter.hp = DEFAULT_HP;
    
}

void battle_start (player *playerOne, player *playerTwo) 
{
    mixer_ch_set_vol(30, 0.15f, 0.15f);
    isBattle = true;
    isDead = false;
    deathTimer = 0;
    currentBattlers[0] = playerOne;
    currentBattlers[1] = playerTwo;

    for (int i = 0; i < 2; i++)
    {
        currentBattlers[i]->fighter.onLeftSide = i == 0 ? true : false;
        fighter_start(currentBattlers[i], fighter_start_positions[i], fighter_start_rotations[i]);
    }
}
void ability_finish(player *source, int abilityIndex)
{
    source->fighter.abilityTimers[abilityIndex] = 0;
    source->fighter.abilityActive[abilityIndex] = false;
}

void add_message_to_board(player *p, int index)
{
    for (int i = 2; i >= 0; i--)
    {
        p->fighter.messageboard[i + 1] = p->fighter.messageboard[i];
    }
    p->fighter.messageboard[0] = index;
}
void deal_damage(player *src, player* dst) 
{
    int evasionRand = rand()%100;
    if (!dst->fighter.abilityActive[EVADE_REMOVE]) 
    {
        int evchnce = DEFAULT_EVADE_CHANCE + (dst->fighter.abilityActive[EVADE] * DEFAULT_EVADE_CHANCE);
        //basically if the random number is below the evade chance (which could be boosted by buff)
        if (evasionRand < evchnce)
        {
            add_message_to_board(dst,2);
            return;
        }
    }

    //wouldn't be a lucker game if default damage didn't have some randomness to it
    int damage = (DEFAULT_DAMAGE + (src->fighter.abilityActive[SWORD] * DEFAULT_DAMAGE)) + (rand()%4);

    int critRand = rand()%100;
    if (critRand < (DEFAULT_CRIT_CHANCE + (dst->fighter.abilityActive[CRIT] *2* DEFAULT_CRIT_CHANCE)))
    {
        //CRITICAL HIT!!!
        damage *= DEFAULT_CRIT_MULT;
        dst->fighter.lastDamageCrit = damage;
        add_message_to_board(dst,1);
    }

    int bashRand = rand()%100;

    if (bashRand < (DEFAULT_BASH_CHANCE))
    {
        damage += 20;
        dst->fighter.isStunned = true;
        add_message_to_board(dst,3);
    }
    
    if (src->fighter.abilityActive[VAMPIRISM])
    {
        //SUCK THE BLOOD
        src->fighter.hp += damage;
    }

    dst->fighter.hp -= damage;
    wav64_play(&sfx_bite, 28);

    
}
void ability_fixedLoop(player *p, float dt)
{
    for (int i = 0; i < 10; i++)
    {
        if (abilityCoolDowns[i] == 0)
        {
            continue;
        }
        if (p->fighter.abilityActive[i])
        {
            p->fighter.abilityTimers[i] += dt;
            if (p->fighter.abilityTimers[i] >= abilityCoolDowns[i])
            {
                ability_finish(p,i);
            }
        }
    }
}
void battle_fixedLoop(float dt) 
{
    if (isDead) 
    {
        deathTimer += dt;
        if (deathTimer >= COMBAT_DEATH_DELAY)
        {
            battle_end(battleVictor);
            isDead = false;
        }
        return;
    }
    //happens every tick
    //responsible for fighters attacking eachother!
    //and for calculating ability timers and cd
    for (int i = 0; i < 2; i++) {
        if (currentBattlers[i]->fighter.hp <= 0)
        {
            isDead = true;
            mixer_ch_set_vol(30, 0.0f, 0.0f);
            
            
            if (currentBattlers[i]->fighter.boom)
            {
                wav64_play(&sfx_boom, 26);
            } else 
            {
                wav64_play(&sfx_cheer, 27);
            }
            //do these to make death anim play
            currentBattlers[i]->fighter.isAttack = false;
            currentBattlers[i]->fighter.isStunned = false;
            currentBattlers[i]->fighter.abilityActive[LIGHTNING] = false;
            player* p = currentBattlers[(i+1)%2];
            p->fighter.isAttack = false;
            p->fighter.isStunned = false;
            p->fighter.abilityActive[LIGHTNING] = false;
            t3d_anim_set_playing(&currentBattlers[i]->fighter.animDeath, true);
            t3d_anim_set_time(&currentBattlers[i]->fighter.animDeath, 0.0f);
            t3d_anim_set_playing(&p->fighter.animJump, true);
            t3d_anim_set_time(&p->fighter.animJump, 0.0f);
            
            battleVictor = p;
        }
        currentBattlers[i]->fighter.attackTimer += dt;

        ability_fixedLoop(currentBattlers[i], dt);

        //BASHED
        if (currentBattlers[i]->fighter.isStunned)
        {
            currentBattlers[i]->fighter.stunTimer += dt;
            if (currentBattlers[i]->fighter.stunTimer >= DEFAULT_STUN_DURATION)
            {
                currentBattlers[i]->fighter.stunTimer = 0;
                currentBattlers[i]->fighter.isStunned = false;
            }
        }

        //STUNNED
        if (currentBattlers[i]->fighter.abilityActive[LIGHTNING] || currentBattlers[i]->fighter.isStunned)
        {
            continue;
        }
        float cd = currentBattlers[i]->fighter.abilityActive[BURST] ? DEFAULT_ATTACK_CD / 2 : DEFAULT_ATTACK_CD;
        if (currentBattlers[i]->fighter.attackTimer >= cd)
        {
            currentBattlers[i]->fighter.isAttack = true;
            t3d_anim_set_playing(&currentBattlers[i]->fighter.animAttack, true);
            t3d_anim_set_time(&currentBattlers[i]->fighter.animAttack, 0.0f);
            deal_damage(currentBattlers[i], currentBattlers[(i+1)%2]);
            currentBattlers[i]->fighter.attackTimer = 0;
        }
    }
}
void battle_draw() {
    for (int i = 0; i < 2; i++)
    {
        //not using rspq block because we can't change color
        //rspq_block_run(currentBattlers[i]->fighter.dplFighter);
        t3d_matrix_push(currentBattlers[i]->fighter.fighterMatFP);
        rdpq_set_prim_color(currentBattlers[i]->fighter.color);
        t3d_model_draw_skinned(currentBattlers[i]->fighter.model, 
        &currentBattlers[i]->fighter.skel); 
        t3d_matrix_pop(1);
    }
}
void battle_ui(int fontIndex, T3DViewport viewport)
{
    //320x240
    rdpq_sync_tile();
    rdpq_sync_pipe(); 
    const int hp[2] = {15, 250};
    const int fullhp[2] = {15, 250};
    const int y = 15;
    for (int i = 0; i < 2; i++)
    {

        rdpq_textparms_t textparms = {.style_id = currentBattlers[i]->playerNumber};
        rdpq_text_printf(&textparms, fontIndex, hp[i], y, "P%d HP W:%d",
        currentBattlers[i]->playerNumber+1, currentBattlers[i]->wins);


        int curhp = (int)currentBattlers[i]->fighter.hp;

        //rdpq_textparms_t textparms = {.style_id = currentBattlers[i]->playerNumber};
        
        rdpq_text_printf(&textparms, fontIndex, fullhp[i], y*2, "%d / %d",curhp, DEFAULT_HP);
        

        /*T3DVec3 FiPos = (T3DVec3)
        {{
            players[i].sl.pos[j].v[0],
            players[i].sl.pos[1].v[1] + BILLBOARD_YOFFSET,
            players[i].sl.pos[1].v[2]
        }};*/
        T3DVec3 barPos = currentBattlers[i]->fighter.pos;
        barPos.v[1] += POWERBAR_YOFFSET;

        T3DVec3 billboardScreenPos;
        t3d_viewport_calc_viewspace_pos(&viewport, &billboardScreenPos, &barPos);

        int x = floorf(billboardScreenPos.v[0]) - 8;
        int y = floorf(billboardScreenPos.v[1]);

        if (i == 1)
        {
            x -= 30;
        }
        
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        float t = (float)curhp/DEFAULT_HP;
        
        //r is the final value and g is the starting value
        int r = floorf(t*(-255) + 255);
        int g = floorf((float)(t*255));

        rdpq_set_prim_color(RGBA32(r,g,0,255));
        int width = (curhp*POWERBAR_WIDTH)/DEFAULT_HP;
        if (width > POWERBAR_WIDTH) width = POWERBAR_WIDTH;
        rdpq_fill_rectangle(x, y, x+width, (y)+POWERBAR_HEIGHT);

        y -= 10;
        x += 5;
        for (int j = 0; j < 4; j++)
        {
            //rdpq_text_printf(&textparms, fontIndex, x, y - (j * 15), "test");
            
            //rdpq_text_printf(&textparms, fontIndex, x, y - (j * 15), 
            //messages[currentBattlers[i]->fighter.messageboard[0]]);
            
            /*if (currentBattlers[i]->fighter.messageboard[j] == 1) 
            {
                //rdpq_text_printf(&textparms, fontIndex, x, y - (j * 15), "%d",
                //currentBattlers[i]->fighter.lastDamageCrit);
            } else 
            {
                
            }*/
            if (currentBattlers[i]->fighter.messageboard[j] == 1) 
            {
                rdpq_text_printf(&textparms, fontIndex, x, y - (j * 15),
                 "%d!",currentBattlers[i]->fighter.lastDamageCrit);
            } else 
            {
                rdpq_text_printn(&textparms, fontIndex, x, y - (j * 15), 
                messages[currentBattlers[i]->fighter.messageboard[j]], sizeof(char)*5);
            }
        }
    }
    
}
void battle_loop(float dt, rspq_syncpoint_t syncPoint) 
{
    //happens every frame
    //draw the mayhem!
    //respoinsible for drawing bots and animating slots
    if (isBattle)
    {
        for (int i = 0; i < 2; i++)
        {
            
            if (currentBattlers[i]->fighter.abilityActive[LIGHTNING] || currentBattlers[i]->fighter.isStunned) 
            {
                t3d_anim_update(&currentBattlers[i]->fighter.animWalk, dt);
            } else if(currentBattlers[i]->fighter.isAttack) 
            {
                t3d_anim_update(&currentBattlers[i]->fighter.animAttack, dt); // attack animation now overrides the idle one
                if(!currentBattlers[i]->fighter.animAttack.isPlaying)
                {
                    currentBattlers[i]->fighter.isAttack = false;
                }
            } else if (currentBattlers[i]->fighter.hp <= 0) 
            {
                t3d_anim_update(&currentBattlers[i]->fighter.animDeath, dt);
            } else if (isDead && currentBattlers[i]->fighter.hp > 0)
            {
                t3d_anim_update(&currentBattlers[i]->fighter.animJump, dt);
            } else
            {
                t3d_anim_update(&currentBattlers[i]->fighter.animIdle, dt);
            }
            if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
            t3d_skeleton_update(&currentBattlers[i]->fighter.skel);

            t3d_mat4fp_from_srt_euler(currentBattlers[i]->fighter.fighterMatFP,
                (float[3]){.5f, .5f, .5f},
                currentBattlers[i]->fighter.rot.v,
                currentBattlers[i]->fighter.pos.v
            );
        }
    }
    
}
//return the left palyer if true, otherwise return the right player
player* get_current_player(bool left) 
{
    return left ? currentBattlers[0] : currentBattlers[1];
}

void fighter_cleanup(player *player) 
{
    //rspq_block_free(player->fighter.dplFighter);

    t3d_skeleton_destroy(&player->fighter.skel);
    t3d_skeleton_destroy(&player->fighter.skelBlend);

    t3d_anim_destroy(&player->fighter.animIdle);
    t3d_anim_destroy(&player->fighter.animWalk);
    t3d_anim_destroy(&player->fighter.animDeath);
    t3d_anim_destroy(&player->fighter.animJump);
    t3d_anim_destroy(&player->fighter.animAttack);

    free_uncached(player->fighter.fighterMatFP);
}

void battle_cleanup() 
{
    wav64_close(&sfx_bite);
    wav64_close(&sfx_cheer);
    wav64_close(&sfx_boom);
}

void battle_end(player *victor) 
{
    victor->wins++;
    isBattle = false;
}