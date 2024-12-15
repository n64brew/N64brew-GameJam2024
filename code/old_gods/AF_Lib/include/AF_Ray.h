/*
===============================================================================
AF_RAY_H definitions
Definition of the ray structure
===============================================================================
*/
#ifndef AF_RAY_H
#define AF_RAY_H
#include "AF_Vec3.h"

#ifdef __cplusplus
extern "C" {    
#endif

/*
====================
Ray Struct used to describe a ray used in raycasting 
====================
*/
typedef struct {
	Vec3 position;
	Vec3 direction;
}Ray;


#ifdef __cplusplus
}
#endif

#endif //AF_RAY_H
