/*
===============================================================================
AF_CTransform2D_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_CTRANSFORM2D_H
#define AF_CTRANSFORM2D_H
#include "AF_Vec2.h"
#include "AF_Lib_Define.h"
#include "AF_Component.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
====================
AF_CTransform2D
basic struct for the transform component
====================
*/
// Size is 64 bytes
typedef struct {
    PACKED_CHAR enabled;
    Vec2 pos;
    Vec2 rot;
    Vec2 scale;
} AF_CTransform2D;

/*
====================
AF_CTransform2D_ZERO
Empty constructor
====================
*/
static inline AF_CTransform2D AF_CTransform2D_ZERO(void){
	AF_CTransform2D returnTransform = {
        .enabled = FALSE,
        .pos = {0, 0},
        .rot = {0, 0},
        .scale = {1, 1}
	// Default position matrix
    };
	return returnTransform;
}

/*
====================
AF_CTransform2D_ADD
Add component constructor for the component
====================
*/
static inline AF_CTransform2D AF_CTransform2D_ADD(void){
	AF_CTransform2D returnTransform = {
        //.has = TRUE,
        .enabled = TRUE,
        .pos = {0, 0},
        .rot = {0, 0},
        .scale = {1, 1}
	// Default position matrix
    };
	return returnTransform;
}

#ifdef __cplusplus
}
#endif

#endif  // AF_TRANSFORM2D_H
