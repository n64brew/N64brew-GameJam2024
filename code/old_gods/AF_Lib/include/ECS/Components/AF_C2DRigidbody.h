/*
===============================================================================
AF_C2DRIGIDBODY_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_C2DRIGIDBODY_H
#define AF_C2DRIGIDBODY_H
#include "AF_Lib_Define.h"
#include "AF_CCollider.h"
#include "AF_Vec2.h"
#ifdef __cplusplus
extern "C" {    
#endif

/*
====================
AF_C2DRigidbody 
Component used to contain data for physics
====================
*/
// size is 64 bytes
typedef struct {
    //BOOL has;
    BOOL enabled;
    BOOL isKinematic;
    uint32_t  gravity;
    Vec2 velocity;
} AF_C2DRigidbody;

/*
====================
AF_C2DRigidbody_ZERO
Empty constructor for the CRigidbody component
====================
*/
static inline AF_C2DRigidbody AF_C2DRigidbody_ZERO(void){
	AF_C2DRigidbody rigidbody = {
		//.has = false,
		.enabled = FALSE,
		.isKinematic = FALSE,
		.velocity = {0, 0},
		.gravity = 0
	};
	return rigidbody;
}
/*
====================
AF_C2DRigidbody_ADD
Add the CRigidbody component
enable the component and set some values to default values.
====================
*/
static inline AF_C2DRigidbody AF_C2DRigidbody_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);

	AF_C2DRigidbody rigidbody = {
		//.has = true,
		.enabled = component,
		.isKinematic = FALSE,			// isKinematic means to be controlled by script rather than the velocity
		.velocity = {0, 0},		// zero velocity 
		.gravity = 0				// gravity off by default
	};
	return rigidbody;
}


#ifdef __cplusplus
}
#endif

#endif //AF_C2DRIGIDBODY_H

