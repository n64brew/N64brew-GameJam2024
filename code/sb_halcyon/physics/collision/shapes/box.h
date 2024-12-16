#ifndef BOX_H
#define BOX_H

// structures

typedef struct
{
    Vector3 size;
    Vector3 center;
    Vector3 rotation;
} Box;

// function prototypes

AABB box_getLocalAABB(const Box *box);

bool box_contactSphere(const Box *box, const Sphere *sphere);
void box_contactSphereSetData(ContactData *contact, const Box *box, const Sphere *sphere);

void box_init(Box *box, Vector3 size, Vector3 center, Vector3 rotation, float scalar);

// function implementations

AABB box_getLocalAABB(const Box *box)
{
    AABB aabb;
    aabb.minCoordinates = vector3_returnScaled(&box->size, -0.5f);
    aabb.maxCoordinates = vector3_returnScaled(&box->size, 0.5f);
    return aabb;
}

bool box_contactSphere(const Box *box, const Sphere *sphere)
{
    // Transform the center of the sphere to the local space of the box
    Vector3 local_sphere_center = sphere->center;
    point_transformToLocalSpace(&local_sphere_center, &box->center, &box->rotation);

    // Get the local AABB of the box and local sphere
    AABB local_aabb = box_getLocalAABB(box);
    const Sphere local_sphere = {
        center : local_sphere_center,
        radius : sphere->radius
    };

    // Check for collision in the local space
    return aabb_contactSphere(&local_aabb, &local_sphere);
}

void box_contactSphereSetData(ContactData *contact, const Box *box, const Sphere *sphere)
{
    // Transform the center of the sphere to the local space of the box
    Vector3 local_sphere_center = sphere->center;
    point_transformToLocalSpace(&local_sphere_center, &box->center, &box->rotation);

    // Get the local AABB of the box
    AABB local_aabb = box_getLocalAABB(box);

    // Find the point on the AABB closest to the center of the sphere
    contact->point = aabb_closestToPoint(&local_aabb, &local_sphere_center);

    // Calculate the normal in local space pointing from the AABB towards the sphere
    contact->normal = vector3_difference(&local_sphere_center, &contact->point);
    vector3_normalize(&contact->normal);

    // Transform the closest point and normal back to global space
    point_transformToGlobalSpace(&contact->point, &box->center, &box->rotation);
    rotate_normal(&contact->normal, &box->rotation); // Rotate normal to global space
}

void box_init(Box *box, Vector3 size, Vector3 center, Vector3 rotation, float scalar)
{
    box->size = vector3_returnScaled(&size, scalar);
    box->center = vector3_returnScaled(&center, scalar);
    box->rotation = rotation;
}

#endif
