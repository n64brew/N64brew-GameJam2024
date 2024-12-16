#ifndef HYDRA_AUDIO_H
#define HYDRA_AUDIO_H

#include "hydraharmonics.h"

typedef enum {
	SFX_SLIDE_WHISTLE_DOWN, // By freesound_community
	SFX_SLIDE_WHISTLE_UP, // By freesound_community
	SFX_GULP_0,
	SFX_GULP_1,
	SFX_GULP_2,
	SFX_GULP_3,
	SFX_AAH_0,
	SFX_AAH_1,
	SFX_AAH_2,
	SFX_AAH_3,
	SFX_EWW_0,
	SFX_EWW_1,
	SFX_EWW_2,
	SFX_EWW_3,
	SFX_DRUMROLL, // By leenn792
	SFX_WINNER, // Core asset
	SFX_OW, // By freesound_community
	SFX_COUNT,
	SFX_GULP_MIN = SFX_GULP_0,
	SFX_GULP_MAX = SFX_GULP_3,
	SFX_AAH_MIN = SFX_AAH_0,
	SFX_AAH_MAX = SFX_AAH_3,
	SFX_EWW_MIN = SFX_EWW_0,
	SFX_EWW_MAX = SFX_EWW_3,
} audio_sfx_t;

void audio_sfx_init(void);
uint8_t audio_sfx_get_last_channel (void);
void audio_sfx_play (audio_sfx_t sound);
void audio_music_play (void);
void audio_music_stop (void);
int audio_music_get_pattern (void);
void audio_music_monitor (void);
void audio_sfx_close(void);

#endif
