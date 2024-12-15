#include "swing_collider.h"

#include "./dynamic_object.h"

void swing_colliderminkowski_sum(void* data, struct Vector3* direction, struct Vector3* output) {
    union dynamic_object_type_data* shape_data = (union dynamic_object_type_data*)data;

    struct Vector3i16* points = shape_data->swing_collider.points;

    float score = 0.0f;
    
    for (int i = 0; i < 4; i += 1) {
        struct Vector3 point;
        vector3i16ToF(&points[i], &point);

        float distance = vector3Dot(&point, direction);

        if (i == 0 || distance > score) {
            score = distance;
            *output = point;
        }
    }
}

void swing_colliderbounding_box(void* data, struct Vector2* rotation, struct Box3D* box) {
    union dynamic_object_type_data* shape_data = (union dynamic_object_type_data*)data;

    struct Vector3i16* points = shape_data->swing_collider.points;

    vector3i16ToF(&points[0], &box->min);
    box->max = box->min;

    for (int i = 1; i < 4; i += 1) {
        struct Vector3 point;
        vector3i16ToF(&points[i], &point);

        vector3Min(&box->min, &point, &box->min);
        vector3Max(&box->max, &point, &box->max);
    }
}