/*
===============================================================================
Implementation of AF_Audio.h to be used with 
n64 libdragon
===============================================================================
*/
#include "AF_Audio.h"
#include <libdragon.h>

// Sound
// data type to hold special n64 data
wav64_t sfx_cannon, sfx_laser, sfx_music;
// Mixer channel allocation
#define CHANNEL_SFX1    0
#define CHANNEL_SFX2    1
#define CHANNEL_MUSIC   2

//We need to show lots of internal details of the module which are not
// exposed via public API, so include the internal header file.
//#include "./libdragon/src/audio/libxm/xm_internal.h"
//#include "libdragon/src/audio/audio/libxm/xm_internal.h" //"libdragon/src/audio/libxm/xm_internal.h"
//#include "xm_internal.h"
// Forward declare
void PlayMusic();

// Play audio using lib dragon mixer
void AF_Audio_Play(AF_CAudioSource* _audioSource, float _volume, BOOL _isLooping){
    if(_audioSource == NULL){
        debugf("Audio: AF_Audio_Play: tried to play a null audio source\n");
        return;
    }
    _audioSource->loop = _isLooping;
    wav64_set_loop((wav64_t*)_audioSource->clipData, _audioSource->loop);
    wav64_play((wav64_t*)_audioSource->clipData, _audioSource->channel);
	mixer_ch_set_vol(_audioSource->channel, _volume, _volume);
}

void PlayMusic(){
    //xm64player_set_vol (&xm, 1.0f);
    //xm64player_open(&xm, cur_rom);
	//xm64player_play(&xm, 2);
}
