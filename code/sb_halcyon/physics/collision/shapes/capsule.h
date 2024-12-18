#ifndef CAPSULE_H
#define CAPSULE_H

typedef struct
{
    Vector3 start;
    Vector3 end;
    float radius;
    float length;
} Capsule;

// Function prototypes

void capsule_setVertical(Capsule *capsule, const Vector3 *position);

bool capsule_contactSphere(const Capsule *capsule, const Sphere *sphere);
void capsule_contactSphereSetData(ContactData *contact, const Capsule *capsule, const Sphere *sphere);

bool capsule_contactAABB(const Capsule *capsule, const AABB *aabb);
void capsule_contactAABBSetData(ContactData *contact, const Capsule *capsule, const AABB *aabb);

bool capsule_contactBox(const Capsule *capsule, const Box *box);
void capsule_contactBoxSetData(ContactData *contact, const Capsule *capsule, const Box *box);

bool capsule_contactPlane(const Capsule *capsule, const Plane *plane);

bool capsule_intersectionRay(const Capsule *capsule, const Ray *ray);

// Function implementations

void capsule_setVertical(Capsule *capsule, const Vector3 *position)
{
    capsule->start = *position;
    capsule->start.z += capsule->radius;
    capsule->end = *position;
    capsule->end.z = capsule->end.z + capsule->length - capsule->radius;
}

bool capsule_contactSphere(const Capsule *capsule, const Sphere *sphere)
{
    // Calculate the closest point on the capsule segment to the sphere center
    Vector3 closest_on_axis = segment_closestToPoint(&capsule->start, &capsule->end, &sphere->center);

    // Calculate the squared distance from the closest point to the sphere center
    Vector3 difference = vector3_difference(&closest_on_axis, &sphere->center);
    float distanceSquared = vector3_squaredMagnitude(&difference);

    // Compare the squared distance with the squared combined radius
    float combinedRadius = capsule->radius + sphere->radius;
    return distanceSquared <= combinedRadius * combinedRadius;
}

void capsule_contactSphereSetData(ContactData *contact, const Capsule *capsule, const Sphere *sphere)
{
    // Calculate the closest point on the capsule segment to the sphere center
    Vector3 closest_on_axis = segment_closestToPoint(&capsule->start, &capsule->end, &sphere->center);

    // Calculate the squared distance from the closest point to the sphere center
    contact->normal = vector3_difference(&closest_on_axis, &sphere->center);
    contact->penetration = capsule->radius + sphere->radius - fabsf(vector3_magnitude(&contact->normal));
    vector3_normalize(&contact->normal);

    // Calculate the contact point in reference to the sphere
    contact->point = sphere->center;
    vector3_addScaledVector(&contact->point, &contact->normal, sphere->radius);
}

bool capsule_contactAABB(const Capsule *capsule, const AABB *aabb)
{
    Vector3 closest_point_on_aabb = aabb_closestToSegment(aabb, &capsule->end, &capsule->start);
    Vector3 closest_point_on_axis = segment_closestToPoint(&capsule->end, &capsule->start, &closest_point_on_aabb);
    Vector3 distance_vector = vector3_difference(&closest_point_on_axis, &closest_point_on_aabb);

    return vector3_squaredMagnitude(&distance_vector) <= capsule->radius * capsule->radius;
}

void capsule_contactAABBSetData(ContactData *contact, const Capsule *capsule, const AABB *aabb)
{
    contact->point = aabb_closestToSegment(aabb, &capsule->end, &capsule->start);

    Vector3 closest_point_on_axis = segment_closestToPoint(&capsule->end, &capsule->start, &contact->point);
    Vector3 distance_vector = vector3_difference(&closest_point_on_axis, &contact->point);
    contact->penetration = capsule->radius - fabsf(vector3_magnitude(&distance_vector));
    contact->normal = distance_vector;
    vector3_normalize(&contact->normal);
}

bool capsule_contactBox(const Capsule *capsule, const Box *box)
{
    Capsule local_capsule = *capsule;
    point_transformToLocalSpace(&local_capsule.start, &box->center, &box->rotation);
    point_transformToLocalSpace(&local_capsule.end, &box->center, &box->rotation);
    AABB local_aabb = box_getLocalAABB(box);
    return capsule_contactAABB(&local_capsule, &local_aabb);
}

void capsule_contactBoxSetData(ContactData *contact, const Capsule *capsule, const Box *box)
{
    Capsule local_capsule = *capsule;
    point_transformToLocalSpace(&local_capsule.start, &box->center, &box->rotation);
    point_transformToLocalSpace(&local_capsule.end, &box->center, &box->rotation);
    AABB local_aabb = box_getLocalAABB(box);

    capsule_contactAABBSetData(contact, &local_capsule, &local_aabb);
    point_transformToGlobalSpace(&contact->point, &box->center, &box->rotation);
    rotate_normal(&contact->normal, &box->rotation);
}

