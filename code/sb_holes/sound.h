#ifndef SOUND_H
#define SOUND_H

#include <libdragon.h>

// Core Definitions
#define MUSIC_CHANNEL 0
#define SFX_CHANNEL 31

// XM sequences
enum BG_XM
{
    XM_TMA,
    NUM_XM
};

xm64player_t xmPlayer;

const char *xmFileNames[NUM_XM] = {
    "rom:/sb_holes/TheMorningAfter.xm64", // https://github.com/DragonMinded/libdragon/blob/trunk/examples/audioplayer/assets/TheMorningAfter.xm
};

// WAV files
enum SFX_WAV
{
    SFX_BUILDING,
    SFX_CAR,
    SFX_HYDRANT,
    SFX_START,
    SFX_COUNTDOWN,
    SFX_STOP,
    SFX_WINNER,
    NUM_WAV
};

// Each WAV must have its own structure
wav64_t soundEffects[NUM_WAV];

const char *wavFileNames[NUM_WAV] = {
    "rom:/strawberry_byte/sound/stones-falling.wav64",
    "rom:/sb_holes/car.wav64",
    "rom:/sb_holes/hydrant.wav64",
    "rom:/core/Start.wav64",
    "rom:/core/Countdown.wav64",
    "rom:/core/Stop.wav64",
    "rom:/core/Winner.wav64",
};

/* Function Declarations */
void sound_load(bool playXM);
void sound_xmSwitch(int songID, float volume, bool loop);
void sound_xmStop(void);
void sound_xmUpdate(float volume, bool loop);
void sound_wavPlay(int sfxID, bool loop);
void sound_wavClose(int sfxID);
void sound_wavCleanup(void);
void sound_cleanup(void);
void sound_update(void);
float sound_reverb(float volume, float mix);
void sound_setChannels(void);

/* Function Definitions */

void sound_load(bool playXM)
{
    // Open all WAVs at boot
    for (int w = 0; w < NUM_WAV; ++w)
        wav64_open(&soundEffects[w], wavFileNames[w]);

    // Open and play first XM in the list
    if (playXM)
    {
        xm64player_open(&xmPlayer, xmFileNames[0]);
        xm64player_set_vol(&xmPlayer, 0.5f);
        xm64player_play(&xmPlayer, MUSIC_CHANNEL);
    }
}

// Stops current XM, opens and plays requested module with set volume and whether to loop
void sound_xmSwitch(int songID, float volume, bool loop)
{
    xm64player_stop(&xmPlayer);
    xm64player_close(&xmPlayer);
    xm64player_open(&xmPlayer, xmFileNames[songID]);
    xm64player_set_loop(&xmPlayer, loop);
    xm64player_set_vol(&xmPlayer, volume);
    xm64player_play(&xmPlayer, MUSIC_CHANNEL);
}

// Stops and closes XM player
void sound_xmStop(void)
{
    xm64player_stop(&xmPlayer);
}

// Adjusts volume and looping of current XM module
void sound_xmUpdate(float volume, bool loop)
{
    xm64player_set_loop(&xmPlayer, loop);
    xm64player_set_vol(&xmPlayer, volume);
}

// Plays requested WAV and whether to loop
void sound_wavPlay(int sfxID, bool loop)
{
    wav64_set_loop(&soundEffects[sfxID], loop);
    wav64_play(&soundEffects[sfxID], SFX_CHANNEL - sfxID);
}

void sound_wavClose(int sfxID)
{
    wav64_close(&soundEffects[sfxID]);
}

void sound_wavCleanup(void)
{
    for (int w = 0; w < NUM_WAV; ++w)
        wav64_close(&soundEffects[w]);
}

void sound_cleanup(void)
{
    if (core_get_playercount() != 4)
        xm64player_close(&xmPlayer);
    sound_wavCleanup();
}

void sound_update(void)
{
    mixer_try_play();
}

////// Audio filters

#define REVERB_BUFFER_SIZE 32000 // Size for the delay buffer
#define MAX_COMB_FILTERS 3
#define MAX_ALLPASS_FILTERS 2

