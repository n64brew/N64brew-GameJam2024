/*
===============================================================================
AF_C3DRIGIDBODY_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_C3DRIGIDBODY_H
#define AF_C3DRIGIDBODY_H
#include "AF_Lib_Define.h"
#include "AF_CCollider.h"
#include "AF_Vec3.h"
#ifdef __cplusplus
extern "C" {    
#endif

/*
====================
AF_C3DRigidbody 
Component used to contain data for physics
====================
*/
// size is 64 bytes
typedef struct {
    //BOOL has;
    BOOL enabled;
    BOOL isKinematic;
    BOOL  gravity;
    Vec3 velocity;
	Vec3 anglularVelocity;
	float inverseMass;
	Vec3 force;
	Vec3 torque;
	Vec3 inertiaTensor;

} AF_C3DRigidbody;

/*
====================
AF_C3DRigidbody_ZERO
Empty constructor for the CRigidbody component
====================
*/
static inline AF_C3DRigidbody AF_C3DRigidbody_ZERO(void){
	AF_C3DRigidbody rigidbody = {
		//.has = false,
		.enabled = FALSE,
		.isKinematic = FALSE,
		.velocity = {0, 0, 0},
		.anglularVelocity = {0,0,0},
		.gravity = TRUE,
		.inverseMass = 1,
		.force = {0,0,0},
		.torque = {0,0,0},
		.inertiaTensor = {0,0,0}
	};
	return rigidbody;
}
/*
====================
AF_C3DRigidbody_ADD
Add the CRigidbody component
enable the component and set some values to default values.
====================
*/
static inline AF_C3DRigidbody AF_C3DRigidbody_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);

	AF_C3DRigidbody rigidbody = {
		//.has = true,
		.enabled = component,
		.isKinematic = FALSE,			// isKinematic means to be controlled by script rather than the velocity
		.velocity = {0, 0, 0},		// zero velocity 
		.anglularVelocity = {0,0,0},
		.gravity = TRUE,				// gravity off by default
		.inverseMass = 1,
		.force = {0,0,0},
		.torque = {0,0,0},
		.inertiaTensor = {0,0,0}
	};
	return rigidbody;
}


#ifdef __cplusplus
}
#endif

#endif //AF_C3DRIGIDBODY_H

