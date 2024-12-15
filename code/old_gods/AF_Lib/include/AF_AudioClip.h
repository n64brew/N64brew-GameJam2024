/*
===============================================================================
AF_AUDIOALIP_H definitions

Definition for the audio clip component struct
and audio helper functions
===============================================================================
*/
#ifndef AF_AUDIOALIP_H
#define AF_AUDIOALIP_H
#include "AF_Lib_Define.h"

typedef struct AF_CAudioClip {
    uint8_t clipID;
    const char* clipPath;
    uint32_t clipFrequency;
} AF_AudioClip;

#endif  // AF_AUDIOALIP_H