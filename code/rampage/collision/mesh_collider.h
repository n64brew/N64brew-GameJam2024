#ifndef __COLLISION_MESH_COLLIDER_H__
#define __COLLISION_MESH_COLLIDER_H__

#include <stdint.h>
#include <stdbool.h>

#include "../math/vector3.h"
#include "../math/box3d.h"

struct mesh_triangle_indices {
    uint16_t indices[3];
};

struct mesh_index_block {
    uint16_t first_index;
    uint16_t last_index;
};

struct mesh_collider {
    struct Vector3* vertices;
    struct mesh_triangle_indices* triangles;
    uint16_t triangle_count;
};

struct mesh_triangle {
    struct Vector3* vertices;
    struct mesh_triangle_indices triangle;
};

typedef bool (*triangle_callback)(struct mesh_collider* collider, void* data, int triangle_index);

void mesh_triangle_minkowski_sum(void* data, struct Vector3* direction, struct Vector3* output);

bool mesh_collider_for_each(struct mesh_collider* collider, triangle_callback callback, void* data);

#endif