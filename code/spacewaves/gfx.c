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
#include "gfx.h"
#include <fmath.h>

color_t uicolor;

rdpq_font_t *font;
rdpq_font_t *font2;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

sprite_t *sprites[SPRITE_COUNT];

const char *texture_path[SPRITE_COUNT] = {
	"rom://spacewaves/asteroid1.ihq.sprite",
	"rom://spacewaves/explosion.rgba32.sprite",
	"rom://spacewaves/galaxy1.rgba16.sprite",
	"rom://spacewaves/galaxy2.rgba16.sprite",
	"rom://spacewaves/galaxy3.rgba16.sprite",
	"rom://spacewaves/planet1.i4.sprite",
	"rom://spacewaves/planet2_clouds.ia4.sprite",
	"rom://spacewaves/planet2.i4.sprite",
	"rom://spacewaves/spacecraft.ihq.sprite",
	"rom://spacewaves/spacecraft_engine.rgba32.sprite",
	"rom://spacewaves/stars1.i4.sprite",
	"rom://spacewaves/station.ihq.sprite",
    "rom://spacewaves/lensflare1.i8.sprite",
	"rom://spacewaves/lensflare2.i8.sprite",
	"rom://spacewaves/lensflare3.i8.sprite",
    "rom://spacewaves/bullet.rgba32.sprite",
    "rom://spacewaves/rocket.rgba16.sprite",

    "rom://spacewaves/ui.bonus0.ia8.sprite",
    "rom://spacewaves/ui.bonus1.rgba32.sprite",
    "rom://spacewaves/ui.bonus2.rgba32.sprite",
    "rom://spacewaves/ui.bonus3.rgba32.sprite",
    "rom://spacewaves/ui.bonus4.rgba32.sprite",

    "rom://spacewaves/ui.crosshair.ia4.sprite",
    "rom://spacewaves/ui.crosshair2.ia4.sprite",
    "rom://spacewaves/ui.target.ia8.sprite",

    "rom://spacewaves/exp3d.rgba32.sprite",

    "rom://spacewaves/machinegun_new_01.i4.sprite",
    "rom://spacewaves/machinegun_new_02.i4.sprite",
    "rom://spacewaves/machinegun_new_03.ci4.sprite",
    "rom://spacewaves/machinegun_new_04.ci4.sprite"
};


T3DViewport viewport;
surface_t* framebuffer;
surface_t* zbuffer;
rdpq_dither_t dither = DITHER_SQUARE_INVSQUARE;

T3DModel *models[MODEL_COUNT];

const char *model_path[MODEL_COUNT] = {
    "rom://spacewaves/space.t3dm",
    "rom://spacewaves/planet.t3dm",
	"rom://spacewaves/enemycraft.t3dm",
    "rom://spacewaves/asteroid.t3dm",
    "rom://spacewaves/rocket.t3dm",
	"rom://spacewaves/machinegun_new.t3dm",
	"rom://spacewaves/rocketgun.t3dm",
	"rom://spacewaves/station.t3dm",
    "rom://spacewaves/explosion3d.t3dm"
};

int currentmusicindex = 0;
xm64player_t xmplayer;
bool xmplayeropen = false;
T3DMat4FP* modelMatFP;

const char *music_path[MUSIC_COUNT] = {
    "rom://spacewaves/biotech.xm64",
    "rom://spacewaves/natural.xm64",
	"rom://spacewaves/chasingufo.xm64",
    "rom://spacewaves/pinkbats.xm64"
};

const char *music_credits[MUSIC_COUNT] = {
    "Biotech by Kokesz, Public Domain license",
    "Natural Vision by Kokesz, Public Domain license",
	"Chasing the UFO by JAM, Public Domain license",
    "Pink Bats in Space by JAM, Public Domain license"
};