bool capsule_contactPlane(const Capsule *capsule, const Plane *plane)
{
    // Calculate the distances of the capsule endpoints from the plane
    float distance_to_start = plane_distanceToPoint(plane, &capsule->start);
    float distance_to_end = plane_distanceToPoint(plane, &capsule->end);

    // Check if either endpoint of the capsule is within the radius distance from the plane
    if (fabsf(distance_to_start) <= capsule->radius || fabsf(distance_to_end) <= capsule->radius)
    {
        return true;
    }

    // Check if the plane intersects the segment of the capsule
    if ((distance_to_start > 0 && distance_to_end < 0) || (distance_to_start < 0 && distance_to_end > 0))
    {
        return true;
    }

    return false;
}

void capsule_contactPlaneSetData(ContactData *contact, const Capsule *capsule, const Plane *plane)
{
    // Calculate the distances of the capsule endpoints from the plane
    float distance_to_start = plane_distanceToPoint(plane, &capsule->start);
    float distance_to_end = plane_distanceToPoint(plane, &capsule->end);

    // Set the contact normal as the plane's normal
    contact->normal = plane->normal;

    // Determine the point of contact and penetration depth
    if (fabsf(distance_to_start) <= capsule->radius)
    {
        // The start point of the capsule is within the radius distance from the plane
        contact->penetration = capsule->radius - fabsf(distance_to_start);
        contact->point = capsule->start;
        vector3_addScaledVector(&contact->point, &contact->normal, -distance_to_start);
    }
    else if (fabsf(distance_to_end) <= capsule->radius)
    {
        // The end point of the capsule is within the radius distance from the plane
        contact->penetration = capsule->radius - fabsf(distance_to_end);
        contact->point = capsule->end;
        vector3_addScaledVector(&contact->point, &contact->normal, -distance_to_end);
    }
}

bool capsule_intersectionRay(const Capsule *capsule, const Ray *ray)
{
    // Calculate the vector from the start to the end of the capsule
    Vector3 ba = vector3_difference(&capsule->end, &capsule->start);
    // Calculate the vector from the start of the capsule to the origin of the ray
    Vector3 oa = vector3_difference(&ray->origin, &capsule->start);

    // Compute dot products used in the quadratic equation
    float baba = vector3_returnDotProduct(&ba, &ba);
    float bard = vector3_returnDotProduct(&ba, &ray->direction);
    float baoa = vector3_returnDotProduct(&ba, &oa);
    float rdoa = vector3_returnDotProduct(&ray->direction, &oa);
    float oaoa = vector3_returnDotProduct(&oa, &oa);

    // Compute the coefficients of the quadratic equation
    float a = baba - bard * bard;
    float b = baba * rdoa - baoa * bard;
    float c = baba * oaoa - baoa * baoa - capsule->radius * capsule->radius * baba;
    float h = b * b - a * c;

    // Check if the discriminant is non-negative (real roots)
    if (h >= 0.0f && a != 0.0f)
    {
        // Compute the smallest root of the quadratic equation
        float t = (-b - sqrtf(h)) / a;
        float y = baoa + t * bard;
        // Check if the intersection is within the cylindrical body of the capsule
        if (y > 0.0f && y < baba)
            return true;
        // Otherwise, check for intersections with the spherical caps
        Vector3 oc = (y <= 0.0f) ? oa : vector3_difference(&ray->origin, &capsule->end);
        b = vector3_returnDotProduct(&ray->direction, &oc);
        c = vector3_returnDotProduct(&oc, &oc) - capsule->radius * capsule->radius;
        h = b * b - c;
        // Check if the intersection with the cap is valid
        if (h > 0.0f)
            return true;
    }
    // Return false if no intersection is found
    return false;
}

bool capsule_intersectsEdge(const Capsule *capsule, const Vector3 *edgeStart, const Vector3 *edgeEnd)
{
    // Step 1: Find the closest point on the capsule's segment to the edge
    Vector3 closestPointOnCapsule = segment_closestToPoint(&capsule->start, &capsule->end, edgeStart);
    Vector3 closestPointOnEdge = segment_closestToPoint(edgeStart, edgeEnd, &capsule->start);

    // Step 2: Calculate the distance vector between these two closest points
    Vector3 distanceVector = vector3_difference(&closestPointOnCapsule, &closestPointOnEdge);

    // Step 3: Calculate the squared distance
    float distanceSquared = vector3_squaredMagnitude(&distanceVector);

    // Step 4: Calculate the combined radius
    float combinedRadius = capsule->radius; // Adjust if the edge has its own thickness

    // Step 5: Check for intersection
    return distanceSquared <= combinedRadius * combinedRadius;
}

/*
 */

#endif