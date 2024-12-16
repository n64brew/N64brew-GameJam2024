#ifndef SOUND_H
#define SOUND_H

#include <libdragon.h>

// Core Definitions
#define MUSIC_CHANNEL 0
#define SFX_CHANNEL 31

// XM sequences
enum BG_XM
{
    XM_SH,
    NUM_XM
};

xm64player_t xmPlayer;

const char *xmFileNames[NUM_XM] = {
    "rom:/strawberry_byte/sound/sky_high.xm64",
};

// WAV files
enum SFX_WAV
{
    SFX_JUMP,
    SFX_STONES,
    SFX_WIND,
    SFX_START,
    SFX_COUNTDOWN,
    SFX_STOP,
    SFX_WINNER,
    NUM_WAV
};

// Each WAV must have its own structure
wav64_t soundEffects[NUM_WAV];

const char *wavFileNames[NUM_WAV] = {
    "rom:/strawberry_byte/sound/grunt-01.wav64",
    "rom:/strawberry_byte/sound/stones-falling.wav64",
    "rom:/strawberry_byte/sound/strong_wind_blowing.wav64",
    "rom:/core/Start.wav64",
    "rom:/core/Countdown.wav64",
    "rom:/core/Stop.wav64",
    "rom:/core/Winner.wav64",
};

/* Function Declarations */
void sound_load(void);
void sound_xmSwitch(int songID, float volume, bool loop);
void sound_xmStop(void);
void sound_xmUpdate(float volume, bool loop);
void sound_wavPlay(int sfxID, bool loop);
void sound_wavClose(int sfxID);
void sound_wavCleanup(void);
void sound_cleanup(void);
void sound_update(void);

/* Function Definitions */

void sound_load(void)
{
    // Open all WAVs at boot
    for (int w = 0; w < NUM_WAV; ++w)
        wav64_open(&soundEffects[w], wavFileNames[w]);

    // Open and play first XM in the list
    xm64player_open(&xmPlayer, xmFileNames[0]);
    xm64player_set_vol(&xmPlayer, 0.5f);
    xm64player_play(&xmPlayer, MUSIC_CHANNEL);
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
    xm64player_close(&xmPlayer);
    sound_wavCleanup();
}

void sound_update(void)
{
    mixer_try_play();
}

// Function to calculate forward facing direction of the camera based on its target
Vector3 calculate_camera_forward(const Camera *camera)
{
    Vector3 direction = {
        camera->target.x - camera->position.x,
        camera->target.y - camera->position.y,
        camera->target.z - camera->position.z};
    vector3_normalize(&direction);
    return direction;
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

////// Spatial 3D sound experiment
void sound_spatial(const Vector3 *spawner,
                   const Vector3 *player,
                   const Camera *camera)
{

    float pan = 0.5f;
    float volume = 1.0f;

    // Calculate distance vector for volume attenuation
    Vector3 diff = vector3_difference(spawner, player);
    float distance = vector3_magnitude(&diff);

    // If close enough to the sound spawner, skip unnecessary spatial calculations
    if (distance > 250.0f)
    {
        // Volume attenuation (inverse-square or linear falloff)
        float max_distance = 8000.0f; // Maximum distance for audible sound
        volume = 1.0f - (distance / max_distance);
        volume = fmaxf(0.1f, volume); // Clamp volume to minimum

        // Calculate the horizontal angle (azimuth) for panning
        Vector3 horizontal_diff = {diff.x, diff.y, 0.0f};
        float horizontal_distance = vector3_magnitude(&horizontal_diff);

        // Avoid division by zero
        if (horizontal_distance > 0.0f)
        {
            Vector3 player_forward = calculate_camera_forward(camera);
            float cos_angle = vector3_returnDotProduct(&horizontal_diff, &player_forward) / horizontal_distance;
            float angle = acosf(cos_angle); // Angle in radians

            // Determine if sound is to the left or right
            pan = 0.5f * (1.0f - angle / M_PI); // Centered at 0.5, where 0 is left and 1 is right

            // Flip pan if the sound source is on the right side
            if (diff.x * player_forward.y - diff.y * player_forward.x > 0)
                pan = 1.0f - pan;

            // Cap the pan
            pan = fminf(0.8f, fmaxf(0.2f, pan));
        }
        else
        {
            // If horizontal_distance is zero, keep the pan centered
            pan = 0.5f;
        }

        // Apply a vertical attenuation if sound changes by height
        float vertical_attenuation = fminf(1.0f, fmaxf(0.1f, 1.0f - fabsf(diff.z) / max_distance));

        // Volume adjustment with vertical attenuation
        volume *= vertical_attenuation;

        // Calculate reverb mix based on distance (0.3 at 250, 1.0 at max_distance)
        float min_reverb_distance = 250.0f;
        float mix = (distance - min_reverb_distance) / (max_distance - min_reverb_distance);
        mix = fminf(1.0f, fmaxf(0.3f, mix)); // Clamp mix to [0.0, 1.0]

        // Apply reverb
        volume = sound_reverb(volume, mix);
    }

    // Set the volume and panning for each SFX channel
    for (int i = 0; i < NUM_WAV; i++)
    {
        mixer_ch_set_vol_pan(SFX_CHANNEL - i, volume, pan);
    }
}

#endif // SOUND_H