/*
===============================================================================
AF_CAudioSource_H definitions

Definition for the audio source component struct
and audio helper functions
===============================================================================
*/
#ifndef AF_CAudioSource_H
#define AF_CAudioSource_H
#include "AF_AudioClip.h"
#include "AF_Lib_Define.h"
#include "AF_Component.h"

typedef struct AF_CAudioSource {
    PACKED_CHAR enabled;	
    AF_AudioClip clip;
    uint8_t channel;
    BOOL loop;
    BOOL isPlaying;
    void* clipData; // special void* for data if needed .e.g wav64 format for n64/libdragon
} AF_CAudioSource;

/*
====================
AF_CAudioSource_ZERO
Function used to create an empty text component
====================
*/
static inline AF_CAudioSource AF_CAudioSource_ZERO(void){
    AF_CAudioSource returnMesh = {
	.enabled = FALSE,
    .clip = {0,0,0},
    .channel = 255,
    .loop = FALSE,
    .isPlaying = FALSE,
    .clipData = NULL
    };
    return returnMesh;
}

/*
====================
AF_CAudioSource_ADD
Function used to Add the component
====================
*/
static inline AF_CAudioSource AF_CAudioSource_ADD(void){
    PACKED_CHAR component = 0;
    component = AF_Component_SetHas(component, TRUE);
    component = AF_Component_SetEnabled(component, TRUE);

    AF_CAudioSource returnMesh = {
	.enabled = component,
    .clip = {0,0,0},
    .channel = 255,
    .loop = FALSE,
    .isPlaying = FALSE,
    .clipData = NULL
    };
    return returnMesh;
}

#endif // AF_CAudioSource_H