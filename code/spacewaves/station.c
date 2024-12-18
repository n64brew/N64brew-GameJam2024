#include <unistd.h>
#include <time.h>
#include <libdragon.h>
#include <display.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "station.h"
#include "types.h"
#include "gfx.h"
#include "enemycraft.h"
#include "gamestatus.h"
#include "effects.h"
#include "bonus.h"

DefenseStation station;

void station_init(PlyNum player){
    station.currentplayer = player;
    station.currentplayerport = core_get_playercontroller(player);
    station.color = gfx_get_playercolor(player);
    station.matx = malloc_uncached(sizeof(T3DMat4FP));
    station.arm.rocketgunmatx = malloc_uncached(sizeof(T3DMat4FP));
    station.arm.machinegunmatx = malloc_uncached(sizeof(T3DMat4FP));
    station.arm.rocketcount = 0;
    for(int p = 0; p < MAX_PROJECTILES; p++) 
        station.arm.rockets[p].matx = malloc_uncached(sizeof(T3DMat4FP));
    station.maxhp = 500;
    station.hp =  station.maxhp;
    station.arm.powerup = 0;
    station.pitch = 0;
    station.yaw = 0;
    station.arm.bulletnexttime = CURRENT_TIME + 0.5f;
    wav64_set_loop(&sounds[snd_machinery], true);
    wav64_play(&sounds[snd_machinery], SFX_CHANNEL_STATION);
    mixer_ch_set_vol(SFX_CHANNEL_STATION, 0.0f, 0.0f);
}

void station_close(){
    if(station.matx) free_uncached(station.matx);
    if(station.arm.rocketgunmatx) free_uncached(station.arm.rocketgunmatx);
    if(station.arm.machinegunmatx) free_uncached(station.arm.machinegunmatx);
    for(int p = 0; p < MAX_PROJECTILES; p++) {
        if(station.arm.rockets[p].matx) free_uncached(station.arm.rockets[p].matx);
        station.arm.rockets[p].enabled = false;
        station.arm.bullets[p].enabled = false;
    }
    if(station.arm.rocketdl) rspq_block_free(station.arm.rocketdl);
}

