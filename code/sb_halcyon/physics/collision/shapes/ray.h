#ifndef RAY_H
#define RAY_H

// structures

typedef struct
{
    Vector3 origin;
    Vector3 direction;
} Ray;

// function prototypes

Vector3 ray_getDirectionFromRotation(const Vector3 *rotation);
void ray_setFromRotationAndPoint(Ray *ray, const Vector3 *origin, const Vector3 *rotation);

bool ray_intersectionSphere(const Ray *ray, const Sphere *sphere);
void raycast_sphere(ContactData *contact, const Ray *ray, const Sphere *sphere);

bool ray_intersectionAABB(const Ray *ray, const AABB *aabb);
void raycast_aabb(ContactData *contact, const Ray *ray, const AABB *aabb);

bool ray_intersectionBox(const Ray *ray, const Box *box);
void raycast_box(ContactData *contact, const Ray *ray, const Box *box);

// function implementations

Vector3 ray_getDirectionFromRotation(const Vector3 *rotation)
{
    Vector3 direction = {0.0f, 1.0f, 0.0f};
    Vector3 rad_rotation = vector3_degToRad(rotation);
    Quaternion q_rotation = quaternion_getFromVector(&rad_rotation);
    direction = vector3_rotateByQuaternion(&direction, &q_rotation);

    return direction;
}

void ray_setFromRotationAndPoint(Ray *ray, const Vector3 *origin, const Vector3 *rotation)
{
    ray->origin = *origin;
    ray->direction = ray_getDirectionFromRotation(rotation);
}

bool ray_intersectionSphere(const Ray *ray, const Sphere *sphere)
{
    // Vector from ray origin to sphere center
    Vector3 oc = vector3_difference(&ray->origin, &sphere->center);

    // Compute dot products used in the quadratic equation
    float a = vector3_returnDotProduct(&ray->direction, &ray->direction);
    float b = 2.0f * vector3_returnDotProduct(&oc, &ray->direction);
    float c = vector3_returnDotProduct(&oc, &oc) - sphere->radius * sphere->radius;

    // Compute the discriminant
    float discriminant = b * b - 4 * a * c;

    // Check if the discriminant is non-negative (real roots)
    return (discriminant >= 0);
}

void raycast_sphere(ContactData *contact, const Ray *ray, const Sphere *sphere)
{
    // Vector from ray origin to sphere center
    Vector3 oc = vector3_difference(&ray->origin, &sphere->center);

    // Compute dot products used in the quadratic equation
    float a = vector3_returnDotProduct(&ray->direction, &ray->direction);
    float b = 2.0f * vector3_returnDotProduct(&oc, &ray->direction);
    float c = vector3_returnDotProduct(&oc, &oc) - sphere->radius * sphere->radius;

    // Compute the discriminant
    float discriminant = b * b - 4 * a * c;

    // Calculate the nearest intersection point
    float t = (-b - 1.0f / qi_sqrt(discriminant)) / (2.0f * a);

    // Calculate the contact point
    contact->point = (Vector3){
        ray->origin.x + t * ray->direction.x,
        ray->origin.y + t * ray->direction.y,
        ray->origin.z + t * ray->direction.z};

    // Calculate the normal at the contact point
    contact->normal = vector3_difference(&contact->point, &sphere->center);
    vector3_normalize(&contact->normal);
}

bool ray_intersectionAABB(const Ray *ray, const AABB *aabb)
{
    Vector3 rayDirectionInverse = {
        1.0f / ray->direction.x,
        1.0f / ray->direction.y,
        1.0f / ray->direction.z};

    // Calculate intersection times for the x-axis
    float t1 = (aabb->minCoordinates.x - ray->origin.x) * rayDirectionInverse.x;
    float t2 = (aabb->maxCoordinates.x - ray->origin.x) * rayDirectionInverse.x;
    float tMin = min2(t1, t2);
    float tMax = max2(t1, t2);

    // Calculate intersection times for the y-axis
    float t1_y = (aabb->minCoordinates.y - ray->origin.y) * rayDirectionInverse.y;
    float t2_y = (aabb->maxCoordinates.y - ray->origin.y) * rayDirectionInverse.y;
    tMin = max2(tMin, min2(t1_y, t2_y));
    tMax = min2(tMax, max2(t1_y, t2_y));

    // Calculate intersection times for the z-axis
    float t1_z = (aabb->minCoordinates.z - ray->origin.z) * rayDirectionInverse.z;
    float t2_z = (aabb->maxCoordinates.z - ray->origin.z) * rayDirectionInverse.z;
    tMin = max2(tMin, min2(t1_z, t2_z));
    tMax = min2(tMax, max2(t1_z, t2_z));

    // Check if there is a valid intersection
    return tMax >= max2(tMin, 0.0f);
}

