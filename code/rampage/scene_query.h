#ifndef __RAMPAGE_SCENE_QUERY_H__
#define __RAMPAGE_SCENE_QUERY_H__

#include "./math/vector3.h"

struct Vector3* find_nearest_target(struct Vector3* from, float error_tolerance);

#endif