void station_update(){
    joypad_buttons_t pressed = joypad_get_buttons_pressed(station.currentplayerport);
    joypad_inputs_t input = joypad_get_inputs(station.currentplayerport);
    station.yaw     += T3D_DEG_TO_RAD(0.28f * DELTA_TIME * input.stick_x);
    station.pitch   -= T3D_DEG_TO_RAD(0.28f * DELTA_TIME * input.stick_y);
    station.pitch = fclampr(station.pitch, T3D_DEG_TO_RAD(-89), T3D_DEG_TO_RAD(89));
    station.pitchoff = t3d_lerp(station.pitchoff, station.pitch, 0.1f);
    station.yawoff =   t3d_lerp(station.yawoff, station.yaw, 0.1f);
    station.pitchcam = t3d_lerp(station.pitchoff, station.pitch, 0.4f);
    station.yawcam =   t3d_lerp(station.yawoff, station.yaw, 0.4f);
    if(abs(input.stick_x) + abs(input.stick_y) > 30){
        mixer_ch_set_vol(SFX_CHANNEL_STATION, 1.0f, 1.0f);
    } else {
        mixer_ch_set_vol(SFX_CHANNEL_STATION, 0.0f, 0.0f);
    }
    t3d_mat4fp_from_srt_euler(station.matx, 
        (float[3]){1.0f, 1.0f, 1.0f},
        (float[3]){0.0f, station.yawoff, station.pitchoff},
        (float[3]){0,0,0});
    world.currcamangles.v[0] = station.yawcam;
    world.currcamangles.v[1] = station.pitchcam;

    joypad_buttons_t held = joypad_get_buttons_held(station.currentplayerport);

    if(held.z && CURRENT_TIME >= station.arm.bulletnexttime && !gamestatus.paused){
        int b = 0; while(station.arm.bullets[b].enabled && b < MAX_PROJECTILES - 1) b++;
        station.arm.bullets[b].enabled = true;
        station.arm.bullets[b].polarpos = (T3DVec3){{station.pitch, station.yaw, 0.25f}};
        station.arm.bulletnexttime = CURRENT_TIME + 0.35f;
        if(station.arm.powerup > 0.0f && station.arm.powerup < 10.0f) 
            station.arm.bulletnexttime = CURRENT_TIME + 0.2f;
        station.arm.machinegunoffset = -150;
        wav64_play(&sounds[snd_shoot], SFX_CHANNEL_MACHINEGUN);
        effects_add_rumble(station.currentplayerport, 0.1f);
        effects_add_ambientlight(RGBA32(0,25,50,0));
    }

    if((held.l || held.r) && CURRENT_TIME >= station.arm.rocketnexttime && station.arm.rocketcount > 0){
        int b = 0; while(station.arm.rockets[b].enabled && b < MAX_PROJECTILES - 1) b++;
        station.arm.rockets[b].enabled = true;
        station.arm.rockets[b].polarpos = (T3DVec3){{station.pitchoff, station.yawoff, 0.25f}};
        station.arm.rocketnexttime = CURRENT_TIME + 2.5f;
        if(station.arm.powerup > 0.0f && station.arm.powerup < 10.0f) 
            station.arm.rocketnexttime = CURRENT_TIME + 1.0f;
        station.arm.rocketcount--;
        station.arm.rocketgunoffset = -150;
        wav64_play(&sounds[snd_shoot_rocket], SFX_CHANNEL_ROCKET);
        effects_add_rumble(station.currentplayerport, 0.25f);
        effects_add_ambientlight(RGBA32(50,50,25,0));
    }

    station.arm.machinegunoffset = lerp(station.arm.machinegunoffset, 0, 0.1f);

    if(station.arm.rocketcount) station.arm.rocketgunoffset  = lerp(station.arm.rocketgunoffset,  0, 0.1f);
    else                        station.arm.rocketgunoffset  = lerp(station.arm.rocketgunoffset,  -250, 0.1f);

    if((pressed.a && station.arm.shield == 10.0f)){
        station.arm.shield -= DELTA_TIME;
        wav64_play(&sounds[snd_use_shield], SFX_CHANNEL_BONUS);
        effects_add_ambientlight(RGBA32(0,0,100,0));
    }
    if(station.arm.shield < 10.0f)
            station.arm.shield -= DELTA_TIME;

    station.arm.shield = fclampr(station.arm.shield, 0.0f, 10.0f);

    if((pressed.b && station.arm.powerup == 10.0f)){
        station.arm.powerup -= DELTA_TIME;
        wav64_play(&sounds[snd_use_powerup], SFX_CHANNEL_BONUS);
        effects_add_ambientlight(RGBA32(0,100,0,0));
    }
    if(station.arm.powerup < 10.0f)
        station.arm.powerup -= DELTA_TIME;
    
    station.arm.powerup = fclampr(station.arm.powerup, 0.0f, 10.0f);

    T3DVec3 bullet_worldpos = (T3DVec3){{0,0,0}};
    T3DVec3 rocket_worldpos = (T3DVec3){{0,0,0}};
    for(int b = 0; b < MAX_PROJECTILES; b++){
        if(station.arm.bullets[b].enabled){
            station.arm.bullets[b].polarpos.v[2] += DELTA_TIME * 140.0f;
            if(station.arm.bullets[b].polarpos.v[2] > 200.0f)
                station.arm.bullets[b].enabled = false;

            bullet_worldpos = gfx_worldpos_from_polar(
                station.arm.bullets[b].polarpos.v[0], 
                station.arm.bullets[b].polarpos.v[1], 
                station.arm.bullets[b].polarpos.v[2]);
        }
        if(station.arm.rockets[b].enabled){
            station.arm.rockets[b].polarpos.v[2] += DELTA_TIME * 40.0f;
            if(station.arm.rockets[b].polarpos.v[2] > 200.0f)
                station.arm.rockets[b].enabled = false;

            rocket_worldpos = gfx_worldpos_from_polar(
                station.arm.rockets[b].polarpos.v[0], 
                station.arm.rockets[b].polarpos.v[1], 
                station.arm.rockets[b].polarpos.v[2]);
        }

        for(int c = 0; c < 3; c++){
            T3DVec3 crft_worldpos;
            if(crafts[c].enabled){
                crft_worldpos = gfx_worldpos_from_polar(
                    crafts[c].pitchoff,
                    crafts[c].yawoff,
                    crafts[c].distanceoff);
                if(station.arm.bullets[b].enabled && t3d_vec3_distance(&crft_worldpos, &bullet_worldpos) < 5.0f){
                    if(!(crafts[c].arm.shield > 0.0f && crafts[c].arm.shield < 10.0f)){
                        crafts[c].hp -= 10;
                        gamestatus.playerscores[station.currentplayer] += 10 * 50;
                    }
                    station.arm.bullets[b].enabled = false;
                    wav64_play(&sounds[snd_hit], SFX_CHANNEL_HIT);
                    effects_add_exp2d(gfx_worldpos_from_polar(
                            station.arm.bullets[b].polarpos.v[0],
                            station.arm.bullets[b].polarpos.v[1],
                            station.arm.bullets[b].polarpos.v[2] * 25), 
                            RGBA32(150,150,255,255));
                }
                if(station.arm.rockets[b].enabled && t3d_vec3_distance(&crft_worldpos, &rocket_worldpos) < 8.0f){
                    if(!(crafts[c].arm.shield > 0.0f && crafts[c].arm.shield < 10.0f)){
                        crafts[c].hp -= 100;
                        gamestatus.playerscores[station.currentplayer] += 8000;
                    }
                    station.arm.rockets[b].enabled = false;
                    wav64_play(&sounds[snd_hit], SFX_CHANNEL_HIT);
                    effects_add_ambientlight(RGBA32(50,50,25,0));
                }
            }
        }
        for(int c = 0; c < 3; c++)
        for(int p = 0; p < MAX_PROJECTILES; p++){
            T3DVec3 ast_worldpos, enrocket_worldpos;
            if(crafts[c].arm.asteroids[p].enabled){
                ast_worldpos = gfx_worldpos_from_polar(
                    crafts[c].arm.asteroids[p].polarpos.v[0],
                    crafts[c].arm.asteroids[p].polarpos.v[1],
                    crafts[c].arm.asteroids[p].polarpos.v[2]);
                if(station.arm.bullets[b].enabled && t3d_vec3_distance(&ast_worldpos, &bullet_worldpos) < 4.0f){
                    crafts[c].arm.asteroids[p].hp -= 10;
                    gamestatus.playerscores[station.currentplayer] += 10 * 20;
                    station.arm.bullets[b].enabled = false;
                    wav64_play(&sounds[snd_hit], SFX_CHANNEL_EFFECTS);
                    effects_add_exp2d(gfx_worldpos_from_polar(
                            station.arm.bullets[b].polarpos.v[0],
                            station.arm.bullets[b].polarpos.v[1],
                            station.arm.bullets[b].polarpos.v[2] * 25), 
                            RGBA32(150,150,255,255));
                }
                if(station.arm.rockets[b].enabled && t3d_vec3_distance(&ast_worldpos, &rocket_worldpos) < 6.0f){
                    crafts[c].arm.asteroids[p].hp -= 100;
                    gamestatus.playerscores[station.currentplayer] += 1000;
                    station.arm.rockets[b].enabled = false;
                    wav64_play(&sounds[snd_hit], SFX_CHANNEL_EFFECTS);
                    effects_add_rumble(crafts[c].currentplayerport, 1.25f);
                    effects_add_shake(1.25f);
                }
            }
            if(crafts[c].arm.rockets[p].enabled){
                enrocket_worldpos = gfx_worldpos_from_polar(
                    crafts[c].arm.rockets[p].polarpos.v[0],
                    crafts[c].arm.rockets[p].polarpos.v[1],
                    crafts[c].arm.rockets[p].polarpos.v[2]);
                if(station.arm.bullets[b].enabled && t3d_vec3_distance(&enrocket_worldpos, &bullet_worldpos) < 5.0f){
                    crafts[c].arm.rockets[p].hp -= 10;
                    gamestatus.playerscores[station.currentplayer] += 10 * 50;
                    station.arm.bullets[b].enabled = false;
                    wav64_play(&sounds[snd_hit], SFX_CHANNEL_EFFECTS);
                    effects_add_rumble(crafts[c].currentplayerport, 0.25f);
                }
            }
        }
    }
    for(int bonus = 0; bonus < MAX_BONUSES; bonus++)
        for(int p = 0; p < MAX_PROJECTILES; p++){
            bullet_worldpos = gfx_worldpos_from_polar(
                station.arm.bullets[p].polarpos.v[0], 
                station.arm.bullets[p].polarpos.v[1], 
                station.arm.bullets[p].polarpos.v[2]);
            rocket_worldpos = gfx_worldpos_from_polar(
                station.arm.rockets[p].polarpos.v[0], 
                station.arm.rockets[p].polarpos.v[1], 
                station.arm.rockets[p].polarpos.v[2]);
            T3DVec3 ast_worldpos;
            if(bonuses[bonus].enabled){
                ast_worldpos = gfx_worldpos_from_polar(
                    bonuses[bonus].polarpos.v[0],
                    bonuses[bonus].polarpos.v[1],
                    bonuses[bonus].polarpos.v[2]);
                if(station.arm.bullets[p].enabled && t3d_vec3_distance(&ast_worldpos, &bullet_worldpos) < 8.0f){
                    bonus_apply(bonus, station.currentplayer, &station, -1);
                    station.arm.bullets[p].enabled = false;
                }
                if(station.arm.rockets[p].enabled && t3d_vec3_distance(&ast_worldpos, &rocket_worldpos) < 10.0f){
                    bonus_apply(bonus, station.currentplayer, &station, -1);
                    station.arm.rockets[p].enabled = false; 
                }
            }
        }
}