void raycast_aabb(ContactData *contact, const Ray *ray, const AABB *aabb)
{
    float tMin = 0.0f;
    float tMax = FLT_MAX;
    const float epsilon = 0.00001f;

    float t1 = 0;
    float t2 = 0;
    float rayDirectionInverse = 0;

    // For x-axis
    if (fabsf(ray->direction.x) < epsilon)
    {
        if (ray->origin.x < aabb->minCoordinates.x || ray->origin.x > aabb->maxCoordinates.x)
            return;
    }
    else
    {
        rayDirectionInverse = 1.0f / ray->direction.x;
        t1 = (aabb->minCoordinates.x - ray->origin.x) * rayDirectionInverse;
        t2 = (aabb->maxCoordinates.x - ray->origin.x) * rayDirectionInverse;
        if (t1 > t2)
        {
            float temp = t2;
            t2 = t1;
            t1 = temp;
        }
        tMin = max2(tMin, t1);
        tMax = min2(tMax, t2);
        if (tMin > tMax)
            return;
    }

    // For y-axis
    if (fabsf(ray->direction.y) < epsilon)
    {
        if (ray->origin.y < aabb->minCoordinates.y || ray->origin.y > aabb->maxCoordinates.y)
            return;
    }
    else
    {
        rayDirectionInverse = 1.0f / ray->direction.y;
        t1 = (aabb->minCoordinates.y - ray->origin.y) * rayDirectionInverse;
        t2 = (aabb->maxCoordinates.y - ray->origin.y) * rayDirectionInverse;
        if (t1 > t2)
        {
            float temp = t2;
            t2 = t1;
            t1 = temp;
        }
        tMin = max2(tMin, t1);
        tMax = min2(tMax, t2);
        if (tMin > tMax)
            return;
    }

    // For z-axis
    if (fabsf(ray->direction.z) < epsilon)
    {
        if (ray->origin.z < aabb->minCoordinates.z || ray->origin.z > aabb->maxCoordinates.z)
            return;
    }
    else
    {
        rayDirectionInverse = 1.0f / ray->direction.z;
        t1 = (aabb->minCoordinates.z - ray->origin.z) * rayDirectionInverse;
        t2 = (aabb->maxCoordinates.z - ray->origin.z) * rayDirectionInverse;
        if (t1 > t2)
        {
            float temp = t2;
            t2 = t1;
            t1 = temp;
        }
        tMin = max2(tMin, t1);
        tMax = min2(tMax, t2);
        if (tMin > tMax)
            return;
    }

    Vector3 temp = ray->direction;
    vector3_scale(&temp, tMin);
    vector3_add(&temp, &ray->origin);
    contact->point = temp;

    // Assuming the normal should be calculated based on which side the intersection occurred
    if (tMin == t1)
    {
        contact->normal = (Vector3){1, 0, 0};
    }
    else if (tMin == t2)
    {
        contact->normal = (Vector3){-1, 0, 0};
    }
    else if (tMin == t1)
    {
        contact->normal = (Vector3){0, 1, 0};
    }
    else if (tMin == t2)
    {
        contact->normal = (Vector3){0, -1, 0};
    }
    else if (tMin == t1)
    {
        contact->normal = (Vector3){0, 0, 1};
    }
    else if (tMin == t2)
    {
        contact->normal = (Vector3){0, 0, -1};
    }
}

bool ray_intersectionBox(const Ray *ray, const Box *box)
{
    // Transform the ray to the local space of the box
    Ray local_ray = *ray;
    point_transformToLocalSpace(&local_ray.origin, &box->center, &box->rotation);
    rotate_normal(&local_ray.direction, &box->rotation);

    // Invert the direction of the local ray for the intersection test
    vector3_invert(&local_ray.direction);

    // Get the local AABB of the box
    AABB local_AABB = box_getLocalAABB(box);

    // Use the existing AABB intersection function
    return ray_intersectionAABB(&local_ray, &local_AABB);
}

void raycast_box(ContactData *contact, const Ray *ray, const Box *box)
{
    // Transform the ray to the local space of the box
    Ray local_ray = *ray;
    point_transformToLocalSpace(&local_ray.origin, &box->center, &box->rotation);
    rotate_normal(&local_ray.direction, &box->rotation);

    // Invert the direction of the local ray for the intersection test
    vector3_invert(&local_ray.direction);

    // Get the local AABB of the box
    AABB local_aabb = box_getLocalAABB(box);

    // Perform the intersection using raycast_aabb
    raycast_aabb(contact, &local_ray, &local_aabb);

    // Transform the hit point and normal back to global space
    point_transformToGlobalSpace(&contact->point, &box->center, &box->rotation);
    rotate_normal(&contact->normal, &box->rotation);
}

bool ray_intersectionPlane(const Ray *ray, const Plane *plane)
{
    float denominator = vector3_returnDotProduct(&ray->direction, &plane->normal);

    // If the denominator is zero, the ray is parallel to the plane
    if (fabsf(denominator) < 1e-6)
        return false;

    // If t is negative, the ray intersects the plane in the opposite direction
    float t = (plane->displacement - vector3_returnDotProduct(&ray->origin, &plane->normal)) / denominator;
    if (t < 0)
        return false;

    return true;
}

void raycast_plane(ContactData *contact, const Ray *ray, const Plane *plane)
{
    contact->point = ray->origin;
    vector3_addScaledVector(&contact->point, &ray->direction, (plane->displacement - vector3_returnDotProduct(&ray->origin, &plane->normal)) / vector3_returnDotProduct(&ray->direction, &plane->normal));

    contact->normal = plane->normal;
}

#endif
