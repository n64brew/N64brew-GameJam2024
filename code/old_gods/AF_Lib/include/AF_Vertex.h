/*
===============================================================================
AF_UTIL_H

Definition of vertex struct

===============================================================================
*/
#ifndef AF_VERTEX_H
#define AF_VERTEX_H
#include "AF_Vec3.h"
#include "AF_Vec2.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
====================
AF_Vertex
vertex struct
====================
*/
typedef struct {
    Vec3 position;
    Vec3 normal;
    //Vec3 tangent;
    //Vec3 bitangent;
    Vec2 texCoord;
} AF_Vertex;

#ifdef __cplusplus
}
#endif

#endif
