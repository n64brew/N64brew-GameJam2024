#include "audio.h"
#include "hydra.h"

// SFX channels for fixed audio
#define AUDIO_SFX_CHANNELS_GULP_START 0
#define AUDIO_SFX_CHANNELS_GULP_TOTAL 4
#define AUDIO_SFX_CHANNELS_AAH_START (AUDIO_SFX_CHANNELS_GULP_START + AUDIO_SFX_CHANNELS_GULP_TOTAL)
#define AUDIO_SFX_CHANNELS_AAH_TOTAL 4
#define AUDIO_SFX_CHANNELS_DRUMROLL_START (AUDIO_SFX_CHANNELS_AAH_START + AUDIO_SFX_CHANNELS_AAH_TOTAL)
#define AUDIO_SFX_CHANNELS_DRUMROLL_TOTAL 1
#define AUDIO_SFX_CHANNELS_WINNER_START (AUDIO_SFX_CHANNELS_DRUMROLL_START + AUDIO_SFX_CHANNELS_DRUMROLL_TOTAL)
#define AUDIO_SFX_CHANNELS_WINNER_TOTAL 1

// SFX channels for slide whistle and hits
#define AUDIO_SFX_CHANNELS_START (AUDIO_SFX_CHANNELS_WINNER_START + AUDIO_SFX_CHANNELS_WINNER_TOTAL)
#define AUDIO_SFX_CHANNELS_TOTAL 8
#define AUDIO_SFX_OW_COUNT 4

#define AUDIO_MUSIC_CHANNEL_START (AUDIO_SFX_CHANNELS_START + AUDIO_SFX_CHANNELS_TOTAL)

static const uint32_t channel_pitch[STATE_COUNT] = {
	28000, 22000, 18000,
};

static wav64_t audio_sfx[SFX_COUNT];
static wav64_t gulp_sfx[PLAYER_MAX];
static wav64_t aah_sfx[PLAYER_MAX];
static wav64_t eww_sfx[PLAYER_MAX];
static wav64_t ow_sfx[AUDIO_SFX_OW_COUNT];
static xm64player_t hh_xm;

static uint8_t channel = 0;

void audio_sfx_init(void) {
	char temptext[64];
	wav64_open(&audio_sfx[SFX_SLIDE_WHISTLE_DOWN], "rom:/hydraharmonics/slide-whistle-down.wav64");
	wav64_open(&audio_sfx[SFX_SLIDE_WHISTLE_UP], "rom:/hydraharmonics/slide-whistle-up.wav64");
	wav64_open(&audio_sfx[SFX_DRUMROLL], "rom:/hydraharmonics/drumroll.wav64");
	wav64_set_loop(&audio_sfx[SFX_DRUMROLL], true);
	wav64_open(&audio_sfx[SFX_WINNER], "rom:/core/Winner.wav64");
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		wav64_open(&gulp_sfx[i], "rom:/hydraharmonics/gulp.wav64");
		wav64_open(&aah_sfx[i], "rom:/hydraharmonics/aah.wav64");
		wav64_open(&eww_sfx[i], "rom:/hydraharmonics/eww.wav64");
	}
	for (uint8_t i=0; i<AUDIO_SFX_OW_COUNT; i++) {
		sprintf(temptext, "rom:/hydraharmonics/ow.wav64");
		wav64_open(&ow_sfx[i], temptext);
	}
	xm64player_open(&hh_xm, "rom:/hydraharmonics/hydraharmonics.xm64");
	xm64player_set_loop(&hh_xm, false);
}

uint8_t audio_sfx_get_last_channel (void) {
	return (channel + AUDIO_SFX_CHANNELS_TOTAL - 1) % AUDIO_SFX_CHANNELS_TOTAL;
}

