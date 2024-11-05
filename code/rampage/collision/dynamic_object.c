#include "dynamic_object.h"

#include "../math/minmax.h"
#include <math.h>
#include <stddef.h>

void dynamic_object_init(
    int entity_id,
    struct dynamic_object* object, 
    struct dynamic_object_type* type,
    uint16_t collision_layers,
    struct Vector3* position, 
    struct Vector2* rotation
) {
    object->entity_id = entity_id;
    object->type = type;
    object->position = *position;
    object->rotation = *rotation;
    object->velocity = gZeroVec;
    object->center = gZeroVec;
    object->has_gravity = 1;
    object->is_trigger = 0;
    object->is_fixed = 0;
    object->is_out_of_bounds = 0;
    object->collision_layers = collision_layers;
    object->collision_group = 0;
    object->active_contacts = 0;
    object->scale = 1.0f;
    dynamic_object_recalc_bb(object);
}

void dynamic_object_update(struct dynamic_object* object, float fixed_time_step) {
    if (object->is_trigger | object->is_fixed) {
        return;
    }

    if (object->has_gravity) {
        object->velocity.y += fixed_time_step * GRAVITY_CONSTANT;
    }

    vector3AddScaled(&object->position, &object->velocity, fixed_time_step, &object->position);
}

struct contact* dynamic_object_nearest_contact(struct dynamic_object* object) {
    struct contact* nearest_target = NULL;
    struct contact* current = object->active_contacts;
    float distance = 0.0f;

    while (current) {
        float check = vector3DistSqrd(&current->point, &object->position);
        if (!nearest_target || check < distance) {
            distance = check;
            nearest_target = current;
        }

        current = current->next;
    }

    return nearest_target;
}

bool dynamic_object_is_touching(struct dynamic_object* object, int id) {
    struct contact* current = object->active_contacts;

    while (current) {
        if (current->other_object == id) {
            return true;
        }
            
        current = current->next;
    }

    return false;
}

void dynamic_object_minkowski_sum(void* data, struct Vector3* direction, struct Vector3* output) {
    struct dynamic_object* object = (struct dynamic_object*)data;

    if (object->rotation.x == 1.0f) {
        object->type->minkowsi_sum(&object->type->data, direction, output);
        vector3Add(output, &object->position, output);
        vector3Add(output, &object->center, output);
        return;
    }

    struct Vector3 rotated_dir;

    rotated_dir.x = direction->x * object->rotation.x - direction->z * object->rotation.y;
    rotated_dir.y = direction->y;
    rotated_dir.z = direction->z * object->rotation.x + direction->x * object->rotation.y;

    struct Vector3 unrotated_out;
    
    object->type->minkowsi_sum(&object->type->data, &rotated_dir, &unrotated_out);

    output->x = unrotated_out.x * object->rotation.x + unrotated_out.z * object->rotation.y;
    output->y = unrotated_out.y;
    output->z = unrotated_out.z * object->rotation.x - unrotated_out.x * object->rotation.y;

    vector3Add(output, &object->center, output);

    if (object->scale != 1.0f) {
        vector3Scale(output, output, object->scale);
    }

    vector3Add(output, &object->position, output);
}

void dynamic_object_recalc_bb(struct dynamic_object* object) {
    object->type->bounding_box(&object->type->data, &object->rotation, &object->bounding_box);
    struct Vector3 offset;
    if (object->scale != 1.0f) {
        vector3Scale(&object->bounding_box.min, &object->bounding_box.min, object->scale);
        vector3Scale(&object->bounding_box.max, &object->bounding_box.max, object->scale);
        vector3AddScaled(&object->position, &object->center, object->scale, &offset);
    } else {
        vector3Add(&object->center, &object->position, &offset);
    }
    vector3Add(&object->bounding_box.min, &offset, &object->bounding_box.min);
    vector3Add(&object->bounding_box.max, &offset, &object->bounding_box.max);
}