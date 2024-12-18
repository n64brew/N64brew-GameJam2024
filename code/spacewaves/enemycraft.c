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
#include "bonus.h"
#include "effects.h"
#include "enemycraft.h"

enemycraft_t crafts[3];

void crafts_init(PlyNum currentstation){
    PlyNum player = PLAYER_1;
    for(int c = 0; c < NUM_CRAFTS; c++, player++){
        if(player == currentstation) player++;
        crafts[c].bot = player >= core_get_playercount();
        crafts[c].currentplayer = player;
        crafts[c].currentplayerport = core_get_playercontroller(player);
        crafts[c].color = gfx_get_playercolor(player);
        crafts[c].maxhp = 100;
        crafts[c].hp = crafts[c].maxhp;
        crafts[c].pitch = frandr(-0.5f, 0.5f);
        crafts[c].yaw = frandr(-0.5f, 0.5f);
        crafts[c].pitchoff = 0;
        crafts[c].yawoff = 0;
        crafts[c].distance = 90;
        crafts[c].distanceoff = crafts[c].distance;
        crafts[c].matx = malloc_uncached(sizeof(T3DMat4FP));
        crafts[c].arm.rocketcount = 0;
        for(int b = 0; b < MAX_PROJECTILES; b++){
            crafts[c].arm.rockets[b].matx = malloc_uncached(sizeof(T3DMat4FP));
            crafts[c].arm.asteroids[b].matx = malloc_uncached(sizeof(T3DMat4FP));
            crafts[c].arm.rockets[b].enabled = false;
            crafts[c].arm.asteroids[b].enabled = false;
        }
        crafts[c].arm.asteroidnexttime = CURRENT_TIME + 0.5f;
        crafts[c].arm.powerup = 0;
        crafts[c].arm.shield = 0;
        crafts[c].enabled = true;
    }
}

void crafts_botlogic_getinput(enemycraft_t* craft, int index,joypad_inputs_t* outinput, joypad_buttons_t* outpressed, joypad_buttons_t* outheld){
    AiDiff diff = core_get_aidifficulty();
    int w,h, w2, h2;
    w = display_get_width();
    h = display_get_height();
    w2 = display_get_width() /  2;
    h2 = display_get_height() / 2;
    float md_toavoid = 0.5f;

    T3DVec3 cursorpos = (T3DVec3){{0,0,0}};
    T3DVec3 viewpos, cursorviewpos = (T3DVec3){{0,0,0}};
    if(diff >= DIFF_HARD){
        cursorpos = gfx_worldpos_from_polar(station.pitch, station.yaw, 1000);
        t3d_viewport_calc_viewspace_pos(&viewport, &cursorviewpos, &cursorpos);
        cursorviewpos.v[0] -= w2;
        cursorviewpos.v[1] -= h2;
    }

    T3DVec3 polarpos = (T3DVec3){{craft->pitchoff, craft->yawoff, craft->distanceoff}};
    T3DVec3 worldpos = gfx_worldpos_from_polar(craft->pitchoff, craft->yawoff, craft->distanceoff);
    t3d_viewport_calc_viewspace_pos(&viewport, &viewpos, &worldpos);
    float xpos = viewpos.v[0] - cursorviewpos.v[0]; float ypos = viewpos.v[1] - cursorviewpos.v[1];

    switch(diff){
        case DIFF_EASY:{
            md_toavoid = 0.45f;
            outheld->z = true;
        } break;
        case DIFF_MEDIUM:{
            md_toavoid = 0.3f;
            outpressed->a = true;
            outpressed->b = true;
            outheld->z = true;
        } break;
        case DIFF_HARD:{
            md_toavoid = 0.2f;
            outpressed->a = true;
            outpressed->b = true;
            outheld->l = true;
            outheld->r = true;
            outheld->z = true;
        } break;
    }

    float borderx, bordery;
    borderx = w * md_toavoid;
    bordery = h * md_toavoid;
    if(gfx_pos_within_rect(xpos, ypos, borderx, bordery, w - borderx, h - bordery) 
    && t3d_vec3_dot(&station.forward, &worldpos) > 0.0f){
        outinput->stick_x = fclampr((xpos - w2) * 100, -68, 68);
        outinput->stick_y = fclampr((h2 - ypos) * 100, -68, 68);
        outpressed->c_up = true;
        outpressed->c_down = false;
    } else{
        outinput->stick_x = 0;
        outinput->stick_y = 0;
        if(fabs(craft->pitchoff) > 0.25)
        outinput->stick_y = fclampr(craft->pitchoff * 100, -68, 68);

        if(diff >= DIFF_MEDIUM){
            float mindist = INFINITY; int bonusindex = -1;
            for(int b = 0; b < MAX_BONUSES; b++){
                if(bonuses[b].enabled){
                    float dist = t3d_vec3_distance(&bonuses[b].polarpos, &polarpos);
                    if(dist < mindist){
                        mindist = dist;
                        bonusindex = b;
                    }
                }
            }
            if(bonusindex >= 0){
                float targpitch = 1000 * (bonuses[bonusindex].polarpos.v[0] - polarpos.v[0]);
                float targyaw = 1000 * (bonuses[bonusindex].polarpos.v[1] - polarpos.v[1]);
                outinput->stick_x = fclampr(targyaw, -68, 68);
                outinput->stick_y = fclampr(-targpitch, -68, 68);
            }
        }

        if(diff >= DIFF_HARD){
            outpressed->c_up = false;
            outpressed->c_down = true;
        }
    }
}

