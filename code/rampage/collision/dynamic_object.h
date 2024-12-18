#ifndef __COLLISION_DYNAMIC_OBJECT_H__
#define __COLLISION_DYNAMIC_OBJECT_H__

#include "../math/vector3.h"
#include "../math/box3d.h"
#include "../math/box2d.h"
#include "./contact.h"
#include "./gjk.h"
#include <stdint.h>
#include <stdbool.h>

#define GRAVITY_CONSTANT    (-9.8f * 64.0f)

enum collision_layers {
    COLLISION_LAYER_TANGIBLE = (1 << 0),
};

enum collision_group {
    COLLISION_GROUP_PLAYER = 1,
};

typedef void (*bounding_box_calculator)(void* data, struct Vector2* rotation, struct Box3D* box);

union dynamic_object_type_data {
    struct { float radius; } sphere;
    struct { float radius; float inner_half_height; } capsule;
    struct { struct Vector3 half_size; } box;
    struct { struct Vector3 size; } cone;
    struct { float radius; float half_height; } cylinder;
    struct { struct Vector2 range; float radius; float half_height; } sweep;
    struct { struct Vector3i16 points[4]; } swing_collider;
};

struct dynamic_object_type {
    MinkowsiSum minkowsi_sum;
    bounding_box_calculator bounding_box;
    union dynamic_object_type_data data;
    float bounce;
    float friction;
};

struct dynamic_object {
    int entity_id;
    struct Vector3 position;
    struct Vector2 rotation;
    struct Vector3 velocity;
    struct dynamic_object_type* type;
    float scale;
    struct Vector3 center;
    struct Box3D bounding_box;
    uint16_t has_gravity: 1;
    uint16_t is_trigger: 1;
    uint16_t is_fixed: 1;
    uint16_t is_out_of_bounds: 1;
    uint16_t collision_layers;
    uint16_t collision_group;
    struct contact* active_contacts;
};

void dynamic_object_init(
    int entity_id,
    struct dynamic_object* object, 
    struct dynamic_object_type* type,
    uint16_t collision_layers,
    struct Vector3* position, 
    struct Vector2* rotation
);

void dynamic_object_update(struct dynamic_object* object, float fixed_time_step);

struct contact* dynamic_object_nearest_contact(struct dynamic_object* object);
bool dynamic_object_is_touching(struct dynamic_object* object, int id);

void dynamic_object_minkowski_sum(void* data, struct Vector3* direction, struct Vector3* output);
void dynamic_object_recalc_bb(struct dynamic_object* object);

#endif