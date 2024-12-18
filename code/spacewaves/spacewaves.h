#ifndef GAMEJAM2024_SPACEWAVES_H
#define GAMEJAM2024_SPACEWAVES_H 

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
#include "bonus.h"
#include "station.h"
#include "enemycraft.h"
#include "effects.h"
#include "intro.h"

    extern int main();
    extern void main_close();

    void minigame_init(){main();};
    void minigame_fixedloop(float deltatime);
    void minigame_loop(float deltatime);
    void minigame_cleanup(){main_close();};

    void main_init(){
        intro();

        display_init(
            RESOLUTION_640x480, DEPTH_16_BPP, TRIPLE_BUFFERED, GAMMA_CORRECT_DITHER, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
        
        #ifdef SPACEWAVES_STANDALONE
        // Initialize most subsystems
        asset_init_compression(2);
        asset_init_compression(3);
        dfs_init(DFS_DEFAULT_LOCATION);
        debug_init_usblog();
        debug_init_isviewer();
        joypad_init();
        timer_init();
        rdpq_init();

        audio_init(32000, 3);
        mixer_init(32);
        #endif

        register_VI_handler((void(*)())rand);

        gfx_load();
        timesys_init();

        t3d_init((T3DInitParams){});
        world_init();

        rdpq_mode_antialias(AA_STANDARD);
        rdpq_mode_dithering(dither);
        rdpq_mode_zbuf(false, false);
        rdpq_mode_filter(FILTER_BILINEAR);
        rdpq_mode_persp(true);

        viewport = t3d_viewport_create();
        station_init(PLAYER_1);
        crafts_init(PLAYER_1);
        bonus_init();
        effects_init();

        for(int i = SFX_CHANNEL_MACHINEGUN; i < SFX_CHANNEL_MUSIC; i+=2)
            mixer_ch_set_vol(i, 0.4f, 0.4f);
        for(int i = SFX_CHANNEL_MUSIC; i < 32; i+=2)
            mixer_ch_set_vol(i, 0.7f, 0.7f);

    }

    void main_close(){
        if(xmplayeropen){
            xm64player_stop(&xmplayer);
            xm64player_close(&xmplayer);
            xmplayeropen = false;
        }
        unregister_VI_handler((void(*)())rand);

        world_close();

        station_close();
        crafts_close();
        bonus_close();
        effects_close();
        gfx_close();
        
        t3d_destroy();
        display_close();
    }

#endif