void crafts_update(){
    for(int c = 0; c < NUM_CRAFTS; c++){
        if(crafts[c].enabled){
            if(crafts[c].hp <= 0) {
                crafts[c].enabled = false;
                effects_add_exp3d(gfx_worldpos_from_polar( crafts[c].pitchoff, crafts[c].yawoff, crafts[c].distanceoff * 25), crafts[c].color);
                effects_add_rumble(crafts[c].currentplayerport, 1.25f);
                effects_add_shake(1.25f);
                effects_add_ambientlight(RGBA32(50,50,25,0));
            }
            joypad_inputs_t input = {0};
            joypad_buttons_t pressed = {0}, held = {0};
            if(!crafts[c].bot){
                pressed = joypad_get_buttons_pressed(crafts[c].currentplayerport);
                input = joypad_get_inputs(crafts[c].currentplayerport);
                held = joypad_get_buttons_held(crafts[c].currentplayerport);
            } else crafts_botlogic_getinput(&crafts[c], c, &input, &pressed, &held);
            float speed = crafts[c].arm.powerup? 0.15f : 0.12f;
            crafts[c].yaw     += T3D_DEG_TO_RAD(speed * DELTA_TIME * input.stick_x);
            crafts[c].pitch   -= T3D_DEG_TO_RAD(speed * DELTA_TIME * input.stick_y);
            crafts[c].pitch = fclampr(crafts[c].pitch, T3D_DEG_TO_RAD(-60), T3D_DEG_TO_RAD(60));
            crafts[c].pitchoff = t3d_lerp(crafts[c].pitchoff, crafts[c].pitch, 0.1f);
            crafts[c].yawoff =   t3d_lerp(crafts[c].yawoff, crafts[c].yaw, 0.1f);
            T3DVec3 worldpos = gfx_worldpos_from_polar(
                        crafts[c].pitchoff, 
                        crafts[c].yawoff, 
                        crafts[c].distanceoff * 40);
            t3d_mat4fp_from_srt_euler(crafts[c].matx, 
                (float[3]){1.0f, 1.0f, 1.0f},
                (float[3]){0.0f, crafts[c].yawoff - T3D_DEG_TO_RAD(0.4f * input.stick_x), crafts[c].pitchoff + T3D_DEG_TO_RAD(0.4f * input.stick_y)},
                (float[3]){worldpos.v[0],worldpos.v[1],worldpos.v[2]});

            if(held.c_down) crafts[c].distance -= 5.0 * DELTA_TIME;
            if(held.c_up) crafts[c].distance += 5.0 * DELTA_TIME;
            crafts[c].distance = fclampr(crafts[c].distance, 30, 90);
            crafts[c].distanceoff = t3d_lerp(crafts[c].distanceoff, crafts[c].distance, 0.2f);

            if((pressed.a && crafts[c].arm.shield == 10.0f)){
                crafts[c].arm.shield -= DELTA_TIME;
                wav64_play(&sounds[snd_use_shield], SFX_CHANNEL_BONUS);
                effects_add_ambientlight(RGBA32(0,0,100,0));
            }
            if(crafts[c].arm.shield < 10.0f) crafts[c].arm.shield -= DELTA_TIME;
            crafts[c].arm.shield = fclampr(crafts[c].arm.shield, 0.0f, 10.0f);

            if((pressed.b && crafts[c].arm.powerup == 10.0f)){
                crafts[c].arm.powerup -= DELTA_TIME;
                wav64_play(&sounds[snd_use_powerup], SFX_CHANNEL_BONUS);
                effects_add_ambientlight(RGBA32(0,100,0,0));
            }
            if(crafts[c].arm.powerup < 10.0f) crafts[c].arm.powerup -= DELTA_TIME;
            crafts[c].arm.powerup = fclampr(crafts[c].arm.powerup, 0.0f, 10.0f);

            if(held.z && CURRENT_TIME >= crafts[c].arm.asteroidnexttime && !gamestatus.paused){
                int b = 0; while(crafts[c].arm.asteroids[b].enabled && b < MAX_PROJECTILES - 1) b++;
                crafts[c].arm.asteroids[b].enabled = true;
                crafts[c].arm.asteroids[b].polarpos = (T3DVec3){{crafts[c].pitchoff, crafts[c].yawoff, crafts[c].distanceoff}};
                crafts[c].arm.asteroids[b].rotation = frandr(0, 360);
                crafts[c].arm.asteroids[b].xspeed = frandr(-0.025, 0.025);
                crafts[c].arm.asteroids[b].yspeed = frandr(-0.025, 0.025);
                if(held.d_up) crafts[c].arm.asteroids[b].yspeed += 0.03;
                if(held.d_down) crafts[c].arm.asteroids[b].yspeed -= 0.03;
                if(held.d_right) crafts[c].arm.asteroids[b].xspeed += 0.03;
                if(held.d_left) crafts[c].arm.asteroids[b].xspeed -= 0.03;
                crafts[c].arm.asteroids[b].hp = randr(5, 25);
                crafts[c].arm.asteroidnexttime = CURRENT_TIME + 12.0f;
                wav64_play(&sounds[snd_hit], SFX_CHANNEL_EFFECTS);
                effects_add_ambientlight(RGBA32(50,50,50,0));
            }

            if((held.l || held.r) && CURRENT_TIME >= crafts[c].arm.rocketnexttime && crafts[c].arm.rocketcount > 0){
                int b = 0; while(crafts[c].arm.rockets[b].enabled && b < MAX_PROJECTILES - 1) b++;
                crafts[c].arm.rockets[b].enabled = true;
                crafts[c].arm.rockets[b].polarpos = (T3DVec3){{crafts[c].pitchoff, crafts[c].yawoff, crafts[c].distanceoff}};
                crafts[c].arm.rocketnexttime = CURRENT_TIME + 1.0f;
                crafts[c].arm.rockets[b].hp = 5;
                crafts[c].arm.rocketcount--;
                wav64_play(&sounds[snd_shoot_rocket], SFX_CHANNEL_ROCKET);
                effects_add_rumble(crafts[c].currentplayerport, 0.45f);
                effects_add_ambientlight(RGBA32(50,50,25,0));
            }

            for(int b = 0; b < MAX_BONUSES; b++){
                if(bonuses[b].enabled){
                    T3DVec3 ast_worldpos, encraft_worldpos;
                    ast_worldpos = gfx_worldpos_from_polar(
                        bonuses[b].polarpos.v[0],
                        bonuses[b].polarpos.v[1],
                        bonuses[b].polarpos.v[2]);
                    encraft_worldpos = gfx_worldpos_from_polar(
                        crafts[c].pitchoff,
                        crafts[c].yawoff,
                        bonuses[b].polarpos.v[2]);
                    float dist = t3d_vec3_distance(&ast_worldpos, &encraft_worldpos);
                    if(dist < 5.0f)
                        bonus_apply(b, crafts[c].currentplayer, NULL, c);
                }
            }
        }

            for(int b = 0; b < MAX_PROJECTILES; b++){
                if(crafts[c].arm.asteroids[b].enabled){
                    crafts[c].arm.asteroids[b].polarpos.v[0] += DELTA_TIME * crafts[c].arm.asteroids[b].xspeed;
                    crafts[c].arm.asteroids[b].polarpos.v[1] += DELTA_TIME * crafts[c].arm.asteroids[b].yspeed;
                    crafts[c].arm.asteroids[b].polarpos.v[2] -= DELTA_TIME * 3.0f;
                    crafts[c].arm.asteroids[b].rotation += DELTA_TIME;
                    if(crafts[c].arm.asteroids[b].hp <= 0) {
                        crafts[c].arm.asteroids[b].enabled = false;
                        effects_add_exp3d(gfx_worldpos_from_polar(
                            crafts[c].arm.asteroids[b].polarpos.v[0],
                            crafts[c].arm.asteroids[b].polarpos.v[1],
                            crafts[c].arm.asteroids[b].polarpos.v[2] * 25), 
                            RGBA32(255,255,255,255));
                        effects_add_rumble(station.currentplayerport, 0.45f);
                        effects_add_shake(0.75f);
                        effects_add_ambientlight(RGBA32(50,50,25,0));
                    }
                    if(crafts[c].arm.asteroids[b].polarpos.v[2] < 2.0f){
                        crafts[c].arm.asteroids[b].enabled = false;
                        effects_add_exp3d(gfx_worldpos_from_polar(
                            crafts[c].arm.asteroids[b].polarpos.v[0],
                            crafts[c].arm.asteroids[b].polarpos.v[1],
                            crafts[c].arm.asteroids[b].polarpos.v[2] * 25), 
                            RGBA32(255,255,255,255));
                            effects_add_rumble(station.currentplayerport, 0.75f);
                            effects_add_shake(1.00f);
                            effects_add_ambientlight(RGBA32(50,50,25,0));
                        float damage = 50;
                        if(!(station.arm.shield > 0.0f && station.arm.shield < 10.0f)){
                            station.hp -= damage;
                            gamestatus.playerscores[crafts[c].currentplayer] += damage * 40;
                        }
                    }
                }
                if(crafts[c].arm.rockets[b].enabled){
                    crafts[c].arm.rockets[b].polarpos.v[2] -= DELTA_TIME * 7.0f;
                    if(crafts[c].arm.rockets[b].hp <= 0) {
                        crafts[c].arm.rockets[b].enabled = false;
                        effects_add_exp3d(gfx_worldpos_from_polar(
                            crafts[c].arm.rockets[b].polarpos.v[0],
                            crafts[c].arm.rockets[b].polarpos.v[1],
                            crafts[c].arm.rockets[b].polarpos.v[2] * 25), 
                            RGBA32(255,255,255,255));
                        effects_add_rumble(station.currentplayerport, 0.45f);
                        effects_add_ambientlight(RGBA32(25,25,5,0));
                    }
                    if(crafts[c].arm.rockets[b].polarpos.v[2] < 2.0f){
                        crafts[c].arm.rockets[b].enabled = false;
                        effects_add_exp3d(gfx_worldpos_from_polar(
                            crafts[c].arm.rockets[b].polarpos.v[0],
                            crafts[c].arm.rockets[b].polarpos.v[1],
                            crafts[c].arm.rockets[b].polarpos.v[2] * 25), 
                            RGBA32(255,255,255,255));
                        effects_add_rumble(station.currentplayerport, 0.65f);
                        effects_add_shake(1.00f);
                        effects_add_ambientlight(RGBA32(50,50,25,0));
                        float damage = 65;
                        if(!(station.arm.shield > 0.0f && station.arm.shield < 10.0f)){
                            station.hp -= damage;
                            gamestatus.playerscores[crafts[c].currentplayer] += damage * 40;
                        }
                    }
                }
            }

    }  
}

