#include "collide.h"

#include "epa.h"

#include "collision_scene.h"
#include "../util/flags.h"
#include <stdio.h>


void correct_velocity(struct dynamic_object* object, struct EpaResult* result, float ratio, float friction, float bounce) {
    float velocityDot = vector3Dot(&object->velocity, &result->normal);

    if ((velocityDot < 0) == (ratio < 0)) {
        struct Vector3 tangentVelocity;

        vector3AddScaled(&object->velocity, &result->normal, -velocityDot, &tangentVelocity);
        vector3Scale(&tangentVelocity, &tangentVelocity, 1.0f - friction);

        vector3AddScaled(&tangentVelocity, &result->normal, velocityDot * -bounce, &object->velocity);
    }
}

void correct_overlap(struct dynamic_object* object, struct EpaResult* result, float ratio, float friction, float bounce) {
    if (object->is_fixed) {
        return;
    }

    vector3AddScaled(&object->position, &result->normal, result->penetration * ratio, &object->position);

    correct_velocity(object, result, ratio, friction, bounce);
}

void collide_object_to_world(struct dynamic_object* object) {   
    if (object->is_trigger) {
        return;
    }

    struct Vector3 down;
    vector3Negate(&gUp, &down);
    struct Vector3 bottom_most;
    dynamic_object_minkowski_sum(object, &down, &bottom_most);

    if (bottom_most.y < 0.0f) {
        struct EpaResult result;

        result.contactA = bottom_most;
        result.contactB = bottom_most;
        result.contactB.y = 0.0f;
        result.normal = gUp;
        result.penetration = bottom_most.y;

        correct_overlap(object, &result, -1.0f, object->type->friction, object->type->bounce);
        collide_add_contact(object, &result);
    }
}

void collide_object_to_object(struct dynamic_object* a, struct dynamic_object* b) {
    if (!(a->collision_layers & b->collision_layers)) {
        return;
    }

    if (a->collision_group && a->collision_group == b->collision_group) {
        return;
    }

    if (a->is_trigger && b->is_trigger) {
        return;
    }

    struct Simplex simplex;
    if (!gjkCheckForOverlap(&simplex, a, dynamic_object_minkowski_sum, b, dynamic_object_minkowski_sum, &gRight)) {
        return;
    }

    if (a->is_trigger || b->is_trigger) {
        struct contact* contact = collision_scene_new_contact();

        if (!contact) {
            return;
        }

        if (b->is_trigger) {
            contact->normal = gZeroVec;
            contact->point = a->position;
            contact->other_object = a ? a->entity_id : 0;

            contact->next = b->active_contacts;
            b->active_contacts = contact;
        } else {
            contact->normal = gZeroVec;
            contact->point = b->position;
            contact->other_object = b ? b->entity_id : 0;

            contact->next = a->active_contacts;
            a->active_contacts = contact;
        }

        return;
    }

    struct EpaResult result;

    if (!epaSolve(&simplex, a, dynamic_object_minkowski_sum, b, dynamic_object_minkowski_sum, &result)) {
        return;
    }

    float friction = a->type->friction < b->type->friction ? a->type->friction : b->type->friction;
    float bounce = a->type->friction > b->type->friction ? a->type->friction : b->type->friction;

    // TODO determine push 
    correct_overlap(b, &result, -0.5f, friction, bounce);
    correct_overlap(a, &result, 0.5f, friction, bounce);

    struct contact* contact = collision_scene_new_contact();

    if (!contact) {
        return;
    }

    contact->normal = result.normal;
    contact->point = result.contactA;
    contact->other_object = a ? a->entity_id : 0;

    contact->next = b->active_contacts;
    b->active_contacts = contact;

    contact = collision_scene_new_contact();

    if (!contact) {
        return;
    }
    
    vector3Negate(&result.normal, &contact->normal);
    contact->point = result.contactB;
    contact->other_object = b ? b->entity_id : 0;

    contact->next = a->active_contacts;
    a->active_contacts = contact;
}

void collide_add_contact(struct dynamic_object* object, struct EpaResult* result) {
    struct contact* contact = collision_scene_new_contact();

    if (!contact) {
        return;
    }

    contact->normal = result->normal;
    contact->point = result->contactA;
    contact->other_object = 0;

    contact->next = object->active_contacts;
    object->active_contacts = contact;
}