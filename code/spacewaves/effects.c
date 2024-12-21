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
#include "gfx.h"
#include "effects.h"

effectdata_t effects;

void effects_init(){
    for(int i = 0; i < MAX_EFFECTS; i++){
        effects.exp3d[i].enabled = false;
        effects.exp3d[i].matx = malloc_uncached(sizeof(T3DMat4FP));

        effects.exp2d[i].enabled = false;
        
    }
    effects.screenshaketime = 0.0f;
    for(int i = 0; i < MAXPLAYERS; i++)
        effects.rumbletime[i] = 0;
}
void effects_update(){
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(effects.exp3d[i].enabled){
            effects.exp3d[i].time -= DELTA_TIME;
            if(effects.exp3d[i].time <= 0) effects.exp3d[i].enabled = false;
        }
        if(effects.exp2d[i].enabled){
            effects.exp2d[i].time -= DELTA_TIME;
            if(effects.exp2d[i].time <= 0) effects.exp2d[i].enabled = false;
        }
    }
    for(int i = 0; i < MAXPLAYERS; i++){
        if(effects.rumbletime[i] > 0){
            effects.rumbletime[i] -= DELTA_TIME;
            joypad_set_rumble_active(i, true);
        } else joypad_set_rumble_active(i, false);
    }
    if(effects.screenshaketime > 0){
        effects.screenshaketime -= DELTA_TIME;
    } else effects.screenshaketime = 0.0f;
    for(int i = 0; i < 4; i++){
        if(effects.ambientlight.r > 3) effects.ambientlight.r -=3;
        if(effects.ambientlight.g > 3) effects.ambientlight.g -=3;
        if(effects.ambientlight.b > 3) effects.ambientlight.b -=3;
        if(effects.ambientlight.a > 3) effects.ambientlight.a -=3;
    }
    world.sun.ambient = effects.ambientlight;
}

void effects_add_exp3d(T3DVec3 pos, color_t color){
    int i = 0; while(effects.exp3d[i].enabled && i < MAX_EFFECTS - 1) i++;

    effects.exp3d[i].enabled = true;
    effects.exp3d[i].position = pos;
    effects.exp3d[i].color = color;
    effects.exp3d[i].time = 2.0f;
    wav64_play(&sounds[snd_explosion_blast], SFX_CHANNEL_EXPLOSION);
}

void effects_add_exp2d(T3DVec3 pos, color_t color){
    int i = 0; while(effects.exp2d[i].enabled && i < MAX_EFFECTS - 1) i++;

    effects.exp2d[i].enabled = true;
    effects.exp2d[i].position = pos;
    effects.exp2d[i].color = color;
    effects.exp2d[i].color.a = 255;
    effects.exp2d[i].time = 1.6f;
}

void effects_add_rumble(joypad_port_t port, float time){
    effects.rumbletime[port] += time;
}

void effects_add_shake(float time){
    effects.screenshaketime += time;
}

void effects_add_ambientlight(color_t light){
    effects.ambientlight.r += light.r;
    effects.ambientlight.g += light.g;
    effects.ambientlight.b += light.b;
    effects.ambientlight.a += light.a;
}

void effects_draw(){
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    rdpq_mode_dithering(dither);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_zbuf(false, false);
    rdpq_mode_persp(true);
    T3DMaterial* mat =  t3d_model_get_material(models[EXP3D], "f3d.exp3d.rgba32");
    T3DObject* obj = t3d_model_get_object_by_index(models[EXP3D], 0);
    t3d_model_draw_material(mat, NULL);
    for(int i = 0; i < MAX_EFFECTS; i++)
        if(effects.exp3d[i].enabled){
            float scale = 2.0 - effects.exp3d[i].time;
            effects.exp3d[i].color.a = effects.exp3d[i].time * (255 / 2);
            rdpq_set_env_color(effects.exp3d[i].color);
            t3d_mat4fp_from_srt_euler(effects.exp3d[i].matx,
            (float[3]){scale,scale,scale},
            (float[3]){0,0,0},
            effects.exp3d[i].position.v);
            t3d_matrix_push(effects.exp3d[i].matx);
            rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
            t3d_model_draw_object(obj, NULL);
            rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
            t3d_matrix_pop(1);
        }
    
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    rdpq_mode_dithering(dither);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_zbuf(false, false);
    for(int i = 0; i < MAX_EFFECTS; i++)
        if(effects.exp2d[i].enabled){
            T3DVec3 viewpos;
            t3d_viewport_calc_viewspace_pos(&viewport, &viewpos, &(effects.exp2d[i].position));
            rdpq_set_prim_color(effects.exp2d[i].color);
            float xpos = viewpos.v[0];
            float ypos = viewpos.v[1];
            int index = 16 - (effects.exp2d[i].time * 10);
            if(index >= 15) index = 15;
            if(index <= 0) index = 0;

            if(gfx_pos_within_viewport(xpos, ypos))
                rdpq_sprite_blit(sprites[spr_exp], xpos, ypos, &(rdpq_blitparms_t){.cx = 16, .cy = 16, .s0 = (index % 4)*32, (index / 4)*32, .width = 32, .height = 32});
        }
}

void effects_close(){
    for(int i = 0; i < MAX_EFFECTS; i++){
        effects.exp3d[i].enabled = false;
        if(effects.exp3d[i].matx) free_uncached(effects.exp3d[i].matx);
        effects.exp3d[i].matx = NULL;
        effects.exp2d[i].enabled = false;
    }
    for(int i = 0; i < MAXPLAYERS; i++)
        joypad_set_rumble_active(i, false);
    
}