wav64_t sounds[SOUND_COUNT];
const char *sound_path[SOUND_COUNT] = {
	"rom://spacewaves/button_click1.wav64",
    "rom://spacewaves/button_click2.wav64",
	"rom://spacewaves/button_click3.wav64",
    "rom://spacewaves/explosion-blast.wav64",
    "rom://spacewaves/hit.wav64",
	"rom://spacewaves/machinery.wav64",
	"rom://spacewaves/pickup_ammo.wav64",
	"rom://spacewaves/pickup_shield.wav64",
	"rom://spacewaves/reload.wav64",
    "rom://spacewaves/shoot.wav64",
    "rom://spacewaves/shoot_rocket.wav64",
	"rom://spacewaves/use_powerup.wav64",
    "rom://spacewaves/use_shield.wav64"
};

void gfx_load(){
    font = rdpq_font_load("rom://spacewaves/JupiteroidBoldItalic.font64");
    font2 = rdpq_font_load("rom://spacewaves/Jupiteroid.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_text_register_font(FONT_HEADING, font2);
    uicolor = RGBA32(0xBF, 0xFF, 0xBF, 0xFF);
    rdpq_font_style(font, STYLE_DEFAULT, &(rdpq_fontstyle_t){.color = uicolor});
    rdpq_font_style(font2, STYLE_DEFAULT, &(rdpq_fontstyle_t){.color = uicolor});

    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");

    for (uint32_t i = 0; i < SPRITE_COUNT; i++)
        sprites[i] = sprite_load(texture_path[i]);
    
    for (uint32_t i = 0; i < MODEL_COUNT; i++)
        models[i] = t3d_model_load(model_path[i]);

    for (uint32_t i = 0; i < SOUND_COUNT; i++)
        wav64_open(&sounds[i], sound_path[i]);

    modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
}

color_t gfx_get_playercolor(PlyNum player){
    switch(player){
        case PLAYER_1:
            return PLAYERCOLOR_1; break;
        case PLAYER_2:
            return PLAYERCOLOR_2; break;
        case PLAYER_3:
            return PLAYERCOLOR_3; break;
        case PLAYER_4:
            return PLAYERCOLOR_4; break;
    }
    return RGBA32(0xFF,0xFF,0xFF,0xFF);
}

T3DVec3 gfx_t3d_dir_from_euler(float pitch, float yaw){
    T3DVec3 vec;
    vec.v[2] = fm_cosf(yaw)*fm_cosf(pitch);
    vec.v[0] = fm_sinf(yaw)*fm_cosf(pitch);
    vec.v[1] = fm_sinf(pitch);
    return vec;
}

T3DVec3 gfx_t3d_dir_from_euler2(float pitch, float yaw){
    T3DVec3 vec;
    yaw -= T3D_DEG_TO_RAD(90.0f);
    vec.v[2] = fm_cosf(-yaw)*fm_cosf(-pitch);
    vec.v[0] = fm_sinf(-yaw)*fm_cosf(-pitch);
    vec.v[1] = fm_sinf(-pitch);
    return vec;
}

T3DVec3 gfx_worldpos_from_polar(float pitch, float yaw, float distance){
    T3DVec3 pos =  gfx_t3d_dir_from_euler(-pitch, -yaw + T3D_DEG_TO_RAD(90.0f));
    t3d_vec3_scale(&pos, &pos, distance);
    return pos;
}

void gfx_close(){
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_text_unregister_font(FONT_HEADING);
    if(font) rdpq_font_free(font);
    if(font2) rdpq_font_free(font2);

    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);

    for (uint32_t i = 0; i < SPRITE_COUNT; i++)
        sprite_free(sprites[i]);
    
    for (uint32_t i = 0; i < MODEL_COUNT; i++)
        t3d_model_free(models[i]);
    
    for (uint32_t i = 0; i < SOUND_COUNT; i++)
        wav64_close(&sounds[i]);
    
    if(modelMatFP) free_uncached(modelMatFP);

}

/* Extern inline instantiations. */
extern inline bool gfx_pos_within_viewport(float x, float y);
extern inline bool gfx_pos_within_rect(float x, float y, float xa, float ya, float xb, float yb);
extern inline float lerp(float a, float b, float t);
extern inline float fclampr(float x, float min, float max);
extern inline float fwrap(float x, float min, float max);