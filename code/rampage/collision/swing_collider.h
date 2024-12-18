#ifndef __SWING_COLLIDER_H__
#define __SWING_COLLIDER_H__

#include "../math/vector2.h"
#include "../math/vector3.h"
#include "../math/box3d.h"

void swing_colliderminkowski_sum(void* data, struct Vector3* direction, struct Vector3* output);
void swing_colliderbounding_box(void* data, struct Vector2* rotation, struct Box3D* box);

#endif