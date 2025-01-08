#ifndef GFX_H
#define GFX_H

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

#define FONT_TEXT           1
#define FONT_HEADING        2
#define STYLE_DEFAULT       0
#define SPRITE_COUNT	    30
#define MODEL_COUNT         9
#define MUSIC_COUNT         4
#define SOUND_COUNT         13

#define SFX_CHANNEL_STATION 0
#define SFX_CHANNEL_MACHINEGUN 2
#define SFX_CHANNEL_ROCKET 4
#define SFX_CHANNEL_EFFECTS 6
#define SFX_CHANNEL_EXPLOSION 8
#define SFX_CHANNEL_HIT 10
#define SFX_CHANNEL_BONUS 12
#define SFX_CHANNEL_MUSIC   14

extern rdpq_font_t *font;

extern wav64_t sfx_start;
extern wav64_t sfx_countdown;
extern wav64_t sfx_stop;
extern wav64_t sfx_winner;

extern sprite_t *sprites[SPRITE_COUNT];
extern const char *texture_path[SPRITE_COUNT];

extern T3DModel *models[MODEL_COUNT];
extern const char *model_path[MODEL_COUNT];

extern wav64_t sounds[SOUND_COUNT];
extern const char *sound_path[SOUND_COUNT];
extern int sound_cur_channels[SOUND_COUNT];

extern xm64player_t xmplayer;
extern bool xmplayeropen;
extern int currentmusicindex;

extern const char *music_path[MUSIC_COUNT];
extern const char *music_credits[MUSIC_COUNT];

enum spritenames_t{
    spr_asteroid1,
    spr_exp,
    spr_galaxy1,
    spr_galaxy2,
    spr_galaxy3,
	spr_planet1,
	spr_planet2_clouds,
	spr_planet2,
	spr_spacecraft,
	spr_spacecraft_engine,
	spr_stars1,
	spr_station,
    spr_lensflare1,
	spr_lensflare2,
	spr_lensflare3,
    spr_bullet,
    spr_rocket,
    spr_ui_bonus0,
	spr_ui_bonus1,
    spr_ui_bonus2,
	spr_ui_bonus3,
    spr_ui_bonus4,
	spr_ui_crosshair,
    spr_ui_crosshair2,
    spr_ui_target,
    spr_explosion
};

enum modelnames_t{
    SPACE,
    PLANET,
    ENEMYCRAFT,
    ASTEROID,
    ROCKET,
    MACHINEGUN,
    ROCKETGUN,
    STATION,
    EXP3D
};

enum soundnames_t{
    snd_button_click1,
    snd_button_click2,
    snd_button_click3,
    snd_explosion_blast,
    snd_hit,
    snd_machinery,
    snd_pickup_ammo,
    snd_pickup_shield,
    snd_reload,
    snd_shoot,
    snd_shoot_rocket,
    snd_use_powerup,
    snd_use_shield,
};

extern color_t uicolor;

extern T3DViewport viewport;
extern T3DMat4FP* modelMatFP;

extern surface_t* framebuffer;
extern surface_t* zbuffer;
extern rdpq_dither_t dither;

void gfx_load();
void gfx_close();

extern color_t gfx_get_playercolor(PlyNum player);

extern T3DVec3 gfx_t3d_dir_from_euler(float pitch, float yaw);
extern T3DVec3 gfx_t3d_dir_from_euler2(float pitch, float yaw);
extern T3DVec3 gfx_worldpos_from_polar(float pitch, float yaw, float distance);

inline bool gfx_pos_within_viewport(float x, float y){
    return x > 0 && x < display_get_width() && y > 0 && y < display_get_height();
}

inline bool gfx_pos_within_rect(float x, float y, float xa, float ya, float xb, float yb){
    return x > xa && x < xb && y > ya && y < yb;
}

inline float lerp(float a, float b, float t){
    return (b - a * t) + a;
}

inline float fclampr(float x, float min, float max){
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

inline float fwrap(float x, float min, float max) {
    if (min > max) {
        return fwrap(x, max, min);
    }
    return (x >= 0 ? min : max) + fmod(x, max - min);
}

void afx_wav64_play_wrapper(int soundindex, int ch);

#endif
