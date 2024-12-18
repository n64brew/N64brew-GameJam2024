#ifndef __COLLISION_RAYCAST_H__
#define __COLLISION_RAYCAST_H__

#include <stdbool.h>
#include "../math/ray.h"

struct RaycastHit {
    struct Vector3 at;
    struct Vector3 normal;
    int entity_id;
};

bool collision_raycast(struct Ray* ray, int collision_layers, struct RaycastHit* hit);

#endif