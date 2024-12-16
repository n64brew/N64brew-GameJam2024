#ifndef ACTOR_COLLISION_H
#define ACTOR_COLLISION_H

// collision types

/*
#define SPHERE 1
#define AABB 2
#define BOX 3
#define PLANE 4
#define RAY 5
#define CAPSULE 6
#define TERRAIN 7
#define MESH 8
*/

// structures

typedef struct
{
    float body_radius;
    float body_height;
    // float sword_radius;
    // float sword_lenght;
    // float shield_radius;
} ActorColliderSettings;

typedef struct
{
    Capsule body;
    // Capsule sword;
    // Sphere shield;
    ActorColliderSettings settings;
} ActorCollider;

typedef struct
{
    Vector3 axis_closest_to_point; // closest point in the capsule axis to the point of contact
    Vector3 velocity_penetration;  // penetration vector in the direction of the velocity
    float slope;                   // angle of inclination of the the plane of contact
    float angle_of_incidence;      // angle between the velocity vector and the plane of contact
    float displacement;            // distance from the origin to the plane of contact
    float ground_distance;         // vertical distance from the actor's position to the nearest plane of contact
    ContactData data;
} ActorContactData;

void actorCollider_init(ActorCollider *collider)
{
    collider->body.radius = collider->settings.body_radius;
    collider->body.length = collider->settings.body_height;
}

void actorCollider_setVertical(ActorCollider *collider, Vector3 *position)
{
    capsule_setVertical(&collider->body, position);
}

void actorCollider_set(ActorCollider *collider, Vector3 *position, Vector3 *rotation)
{
}

void actorContactData_clear(ActorContactData *contact)
{
    contact->axis_closest_to_point = (Vector3){0.0f, 0.0f, 0.0f};
    contact->velocity_penetration = (Vector3){0.0f, 0.0f, 0.0f};
    contact->slope = 1000.0f; // Set the slope to an out of range value to indicate no contact
    contact->displacement = 0.0f;
    contact->ground_distance = 1000.0f;
    contactData_init(&contact->data);
}

void actorContactData_setAxisClosestToPoint(ActorContactData *contact, const ActorCollider *collider)
{
    contact->axis_closest_to_point = segment_closestToPoint(&collider->body.start, &collider->body.end, &contact->data.point);
}

void actorContactData_setSlope(ActorContactData *contact)
{
    float magnitude = vector3_magnitude(&contact->data.normal);
    if (magnitude > 0)
    {
        float cos_slope = contact->data.normal.z / magnitude; // Calculate the cosine of the angle between the normal and the z-axis
        float slope = acosf(cos_slope);
        contact->slope = deg(slope);
    }
}

void actorContactData_setAngleOfIncidence(ActorContactData *contact, const Vector3 *velocity)
{
    contact->angle_of_incidence = -deg((M_PI * 0.5f) - acosf(vector3_returnDotProduct(velocity, &contact->data.normal) / vector3_magnitude(velocity)));
}

void actorContactData_setDisplacement(ActorContactData *contact)
{
    contact->displacement = vector3_returnDotProduct(&contact->data.point, &contact->data.normal);
}

void actorCollision_setGroundDistance(ActorContactData *contact, Vector3 *position)
{
    if (contact->data.normal.z == 0.0f)
        contact->ground_distance = 1000.0; // arbitrary large value to indicate no grounding
    else
        contact->ground_distance = (contact->displacement - vector3_returnDotProduct(position, &contact->data.normal)) / -contact->data.normal.z;
}

bool actorCollision_contactSphere(const ActorCollider *collider, const Sphere *sphere)
{
    return capsule_contactSphere(&collider->body, sphere);
}

void actorCollision_contactSphereSetData(ActorContactData *contact, const ActorCollider *collider, const Sphere *sphere)
{
    capsule_contactSphereSetData(&contact->data, &collider->body, sphere);
    actorContactData_setSlope(contact);
    actorContactData_setDisplacement(contact);
    actorContactData_setAxisClosestToPoint(contact, collider);
}

bool actorCollision_contactAABB(const ActorCollider *collider, const AABB *aabb)
{
    return capsule_contactAABB(&collider->body, aabb);
}

void actorCollision_contactAABBsetData(ActorContactData *contact, const ActorCollider *collider, const AABB *aabb)
{
    capsule_contactAABBSetData(&contact->data, &collider->body, aabb);
    actorContactData_setSlope(contact);
    actorContactData_setDisplacement(contact);
    actorContactData_setAxisClosestToPoint(contact, collider);
}

bool actorCollision_contactBox(const ActorCollider *collider, const Box *box)
{
    return capsule_contactBox(&collider->body, box);
}

void actorCollision_contactBoxSetData(ActorContactData *contact, const ActorCollider *collider, const Box *box)
{
    capsule_contactBoxSetData(&contact->data, &collider->body, box);
    actorContactData_setSlope(contact);
    actorContactData_setDisplacement(contact);
    actorContactData_setAxisClosestToPoint(contact, collider);
}

bool actorCollision_contactPlane(const ActorCollider *collider, const Plane *plane)
{
    return capsule_contactPlane(&collider->body, plane);
}

void actorCollision_contactPlaneSetData(ActorContactData *contact, const ActorCollider *collider, const Plane *plane)
{
    capsule_contactPlaneSetData(&contact->data, &collider->body, plane);
    actorContactData_setSlope(contact);
    contact->displacement = plane->displacement;
    actorContactData_setAxisClosestToPoint(contact, collider);
}

bool actorCollision_intersectionRay(const ActorCollider *collider, const Ray *ray)
{
    return capsule_intersectionRay(&collider->body, ray);
}

#endif
