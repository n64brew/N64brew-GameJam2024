/*
===============================================================================
AF_COLLISIONVOLUME_H definitions

Definition for the collision volume structs
and helper functions.
Types of collision volumes include AABB, OBB, Mesh, Sphere and compound
===============================================================================
*/
#ifndef AF_COLLISIONVOLUME_H
#define AF_COLLISIONVOLUME_H
#include "AF_Lib_Define.h"
#include "AF_Vec3.h"
#include "AF_Vec4.h"

#ifdef __cplusplus
extern "C" {    
#endif

enum CollisionVolumeType {
	AABB = 1, 
	OBB = 2, 
	Plane = 3,
	Sphere = 4, 
	Mesh = 8, 
	Compound = 16, 
	Invalid = 256
};

/*
====================
Sphere_CollisionVolume Struct used to describe a sphere volume 
====================
*/
typedef struct{ 
	Vec3 bounds;
}Sphere_CollisionVolume;

/*
====================
Box_CollisionVolume Struct used to describe a box volume 
====================
*/
typedef struct{ 
	Vec3 bounds;
}Box_CollisionVolume;

/*
====================
AABB_CollisionVolume Struct used to describe a volume 
====================
*/
typedef struct{ 
	Vec3 bounds;
}AABB_CollisionVolume;

/*
====================
OBB_CollisionVolume Struct used to describe a volume 
====================
*/
typedef struct {
	Vec3 bounds;
}OBB_CollisionVolume;

/*
====================
Mesh_CollisionVolume Struct used to describe a volume 
====================
*/
typedef struct {
	Vec3 bounds;
}Mesh_CollisionVolume;

/*
====================
Compound_CollisionVolume Struct used to describe a volume 
====================
*/
typedef struct {
	Vec4 bounds;
}Compound_CollisionVolume;


#ifdef __cplusplus
}
#endif

#endif //AF_COLLISIONVOLUME_H