void audio_sfx_play (audio_sfx_t sound) {
	static uint8_t ow_counter = 0;
	static uint8_t last_whistle_down = 0;

	// Gulp SFX
	if (sound >= SFX_GULP_MIN && sound <= SFX_GULP_MAX) {
		wav64_play(&gulp_sfx[sound - SFX_GULP_MIN], AUDIO_SFX_CHANNELS_GULP_START + sound - SFX_GULP_MIN);
		mixer_ch_set_freq(
			AUDIO_SFX_CHANNELS_GULP_START + sound - SFX_GULP_MIN,
			channel_pitch[hydras[sound - SFX_GULP_MIN].state]
		);
	// Aah SFX
	} else if (sound >= SFX_AAH_MIN && sound <= SFX_AAH_MAX) {
		wav64_play(&aah_sfx[sound - SFX_AAH_MIN], AUDIO_SFX_CHANNELS_AAH_START + sound - SFX_AAH_MIN);
		mixer_ch_set_freq(
			AUDIO_SFX_CHANNELS_AAH_START + sound - SFX_AAH_MIN,
			channel_pitch[hydras[sound - SFX_AAH_MIN].state]
		);
	// Eww SFX
	}  else if (sound >= SFX_EWW_MIN && sound <= SFX_EWW_MAX) {
		mixer_ch_stop(AUDIO_SFX_CHANNELS_GULP_START + sound - SFX_EWW_MIN);
		wav64_play(&eww_sfx[sound - SFX_EWW_MIN], AUDIO_SFX_CHANNELS_GULP_START + sound - SFX_EWW_MIN);
	// Ow SFX
	} else if (sound == SFX_OW) {
		wav64_play(&ow_sfx[ow_counter], AUDIO_SFX_CHANNELS_START + channel);
		ow_counter = (ow_counter + 1) % AUDIO_SFX_OW_COUNT;
	// Drumroll
	} else if (sound == SFX_DRUMROLL) {
		wav64_play(&audio_sfx[sound], AUDIO_SFX_CHANNELS_DRUMROLL_START);
	// Winner
	} else if (sound == SFX_WINNER) {
		mixer_ch_stop(AUDIO_SFX_CHANNELS_DRUMROLL_START);
		wav64_play(&audio_sfx[sound], AUDIO_SFX_CHANNELS_WINNER_START);
	// Other SFX (Whistle up/down)
	} else {
		if (sound == SFX_SLIDE_WHISTLE_DOWN) {
			mixer_ch_stop(last_whistle_down);
			last_whistle_down = AUDIO_SFX_CHANNELS_START + channel;
		}
		wav64_play(&audio_sfx[sound], AUDIO_SFX_CHANNELS_START + channel);
	}
	channel = (channel + 1) % AUDIO_SFX_CHANNELS_TOTAL;
}

void audio_music_play (void) {
	xm64player_play(&hh_xm, AUDIO_MUSIC_CHANNEL_START);
}

void audio_music_stop (void) {
	xm64player_stop(&hh_xm);
}

int audio_music_get_pattern (void) {
	int pattern;
	xm64player_tell(&hh_xm, &pattern, NULL, NULL);
	return pattern;
}

void audio_music_monitor (void) {
	int pattern;
	xm64player_tell(&hh_xm, &pattern, NULL, NULL);
	if (pattern == 9) {
		game_speed = NOTE_SPEED_MID;
	} else if (pattern == 10) {
		game_speed = NOTE_SPEED_FAST;
	}
}

void audio_sfx_close (void) {
	wav64_close(&audio_sfx[SFX_SLIDE_WHISTLE_DOWN]);
	wav64_close(&audio_sfx[SFX_SLIDE_WHISTLE_UP]);
	wav64_close(&audio_sfx[SFX_DRUMROLL]);
	wav64_close(&audio_sfx[SFX_WINNER]);
	wav64_close(&audio_sfx[SFX_AAH_MIN]);
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		wav64_close(&gulp_sfx[i]);
		wav64_close(&aah_sfx[i]);
		wav64_close(&eww_sfx[i]);
	}
	for (uint8_t i=0; i<AUDIO_SFX_OW_COUNT; i++) {
		wav64_close(&ow_sfx[i]);
	}
	xm64player_close(&hh_xm);
}
