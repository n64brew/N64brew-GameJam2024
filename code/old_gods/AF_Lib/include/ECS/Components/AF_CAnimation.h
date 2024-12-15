/*
===============================================================================
AF_CANIMATION_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_CANIMATION_H
#define AF_CANIMATION_H
#include "AF_Component.h"

#ifdef __cplusplus
extern "C" {    
#endif
/*
====================
AF_CAnimation
====================
*/
// Size is 64 bytes
typedef struct {			// 64 or 128 bytes
	PACKED_CHAR enabled;		// 8 bytes
	AF_FLOAT animationSpeed;	// 16 or 32 bytes
	AF_FLOAT uvScrollingSpeed;
	AF_FLOAT nextFrameTime;		// 16 or 32 bytes
	uint8_t currentFrame;		// 8 bytes
	uint8_t animationFrames;	// 8 bytes
	BOOL loop;			// 8 bytes
} AF_CAnimation;

/*
====================
AF_CAnimation_ZERO
Empty constructor for the AF_CAnimation component
====================
*/
static inline AF_CAnimation AF_CAnimation_ZERO(void){
	AF_CAnimation returnAnimation = {
		//.has = false,
		.enabled = FALSE,
		.animationSpeed = 0,
		.uvScrollingSpeed = 0,
		.nextFrameTime = 0,
		.currentFrame = 0,
		.animationFrames = 0,
		.loop = TRUE
	};
	return returnAnimation;
}

/*
====================
AF_CAnimation_ADD
ADD component and set default values
====================
*/
static inline AF_CAnimation AF_CAnimation_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_CAnimation returnAnimation = {
		//.has = true,
		.enabled = component,
		.animationSpeed = 0,
		.uvScrollingSpeed = 0,
		.nextFrameTime = 0,
		.currentFrame = 0,
		.animationFrames = 0,
		.loop = TRUE
	};
	return returnAnimation;
}


#ifdef __cplusplus
}
#endif

#endif //AF_CANIMATION_H

