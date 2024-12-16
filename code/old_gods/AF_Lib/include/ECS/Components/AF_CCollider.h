/*
===============================================================================
AF_CCOLLIDER_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_CCOLLIDER_H
#define AF_CCOLLIDER_H
#include <stddef.h>
#include "AF_Lib_Define.h"
#include "AF_Rect.h"
#include "AF_Collision.h"
#include "AF_CollisionVolume.h"
#include "ECS/Components/AF_Component.h"
#ifdef __cplusplus
extern "C" {    
#endif

/*
====================
AF_CCollider Struct used for physics
====================
*/
typedef struct {
    PACKED_CHAR enabled;
    enum CollisionVolumeType type;
    Vec3 boundingVolume;
	Vec3 pos;
    //AF_Rect bounds;
    AF_Collision collision; //TODO: why do we need this?
    BOOL showDebug;
	Vec3 broadphaseAABB;
} AF_CCollider;

// dummy callback so our initialisation compiles when we call AF_CCollider_Add
static void AF_Collision_DummyCallback(AF_Collision* _collisionPtr){
	if(_collisionPtr){}
}
/*
====================
AF_CCollider_ZERO
Add the CCollider component
enable the component and set some values to default values.
====================
*/
static inline AF_CCollider AF_CCollider_ZERO(void){
	PACKED_CHAR component = FALSE;
	component = AF_Component_SetHas(component, FALSE);
	component = AF_Component_SetEnabled(component, FALSE);
	AF_CCollider collider = {
		.enabled = FALSE,//component,
		//.has = TRUE,
		.type = AABB,
		.boundingVolume = {0,0,0},
		.pos = {0,0,0},
		/*
		.bounds = {
			0,	// x
			0,	// y
			0,	// width
			0	// height
		},*/
		//.collision = {FALSE, NULL, NULL},
		.showDebug = FALSE,
		.broadphaseAABB = {0,0,0}
	};
    collider.collision.collided  = FALSE;
	collider.collision.entity1 = NULL;
	collider.collision.entity2 = NULL;
	collider.collision.callback = AF_Collision_DummyCallback;//(void (*)(AF_Collision))0;
	Vec3 defaultCollisionPoint = {0,0,0};
	collider.collision.collisionPoint = defaultCollisionPoint;
	collider.collision.rayDistance = 0.0f;
	return collider;
}


/*
====================
AF_CCollider_ADD
Add the CCollider component
enable the component and set some values to default values.
====================
*/
static inline AF_CCollider AF_CCollider_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_CCollider collider = {
		//.has = TRUE,
		.enabled = TRUE,//component,
		.type = AABB,
		.boundingVolume = {1,1,1},
		.pos = {0,0,0},
		/*
		.bounds = {
			0,	// x
			0,	// y
			0,	// width
			0	// height
		},*/
		//.collision = {FALSE, NULL, NULL},
		.showDebug = FALSE,
		.broadphaseAABB = {0,0,0}
	};
        collider.collision.collided  = FALSE;
	collider.collision.entity1 = NULL;
	collider.collision.entity2 = NULL;
	collider.collision.callback = AF_Collision_DummyCallback;//(void (*)(AF_Collision))0;
	Vec3 defaultCollisionPoint = {0,0,0};
	collider.collision.collisionPoint = defaultCollisionPoint;
	collider.collision.rayDistance = 0.0f;
	return collider;
}



/*
====================
AF_CCollider_SPHERE_ADD
Add the CCollider component
enable the component and set some values to default values.
====================
*/
static inline AF_CCollider AF_CCollider_Sphere_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_CCollider collider = {
		//.has = TRUE,
		.enabled = TRUE,//component,
		.type = Sphere,
		.boundingVolume = {1,0,0},
		.pos = {0,0,0},
		/*
		.bounds = {
			0,	// x
			0,	// y
			0,	// width
			0	// height
		},*/
		//.collision = {FALSE, NULL, NULL},
		.showDebug = FALSE,
		.broadphaseAABB = {0,0,0}
	};
        collider.collision.collided  = FALSE;
	collider.collision.entity1 = NULL;
	collider.collision.entity2 = NULL;
	collider.collision.callback = AF_Collision_DummyCallback;//(void (*)(AF_Collision))0;
	Vec3 defaultCollisionPoint = {0,0,0};
	collider.collision.collisionPoint = defaultCollisionPoint;
	collider.collision.rayDistance = 0.0f;
	return collider;
}
//
/*
====================
AF_CCollider_BOX_ADD
Add the CCollider component
enable the component and set some values to default values.
====================
*/
static inline AF_CCollider AF_CCollider_Box_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_CCollider collider = {
		//.has = TRUE,
		.enabled = TRUE,//component,
		.type = AABB,
		.boundingVolume = {1,1,1},
		.pos = {0,0,0},
		/*
		.bounds = {
			0,	// x
			0,	// y
			0,	// width
			0	// height
		},*/
		//.collision = {FALSE, NULL, NULL},
		.showDebug = FALSE,
		.broadphaseAABB = {0,0,0}
	};
        collider.collision.collided  = FALSE;
	collider.collision.entity1 = NULL;
	collider.collision.entity2 = NULL;
	collider.collision.callback = AF_Collision_DummyCallback;//(void (*)(AF_Collision))0;
	Vec3 defaultCollisionPoint = {0,0,0};
	collider.collision.collisionPoint = defaultCollisionPoint;
	collider.collision.rayDistance = 0.0f;
	return collider;
}

/*
====================
AF_CCollider_PLANE_ADD
Add the CCollider component
enable the component and set some values to default values.
====================
*/
static inline AF_CCollider AF_CCollider_Plane_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_CCollider collider = {
		//.has = TRUE,
		.enabled = TRUE,//component,
		.type = Plane,
		.boundingVolume = {0,.001f,0},
		.pos = {0,0,0},
		/*
		.bounds = {
			0,	// x
			0,	// y
			0,	// width
			0	// height
		},*/
		//.collision = {FALSE, NULL, NULL},
		.showDebug = FALSE,
		.broadphaseAABB = {0,0,0}
	};
        collider.collision.collided  = FALSE;
	collider.collision.entity1 = NULL;
	collider.collision.entity2 = NULL;
	collider.collision.callback = AF_Collision_DummyCallback;//(void (*)(AF_Collision))0;
	Vec3 defaultCollisionPoint = {0,0,0};
	collider.collision.collisionPoint = defaultCollisionPoint;
	collider.collision.rayDistance = 0.0f;
	return collider;
}


/*
====================
AF_CCollider_ADD_TYPE
Add the CCollider of the MESH Type
enable the component and set some values to default values.
====================
*/

static inline AF_CCollider AF_CCollider_ADD_TYPE(enum CollisionVolumeType _volumeType){
	switch(_volumeType){
		case  AABB:
			return AF_CCollider_Box_ADD();
		break;

		case OBB:
			// TODO add OBB collider type
			return AF_CCollider_Box_ADD();
		break;

		case Plane:
			return AF_CCollider_Plane_ADD();
		break;

		case Sphere:
			// todo, add sphere collider type
			return AF_CCollider_Box_ADD();
		break;

		case Mesh:
			// todo, add mesh collider type
			return AF_CCollider_Box_ADD();
		break;

		case Compound:
			// TODO: add compound collider type
			return AF_CCollider_Box_ADD();
		break;

		case Invalid:
			return AF_CCollider_Box_ADD();
		break;

		default:
			return AF_CCollider_Box_ADD();
		break;
	}
}

#ifdef __cplusplus
}
#endif

#endif //AF_CCOLLIDER_H

