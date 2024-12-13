/*
===============================================================================
AF_CSKELETALANIMATION_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_CSKELETALANIMATION_H
#define AF_CSKELETALANIMATION_H
#include "AF_Component.h"

#ifdef __cplusplus
extern "C" {    
#endif

enum AnimationType {
  ANIMATION_TYPE_IDLE = 0,
  ANIMATION_TYPE_WALK = 1, 
  ANIMATION_TYPE_ATTACK = 2
};

/*
====================
AF_CSkeletalAnimation
====================
*/
// Size is 64 bytes
typedef struct {			// 64 or 128 bytes
	PACKED_CHAR enabled;		// 8 bytes
    void* model; 
    void* skeleton;
    void* skeletonBlend;
    const char* animIdlePath;
    // TODO: make this an array with defined indexs
    void* idleAnimationData;
    const char* animWalkPath;
    void* walkAnimationData;
    const char* animAttackPath;
    void* attackAnimationData;
    AF_FLOAT animationSpeed;
    AF_FLOAT animationBlend;
    enum AnimationType animationTypeID;
	AF_FLOAT nextFrameTime;		// 16 or 32 bytes
	uint8_t currentFrame;		// 8 bytes
	uint8_t animationFrames;	// 8 bytes
	BOOL loop;			// 8 bytes
} AF_CSkeletalAnimation;

/*
====================
AF_CSkeletalAnimation_ZERO
Empty constructor for the AF_CAnimation component
====================
*/
static inline AF_CSkeletalAnimation AF_CSkeletalAnimation_ZERO(void){
	AF_CSkeletalAnimation returnAnimation = {
		.enabled = 0,
        .model = NULL,
        .skeleton = NULL,
        .skeletonBlend = NULL,
        .animIdlePath = NULL,
        // TODO: make this an array with defined indexs
        .idleAnimationData = NULL,
        .animWalkPath = NULL,
        .walkAnimationData = NULL,
        .animAttackPath = NULL,
        .attackAnimationData = 0,
        .animationSpeed = 0,
        .animationBlend = 0,
        .animationTypeID = ANIMATION_TYPE_IDLE,
        .nextFrameTime = 0,
        .currentFrame = 0,
        .animationFrames = 0,
        .loop = FALSE
	};
	return returnAnimation;
}

/*
====================
AF_CSkeletalAnimation_ADD
ADD component and set default values
====================
*/
static inline AF_CSkeletalAnimation AF_CSkeletalAnimation_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_CSkeletalAnimation returnAnimation = {
    .enabled = component,
        .model = NULL,
        .skeleton = NULL,
        .skeletonBlend = NULL,
        .animIdlePath = NULL,
        // TODO: make this an array with defined indexs
        .idleAnimationData = NULL,
        .animWalkPath = NULL,
        .walkAnimationData = NULL,
        .animAttackPath = NULL,
        .attackAnimationData = 0,
        .animationSpeed = 0,
        .animationBlend = 0,
        .animationTypeID = ANIMATION_TYPE_IDLE,
        .nextFrameTime = 0,
        .currentFrame = 0,
        .animationFrames = 0,
        .loop = FALSE
	};
	return returnAnimation;
}


#ifdef __cplusplus
}
#endif

#endif //AF_CSKELETALANIMATION_H