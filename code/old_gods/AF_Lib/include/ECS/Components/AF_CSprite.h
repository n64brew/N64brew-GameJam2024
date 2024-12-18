/*
===============================================================================
AF_CSPRITE_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_CSPRITE_H
#define AF_CSPRITE_H
#include "AF_Component.h"
#include "AF_Vec2.h"
#ifdef __cplusplus
extern "C" {    
#endif
/*
====================
AF_CSprite
====================
*/
// size is 64 bytes
typedef struct {
	//BOOL has;
	//BOOL enabled;
	PACKED_CHAR enabled;	    // 1 byte
	BOOL loop;		    // 1 byte
	char currentFrame;	    // 1 byte
	char animationFrames;	    // 1 byte
	AF_FLOAT currentFrameTime;
	AF_FLOAT nextFrameTime;     // 4 bytes
	AF_FLOAT animationSpeed;    // 4 bytes
	Vec2 spritePos;		    // 8 bytes
	Vec2 spriteSize;    	// size of sprite in pixels
	Vec2 spriteScale;		// transform scale
	float spriteRotation;	// rotation
	BOOL flipX;
	BOOL flipY;
	Vec2 spriteSheetSize;    // 8 bytes
	uint8_t spriteColor[4];
	const char* spritePath;
	void* spriteData; // special ptr for sprite data to be cast when known
	BOOL isPlaying;
	BOOL filtering;
	
} AF_CSprite;

/*
====================
AF_CSprite_ZERO
Empty constructor for the AF_CSprite component
====================
*/
static inline AF_CSprite AF_CSprite_ZERO(void){
	AF_CSprite returnSprite = {
		//.has = false,
		.enabled = FALSE,
		.loop = TRUE,
		.currentFrame = 0,
		.animationFrames = 0,
		.currentFrameTime = 0,
		.nextFrameTime = 0,
		.animationSpeed = 0,
		.spritePos = {0, 0},
		.spriteSize = {0, 0},
		.spriteScale = {0,0},
		.spriteRotation = 0.0f,
		.flipX = FALSE,
		.flipY = FALSE,
		.spriteSheetSize = {0, 0},
		.spriteColor = {255, 255, 255, 255},
		.spritePath = NULL,
		.spriteData = NULL,
		.isPlaying = FALSE,
		.filtering = FALSE
		};
	return returnSprite;
}

/*
====================
AF_CSprite_ADD
ADD component and set default values
====================
*/
static inline AF_CSprite AF_CSprite_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_CSprite returnSprite = {
		//.has = true,
		.enabled = component,
		.loop = TRUE,
		.currentFrame = 0,
		.animationFrames = 0,
		.currentFrameTime = 0,
		.nextFrameTime = 0,
		.animationSpeed = 0,
		.spritePos = {0, 0},
		.spriteSize = {0, 0},
		.spriteScale = {0,0},
		.spriteRotation = 0.0f,
		.flipX = FALSE,
		.flipY = FALSE,
		.spriteSheetSize = {0, 0},
		.spriteColor = {255, 255, 255, 255},
		.spritePath = NULL,
		.spriteData = NULL,
		.isPlaying = FALSE, 
		.filtering = FALSE
	};
	return returnSprite;
}



#ifdef __cplusplus
}
#endif

#endif //AF_CSPRITE_H