void crafts_draw(){
    for(int c = 0; c < NUM_CRAFTS; c++){
        if(crafts[c].enabled){
            t3d_matrix_push(crafts[c].matx);

            T3DModelIter it = t3d_model_iter_create(models[ENEMYCRAFT], T3D_CHUNK_TYPE_OBJECT);
            if(crafts[c].arm.shield > 0.0f && crafts[c].arm.shield < 10.0f) rdpq_set_env_color(RGBA32(0x80,0x80,0xB0,0xFF));
            else rdpq_set_env_color(RGBA32(0x00,0x00,0x00,0xFF));
            while(t3d_model_iter_next(&it)){
                if(it.object->material) {
                    rdpq_mode_combiner(it.object->material->colorCombiner);
                    rdpq_mode_blender(it.object->material->blendMode);

                    t3d_fog_set_enabled(false);
                    rdpq_mode_zbuf(false, false);
                    T3DMaterialTexture *tex = &(it.object->material->textureA);
                    if(tex->texPath || tex->texReference){
                    if(tex->texPath && !tex->texture) tex->texture = sprite_load(tex->texPath);

                    rdpq_texparms_t texParam = (rdpq_texparms_t){};
                    texParam.s.mirror = tex->s.mirror;
                    texParam.s.repeats = tex->s.clamp ? 1 : REPEAT_INFINITE;
                    texParam.s.scale_log = 1;

                    texParam.t.mirror = tex->t.mirror;
                    texParam.t.repeats = tex->t.clamp ? 1 : REPEAT_INFINITE;
                    texParam.t.scale_log = 1;

                    rdpq_sprite_upload(TILE0, tex->texture, &texParam);
                    }
                }
                rdpq_mode_antialias(AA_STANDARD);
                rdpq_set_prim_color(crafts[c].color);
                rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_model_draw_object(it.object, NULL);
                rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
            }
            t3d_matrix_pop(1);
        }
    }

    color_t amb = RGBA32(0xA0, 0xA0,0xA0,0xFF);
    t3d_light_set_ambient((uint8_t*)&amb);
    t3d_light_set_directional(0, (uint8_t*)&world.sun.color, &world.sun.direction);
    t3d_light_set_count(1);
    T3DMaterial* mat =  t3d_model_get_material(models[ROCKET], "f3d.rocket");
    T3DObject* obj = t3d_model_get_object_by_index(models[ROCKET], 0);
    t3d_model_draw_material(mat, NULL);
    for(int c = 0; c < NUM_CRAFTS; c++)
        for(int b = 0; b < MAX_PROJECTILES; b++){
            if(crafts[c].arm.rockets[b].enabled){
                T3DVec3 worldpos = gfx_worldpos_from_polar(
                    crafts[c].arm.rockets[b].polarpos.v[0], 
                    crafts[c].arm.rockets[b].polarpos.v[1], 
                    crafts[c].arm.rockets[b].polarpos.v[2] * 50.0f);
                T3DMat4 rocketmat;
                t3d_mat4_from_srt_euler(&rocketmat,
                (float[3]){1.0f, 1.0f, 1.0f},
                (float[3]){0.0f, crafts[c].arm.rockets[b].polarpos.v[1], crafts[c].arm.rockets[b].polarpos.v[0] + T3D_DEG_TO_RAD(180.0f)},
                (float[3]){worldpos.v[0],worldpos.v[1],worldpos.v[2]});
                t3d_mat4_to_fixed(crafts[c].arm.rockets[b].matx, &rocketmat);
                t3d_matrix_push(crafts[c].arm.rockets[b].matx);
                rdpq_mode_antialias(AA_STANDARD);
                if(!crafts[c].arm.rocketdl){
                    rspq_block_begin();
                    rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                    t3d_model_draw_object(obj, NULL);
                    rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                    crafts[c].arm.rocketdl = rspq_block_end();
                } else rspq_block_run(crafts[c].arm.rocketdl);
                t3d_matrix_pop(1);
                            rdpq_sync_pipe(); // Hardware crashes otherwise
            rdpq_sync_tile(); // Hardware crashes otherwise
            }
        }
    t3d_light_set_ambient((uint8_t*)&world.sun.ambient);
    mat =  t3d_model_get_material(models[ASTEROID], "f3d.rock");
    obj = t3d_model_get_object_by_index(models[ASTEROID], 0);
    t3d_model_draw_material(mat, NULL);
    for(int c = 0; c < NUM_CRAFTS; c++)
        for(int b = 0; b < MAX_PROJECTILES; b++){
            if(crafts[c].arm.asteroids[b].enabled){
                T3DVec3 worldpos = gfx_worldpos_from_polar(
                    crafts[c].arm.asteroids[b].polarpos.v[0], 
                    crafts[c].arm.asteroids[b].polarpos.v[1], 
                    crafts[c].arm.asteroids[b].polarpos.v[2] * 50.0f);
                T3DMat4 mat;
                t3d_mat4_from_srt_euler(&mat,
                (float[3]){1.0f, 1.0f, 1.0f},
                (float[3]){0.0f, crafts[c].arm.asteroids[b].rotation, crafts[c].arm.asteroids[b].rotation},
                (float[3]){worldpos.v[0],worldpos.v[1],worldpos.v[2]});
                t3d_mat4_to_fixed(crafts[c].arm.asteroids[b].matx, &mat);
                t3d_matrix_push(crafts[c].arm.asteroids[b].matx);
                rdpq_mode_antialias(AA_STANDARD);
                if(!crafts[c].arm.asteroiddl){
                    rspq_block_begin();
                    rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                    t3d_model_draw_object(obj, NULL);
                    rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                    crafts[c].arm.asteroiddl = rspq_block_end();
                } else rspq_block_run(crafts[c].arm.asteroiddl);
                t3d_matrix_pop(1);
                            rdpq_sync_pipe(); // Hardware crashes otherwise
            rdpq_sync_tile(); // Hardware crashes otherwise
            }
        }
}


void crafts_close(){
    for(int c = 0; c < NUM_CRAFTS; c++){

        T3DModelIter it = t3d_model_iter_create(models[ENEMYCRAFT], T3D_CHUNK_TYPE_OBJECT);
        while(t3d_model_iter_next(&it)){
                if(it.object->material) {
                    T3DMaterialTexture *tex = &(it.object->material->textureA);
                    if(tex->texture){
                        sprite_free(tex->texture);
                    }
                }
        }

        if(crafts[c].matx) free_uncached(crafts[c].matx);
        for(int b = 0; b < MAX_PROJECTILES; b++){
            if(crafts[c].arm.rockets[b].matx) free_uncached(crafts[c].arm.rockets[b].matx);
            if(crafts[c].arm.asteroids[b].matx) free_uncached(crafts[c].arm.asteroids[b].matx);
        }
        if(crafts[c].arm.asteroiddl) rspq_block_free(crafts[c].arm.asteroiddl);
        if(crafts[c].arm.rocketdl) rspq_block_free(crafts[c].arm.rocketdl);
    }

}