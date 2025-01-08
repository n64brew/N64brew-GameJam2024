#include <libdragon.h>
#include <time.h>
#include <unistd.h>
#include <display.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "../../core.h"
#include "../../minigame.h"
#include "world.h"
#include "gamestatus.h"
#include "station.h"
#include "enemycraft.h"
#include "gfx.h"
#include "bonus.h"

/*#define MAX_BONUSES 10

typedef enum bonustype_s{
    BONUS_POINTS,
    BONUS_ROCKETS,
    BONUS_SHIELD,
    BONUS_UPGRADE
} bonustype_t;

typedef struct bonus_s{
    bool enabled;
    bonustype_t type;
    T3DVec3 polarpos;
    float time;
    float speedx, speedy;
} bonus_t;

void bonus_update();
*/

bonus_t bonuses[MAX_BONUSES];
float nextbonustime = 5.0f;

void bonus_init(){
    for(int b = 0; b < MAX_BONUSES; b++){
        bonuses[b].enabled = false;
    }
}

void bonus_update(){
    nextbonustime -= DELTA_TIME;
    if(nextbonustime < 0){
        nextbonustime = frandr(2.0f, 6.0f);

        int b = 0;
        while(b < MAX_BONUSES && bonuses[b].enabled) b++;
        if(b < MAX_BONUSES){
            bonuses[b].enabled = true;
            bonuses[b].speedx = frandr(-0.05, 0.05);
            bonuses[b].speedy = frandr(-0.05, 0.05);
            bonuses[b].polarpos = (T3DVec3){{frandr(T3D_DEG_TO_RAD(-40), T3D_DEG_TO_RAD(40)), frandr(T3D_DEG_TO_RAD(-180), T3D_DEG_TO_RAD(180)), 60.0f}};
            bonuses[b].time = 25.0f;
            bonuses[b].type = randm(4);
        }

    }

    for(int b = 0; b < MAX_BONUSES; b++){
        bonuses[b].time -= DELTA_TIME;
        bonuses[b].polarpos.v[0] += bonuses[b].speedx * DELTA_TIME;
        bonuses[b].polarpos.v[1] += bonuses[b].speedy * DELTA_TIME;
        if(bonuses[b].time < 0)
            bonuses[b].enabled = false;       
    }
}

void bonus_apply(int bindex, PlyNum playernum, DefenseStation* station, int craft){
    if(bindex >= MAX_BONUSES || bindex < 0) return;
    bonus_t* bonus = &bonuses[bindex];
    switch(bonus->type){
        case BONUS_POINTS:
            gamestatus.playerscores[playernum] += 1500;
            bonus->enabled = false;
            afx_wav64_play_wrapper(snd_reload, SFX_CHANNEL_BONUS);
            break;
        case BONUS_UPGRADE:
            if(station) station->arm.powerup = 10.0f;
            if(craft >= 0)   crafts[craft].arm.powerup = 10.0f;
            bonus->enabled = false;
            afx_wav64_play_wrapper(snd_pickup_shield, SFX_CHANNEL_BONUS);
            break;
        case BONUS_SHIELD:
            if(station) station->arm.shield = 10.0f;
            if(craft >= 0)   crafts[craft].arm.shield = 10.0f;
            bonus->enabled = false;
            afx_wav64_play_wrapper(snd_pickup_shield, SFX_CHANNEL_BONUS);
            break;
        case BONUS_ROCKETS:
            if(station) station->arm.rocketcount += 5;
            if(craft >= 0)   crafts[craft].arm.rocketcount += 3;
            bonus->enabled = false;
            afx_wav64_play_wrapper(snd_pickup_ammo, SFX_CHANNEL_BONUS);
            break;
    }
}

void bonus_draw(){
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_alphacompare(5);
    
    T3DVec3 worldpos, viewpos;
    float xpos, ypos;
    for(int b = 0; b < MAX_BONUSES; b++){
        bonus_t* bonus = &bonuses[b];
        if(bonus->enabled){
            worldpos = gfx_worldpos_from_polar(bonus->polarpos.v[0], bonus->polarpos.v[1], bonus->polarpos.v[2]);
                t3d_viewport_calc_viewspace_pos(&viewport, &viewpos, &worldpos);
                xpos = viewpos.v[0]; ypos = viewpos.v[1] - 16;
                if(gfx_pos_within_viewport(xpos, ypos) && t3d_vec3_dot(&station.forward, &worldpos) > 0.0f){
                    rdpq_sprite_blit(sprites[spr_ui_bonus1 + bonus->type], xpos, ypos, &(rdpq_blitparms_t){.cx = 16, .scale_x = sin(bonus->time * 3)});
                }
        }
    }
}

void bonus_close(){

}
