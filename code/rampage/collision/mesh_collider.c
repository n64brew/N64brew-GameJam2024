#include "mesh_collider.h"

#include <math.h>
#include <stdio.h>
#include "../math/minmax.h"

#define MAX_INDEX_SET_SIZE 64

void mesh_triangle_minkowski_sum(void* data, struct Vector3* direction, struct Vector3* output) {
    struct mesh_triangle* triangle = (struct mesh_triangle*)data;

    int idx = 0;
    float distance = vector3Dot(&triangle->vertices[triangle->triangle.indices[0]], direction);

    float check = vector3Dot(&triangle->vertices[triangle->triangle.indices[1]], direction);

    if (check > distance) {
        idx = 1;
        distance = check;
    }

    check = vector3Dot(&triangle->vertices[triangle->triangle.indices[2]], direction);

    if (check > distance) {
        idx = 2;
    }

    *output = triangle->vertices[triangle->triangle.indices[idx]];
}

float mesh_index_inv_offset(float input) {
    if (fabsf(input) < 0.000001f) {
        return infinityf();
    }

    return 1.0f / input;
}

enum mesh_range_result {
    MESH_RANGE_RESULT_COLLIDE,
    MESH_RANGE_RESULT_SKIP,
    MESH_RANGE_RESULT_END,
};

bool is_inf(float value) {
    return value == infinityf() || value == -infinityf();
}

bool mesh_collider_for_each(struct mesh_collider* collider, triangle_callback callback, void* data) {
    for (int i = 0; i < collider->triangle_count; i += 1) {
        if (callback(collider, data, i)) {
            return true;
        }
    }

    return false;
}