void station_apply_camera(){
    T3DVec3 camPos =  gfx_t3d_dir_from_euler2(station.pitchcam, station.yawcam);
    station.forward = camPos;
    t3d_vec3_scale(&camPos, &camPos, -1.0f);
    const T3DVec3 camTarget = {{0,0,0}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
    t3d_viewport_attach(&viewport);
}

void station_draw(){
    rdpq_set_mode_standard();
    rdpq_mode_dithering(dither);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_zbuf(false, false);
    rdpq_mode_persp(true);
    rdpq_mode_mipmap(MIPMAP_NONE,0);

    color_t amb = RGBA32(0xA0, 0xA0,0xA0,0xFF);
    t3d_light_set_ambient((uint8_t*)&amb);
    T3DMaterial* mat =  t3d_model_get_material(models[ROCKET], "f3d.rocket");
    T3DObject* obj = t3d_model_get_object_by_index(models[ROCKET], 0);
    t3d_model_draw_material(mat, NULL);
    for(int b = 0; b < MAX_PROJECTILES; b++){
        if(station.arm.rockets[b].enabled){
            T3DVec3 worldpos = gfx_worldpos_from_polar(
                station.arm.rockets[b].polarpos.v[0], 
                station.arm.rockets[b].polarpos.v[1], 
                station.arm.rockets[b].polarpos.v[2] * 25.0f);
            worldpos.v[1] += 100.0f;
            T3DMat4 rocketmat;
            t3d_mat4_from_srt_euler(&rocketmat,
            (float[3]){1.0f, 1.0f, 1.0f},
            (float[3]){0.0f, station.arm.rockets[b].polarpos.v[1], station.arm.rockets[b].polarpos.v[0]},
            (float[3]){worldpos.v[0],worldpos.v[1],worldpos.v[2]});
            t3d_mat4_to_fixed(station.arm.rockets[b].matx, &rocketmat);
            t3d_matrix_push(station.arm.rockets[b].matx);
            rdpq_mode_antialias(AA_STANDARD);
            if(!station.arm.rocketdl){
                rspq_block_begin();
                rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_model_draw_object(obj, NULL);
                rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                station.arm.rocketdl = rspq_block_end();
            } else rspq_block_run(station.arm.rocketdl);
            t3d_matrix_pop(1);
        }
    }
    t3d_light_set_ambient((uint8_t*)&world.sun.ambient);

    rdpq_set_mode_standard();
    rdpq_mode_dithering(dither);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_zbuf(false, false);
    rdpq_mode_mipmap(MIPMAP_NONE,0);

    surface_t bullet = sprite_get_pixels(sprites[15]);
    rdpq_tex_upload(TILE0, &bullet, &(rdpq_texparms_t){.s.mirror = true, .s.repeats = 2});
    for(int b = 0; b < MAX_PROJECTILES; b++){
        if(station.arm.bullets[b].enabled){
            T3DVec3 worldpos = gfx_worldpos_from_polar(
                station.arm.bullets[b].polarpos.v[0], 
                station.arm.bullets[b].polarpos.v[1], 
                station.arm.bullets[b].polarpos.v[2]);
            worldpos.v[1] -= 0.5f;
            T3DVec3 viewpos;
            t3d_viewport_calc_viewspace_pos(&viewport, &viewpos, &worldpos);
            float xpos = viewpos.v[0];
            float ypos = viewpos.v[1];
            float zpos = station.arm.bullets[b].polarpos.v[2] * 0.3f;
            float width = sprites[15]->width * 2;
            float height = sprites[15]->height;
            float s = width;
            float t = height;
            width *= (4.0f / zpos);
            height *= (4.0f / zpos);

            if(gfx_pos_within_viewport(xpos, ypos)){
                rdpq_texture_rectangle_scaled(TILE0, xpos - width, ypos - height, xpos + width, ypos + height, 0, 0, s, t);
            }
        }
    }
    t3d_matrix_push(station.matx);

    for(int i = MACHINEGUN; i <= STATION; i++){
        T3DModelIter it = t3d_model_iter_create(models[i], T3D_CHUNK_TYPE_OBJECT);
        t3d_light_set_directional(0, (uint8_t*)&world.sun.color, &world.sun.direction);
        if(station.arm.shield > 0.0f && station.arm.shield < 10.0f) rdpq_set_env_color(RGBA32(0x80,0x80,0xB0,0xFF));
        else rdpq_set_env_color(RGBA32(0x00,0x00,0x00,0xFF));
        while(t3d_model_iter_next(&it))
        {
            rdpq_set_mode_standard();
            rdpq_mode_dithering(dither);
            rdpq_mode_filter(FILTER_BILINEAR);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_mode_zbuf(false, false);
            rdpq_mode_mipmap(MIPMAP_NONE,0);
            rdpq_mode_persp(true);
            rdpq_sync_pipe(); // Hardware crashes otherwise
            rdpq_sync_tile(); // Hardware crashes otherwise
            if(i == ROCKETGUN){
                t3d_mat4fp_from_srt_euler(station.arm.rocketgunmatx, 
                (float[3]){1.0f, 1.0f, 1.0f},
                (float[3]){0.0f, 0.0f, 0.0f},
                (float[3]){station.arm.rocketgunoffset,0,0});
                t3d_matrix_push(station.arm.rocketgunmatx);
                t3d_model_draw_material(it.object->material, NULL);
                rdpq_sprite_upload(TILE0, sprites[spr_spacecraft], &(rdpq_texparms_t){.s.mirror = true, .s.repeats = REPEAT_INFINITE, .t.mirror = true, .t.repeats = REPEAT_INFINITE} );
                rdpq_mode_zbuf(false, false);
                rdpq_mode_antialias(AA_STANDARD);
                rdpq_sync_pipe(); // Hardware crashes otherwise
                rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_model_draw_object(it.object, NULL);
                rdpq_sync_pipe(); // Hardware crashes otherwise
                rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_matrix_pop(1);}
            else if(i == MACHINEGUN){
                t3d_mat4fp_from_srt_euler(station.arm.machinegunmatx, 
                (float[3]){1.0f, 1.0f, 1.0f},
                (float[3]){0.0f, 0.0f, 0.0f},
                (float[3]){station.arm.machinegunoffset,0,0});
                t3d_matrix_push(station.arm.machinegunmatx);
                t3d_model_draw_material(it.object->material, NULL);
                rdpq_set_prim_color(station.color);
                rdpq_mode_zbuf(false, false);
                rdpq_mode_antialias(AA_STANDARD);
                rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_model_draw_object(it.object, NULL);
                rdpq_sync_pipe(); // Hardware crashes otherwise
                rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_matrix_pop(1); } 
            else {
                t3d_model_draw_material(it.object->material, NULL);
                rdpq_sprite_upload(TILE0, sprites[spr_station], &(rdpq_texparms_t){.s.scale_log = 1, .t.scale_log = 1, .s.mirror = true, .s.repeats = REPEAT_INFINITE, .t.mirror = true, .t.repeats = REPEAT_INFINITE} );
                rdpq_mode_zbuf(false, false);
                rdpq_mode_antialias(AA_STANDARD);
                rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_model_draw_object(it.object, NULL);
                rdpq_sync_pipe(); // Hardware crashes otherwise
                rdpq_sync_tile(); // Hardware crashes otherwise
            }
        }
    }
    
    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise
    t3d_matrix_pop(1);
}
