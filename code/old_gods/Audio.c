#include "AF_Audio.h"
#include <libdragon.h>

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