typedef struct
{
    float comb_delays[MAX_COMB_FILTERS];
    float comb_feedback;
    float allpass_delays[MAX_ALLPASS_FILTERS];
    float allpass_feedback;
    float sample_rate;
} ReverbParams;

// Schroeder Reverberator Parameters
ReverbParams paramsSchroeder = {
    {2.0f, 3.0f, 4.0f}, // Comb filter delays in frames
    0.5f,
    {1.0f, 2.0f}, // All-pass filter delays in frames
    0.4f,
    16000.0f,
};

// Circular buffers for the comb and all-pass filters
float comb_delay_buffer[REVERB_BUFFER_SIZE];
int comb_buffer_index = 0;

float allpass_delay_buffer[REVERB_BUFFER_SIZE];
int allpass_buffer_index = 0;

// Comb Filter implementation
float comb_filter(float input, float delay_seconds, float feedback, float sample_rate)
{
    int delay_samples = (int)(delay_seconds * sample_rate);
    int buffer_index = (comb_buffer_index + REVERB_BUFFER_SIZE - delay_samples) % REVERB_BUFFER_SIZE;

    float delayed_sample = comb_delay_buffer[buffer_index];
    float output = delayed_sample + input;

    // Store the output with feedback in the buffer
    comb_delay_buffer[comb_buffer_index] = input + delayed_sample * feedback;
    comb_buffer_index = (comb_buffer_index + 1) % REVERB_BUFFER_SIZE;

    return output;
}

// All-Pass Filter implementation
float allpass_filter(float input, float delay_seconds, float feedback, float sample_rate)
{
    int delay_samples = (int)(delay_seconds * sample_rate);
    int buffer_index = (allpass_buffer_index + REVERB_BUFFER_SIZE - delay_samples) % REVERB_BUFFER_SIZE;

    float delayed_sample = allpass_delay_buffer[buffer_index];
    float output = delayed_sample - (input * feedback) + input;

    // Store the new input into the delay buffer
    allpass_delay_buffer[allpass_buffer_index] = input + delayed_sample * feedback;
    allpass_buffer_index = (allpass_buffer_index + 1) % REVERB_BUFFER_SIZE;

    return output;
}

// Applies reverb based on current volume and set mix
float sound_reverb(float volume, float mix)
{

    if (volume < 0.2f)
        volume = 0.2f; // Clamp volume to minimum

    // Apply comb filters
    float reverb_volume = 0.0f;
    for (int i = 0; i < MAX_COMB_FILTERS; i++)
        reverb_volume += comb_filter(volume, paramsSchroeder.comb_delays[i], paramsSchroeder.comb_feedback, paramsSchroeder.sample_rate);

    // Apply all-pass filters
    for (int i = 0; i < MAX_ALLPASS_FILTERS; i++)
        reverb_volume = allpass_filter(reverb_volume, paramsSchroeder.allpass_delays[i], paramsSchroeder.allpass_feedback, paramsSchroeder.sample_rate);

    // Mix original sound with reverb
    return volume * (1.0f - mix) + reverb_volume * mix;
}

// Sets predefined values for each SFX mixer channel
void sound_setChannels(void)
{
    for (int i = 0; i < NUM_WAV; i++)
    {
        switch (i)
        {
        case SFX_WINNER:
            mixer_ch_set_vol_pan(SFX_CHANNEL - i, 0.4f, 0.5f);
            break;
        case SFX_BUILDING:
        case SFX_CAR:
            mixer_ch_set_vol_pan(SFX_CHANNEL - i, sound_reverb(0.9f, 0.6f), 0.5f);
            break;
        case SFX_HYDRANT:
            mixer_ch_set_vol_pan(SFX_CHANNEL - i, sound_reverb(0.9f, 0.2f), 0.5f);
            break;
        case SFX_START:
        case SFX_COUNTDOWN:
        case SFX_STOP:
            mixer_ch_set_vol_pan(SFX_CHANNEL - i, sound_reverb(0.4f, 0.9f), 0.5f);
            break;
        }
    }
}

#endif // SOUND_H