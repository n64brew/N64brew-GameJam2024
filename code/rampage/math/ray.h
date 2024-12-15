#ifndef __RAY_H__
#define __RAY_H__

#include "vector3.h"

struct Ray {
    struct Vector3 origin;
    struct Vector3 dir;
};

float rayDetermineDistance(struct Ray* ray, struct Vector3* point);

#endif