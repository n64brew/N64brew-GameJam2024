#ifndef AABB_H
#define AABB_H

// structures

typedef struct AABB
{
    Vector3 minCoordinates;
    Vector3 maxCoordinates;
} AABB;

#define MAGIC_NUM 0

// function prototypes

void aabb_setFromCenterAndSize(AABB *aabb, const Vector3 *center, const Vector3 *size);
void aabb_getCorners(const AABB *aabb, Vector3 corners[8]);

Vector3 aabb_closestToPoint(const AABB *aabb, const Vector3 *point);
Vector3 aabb_closestToSegment(const AABB *aabb, const Vector3 *a, const Vector3 *b);

bool aabb_containsPoint(const AABB *aabb, const Vector3 *point);
bool aabb_contactAABB(const AABB *a, const AABB *b);
void aabb_contactAABBsetData(ContactData *contact, const AABB *a, const AABB *b);
bool aabb_contactSphere(const AABB *aabb, const Sphere *sphere);
void aabb_contactSphereSetData(ContactData *contact, const AABB *aabb, const Sphere *sphere);

// function implementations

void aabb_setFromCenterAndSize(AABB *aabb, const Vector3 *center, const Vector3 *size)
{
    Vector3 halfSize = vector3_returnScaled(size, 0.5f);
    aabb->minCoordinates = vector3_difference(center, &halfSize);
    aabb->maxCoordinates = vector3_sum(center, &halfSize);
}

void aabb_getCorners(const AABB *aabb, Vector3 corners[8])
{
    corners[0] = (Vector3){aabb->minCoordinates.x, aabb->minCoordinates.y, aabb->minCoordinates.z};
    corners[1] = (Vector3){aabb->maxCoordinates.x, aabb->minCoordinates.y, aabb->minCoordinates.z};
    corners[2] = (Vector3){aabb->minCoordinates.x, aabb->maxCoordinates.y, aabb->minCoordinates.z};
    corners[3] = (Vector3){aabb->maxCoordinates.x, aabb->maxCoordinates.y, aabb->minCoordinates.z};
    corners[4] = (Vector3){aabb->minCoordinates.x, aabb->minCoordinates.y, aabb->maxCoordinates.z};
    corners[5] = (Vector3){aabb->maxCoordinates.x, aabb->minCoordinates.y, aabb->maxCoordinates.z};
    corners[6] = (Vector3){aabb->minCoordinates.x, aabb->maxCoordinates.y, aabb->maxCoordinates.z};
    corners[7] = (Vector3){aabb->maxCoordinates.x, aabb->maxCoordinates.y, aabb->maxCoordinates.z};
}

Vector3 aabb_closestToPoint(const AABB *aabb, const Vector3 *point)
{
    Vector3 closest;
    closest.x = clamp(point->x, aabb->minCoordinates.x, aabb->maxCoordinates.x);
    closest.y = clamp(point->y, aabb->minCoordinates.y, aabb->maxCoordinates.y);
    closest.z = clamp(point->z, aabb->minCoordinates.z, aabb->maxCoordinates.z);
    return closest;
}

Vector3 aabb_getCenter(const AABB *aabb)
{
    Vector3 sum = vector3_sum(&aabb->minCoordinates, &aabb->maxCoordinates);
    return vector3_returnScaled(&sum, 0.5f);
}

Vector3 aabb_getHalfSize(const AABB *aabb)
{
    Vector3 difference = vector3_difference(&aabb->maxCoordinates, &aabb->minCoordinates);
    return vector3_returnScaled(&difference, 0.5f);
}

Vector3 aabb_closestToSegment(const AABB *aabb, const Vector3 *a, const Vector3 *b)
{
    Vector3 s, v, sign = {1, 1, 1};
    Vector3 aabb_center = aabb_getCenter(aabb);
    Vector3 half_size = aabb_getHalfSize(aabb);
    Vector3 tanchor = {2.0f, 2.0f, 2.0f}; // Initial large number for tanchor
    Vector3 region = {0, 0, 0};
    Vector3 v2;
    float t = 0;

    // Translate the segment's starting point relative to the AABB's center
    s = vector3_difference(a, &aabb_center);

    // Calculate the direction vector of the segment
    v = vector3_difference(b, a);

    // Mirror the line direction if necessary
    if (v.x < 0)
    {
        s.x = -s.x;
        v.x = -v.x;
        sign.x = -1;
    }
    if (v.y < 0)
    {
        s.y = -s.y;
        v.y = -v.y;
        sign.y = -1;
    }
    if (v.z < 0)
    {
        s.z = -s.z;
        v.z = -v.z;
        sign.z = -1;
    }

    // Calculate v^2 (the square of each component of v)
    v2.x = v.x * v.x;
    v2.y = v.y * v.y;
    v2.z = v.z * v.z;

    // Determine regions and tanchor values for a
    if (v.x > MAGIC_NUM)
    {
        if (s.x < -half_size.x)
        {
            region.x = -1;
            tanchor.x = (-half_size.x - s.x) / v.x;
        }
        else if (s.x > half_size.x)
        {
            region.x = 1;
            tanchor.x = (half_size.x - s.x) / v.x;
        }
    }
    else
    {
        region.x = 0;
        tanchor.x = 2; // this will never be a valid tanchor
    }

    if (v.y > MAGIC_NUM)
    {
        if (s.y < -half_size.y)
        {
            region.y = -1;
            tanchor.y = (-half_size.y - s.y) / v.y;
        }
        else if (s.y > half_size.y)
        {
            region.y = 1;
            tanchor.y = (half_size.y - s.y) / v.y;
        }
    }
    else
    {
        region.y = 0;
        tanchor.y = 2; // this will never be a valid tanchor
    }
    if (v.z > MAGIC_NUM)
    {
        if (s.z < -half_size.z)
        {
            region.z = -1;
            tanchor.z = (-half_size.z - s.z) / v.z;
        }
        else if (s.z > half_size.z)
        {
            region.z = 1;
            tanchor.z = (half_size.z - s.z) / v.z;
        }
    }
    else
    {
        region.z = 0;
        tanchor.z = 2; // this will never be a valid tanchor
    }

    // Check the initial dd2dt
    float dd2dt = 0;
    if (region.x != 0)
        dd2dt -= v2.x * tanchor.x;
    if (region.y != 0)
        dd2dt -= v2.y * tanchor.y;
    if (region.z != 0)
        dd2dt -= v2.z * tanchor.z;
    if (dd2dt >= 0)
        goto finalize;

    // Iterate to find the smallest t
    float next_t, next_dd2dt;

    do
    {

        // find the point on the line that is at the next clip plane boundary
        next_t = 1.0f;
        next_t = (tanchor.x > t && tanchor.x < 1 && tanchor.x < next_t) ? tanchor.x : next_t;
        next_t = (tanchor.y > t && tanchor.y < 1 && tanchor.y < next_t) ? tanchor.y : next_t;
        next_t = (tanchor.z > t && tanchor.z < 1 && tanchor.z < next_t) ? tanchor.z : next_t;

        // compute d|d|^2/dt for the next t
        next_dd2dt = 0;
        next_dd2dt += (region.x ? v2.x : 0) * (next_t - tanchor.x);
        next_dd2dt += (region.y ? v2.y : 0) * (next_t - tanchor.y);
        next_dd2dt += (region.z ? v2.z : 0) * (next_t - tanchor.z);

        // if the sign of d|d|^2/dt has changed, solution = the crossover point
        if (next_dd2dt >= 0)
        {
            float m = (next_dd2dt - dd2dt) / (next_t - t);
            t -= dd2dt / m;
            goto finalize;
        }

        // advance to the next anchor point / region
        if (tanchor.x == next_t)
        {
            tanchor.x = (half_size.x - s.x) / v.x;
            region.x++;
        }
        if (tanchor.y == next_t)
        {
            tanchor.y = (half_size.y - s.y) / v.y;
            region.y++;
        }
        if (tanchor.z == next_t)
        {
            tanchor.z = (half_size.z - s.z) / v.z;
            region.z++;
        }

        t = next_t;
        dd2dt = next_dd2dt;

    } while (t < 1.0f);

    t = 1.0f;

finalize:

    // Compute the closest point on the box
    Vector3 tmp = {sign.x * (s.x + t * v.x), sign.y * (s.y + t * v.y), sign.z * (s.z + t * v.z)};

    // Clamp tmp to the AABB's extents
    if (tmp.x < -half_size.x)
        tmp.x = -half_size.x;
    else if (tmp.x > half_size.x)
        tmp.x = half_size.x;

    if (tmp.y < -half_size.y)
        tmp.y = -half_size.y;
    else if (tmp.y > half_size.y)
        tmp.y = half_size.y;

    if (tmp.z < -half_size.z)
        tmp.z = -half_size.z;
    else if (tmp.z > half_size.z)
        tmp.z = half_size.z;

    return (Vector3){tmp.x + aabb_center.x, tmp.y + aabb_center.y, tmp.z + aabb_center.z};
}

bool aabb_containsPoint(const AABB *aabb, const Vector3 *point)
{
    return (point->x >= aabb->minCoordinates.x && point->x <= aabb->maxCoordinates.x &&
            point->y >= aabb->minCoordinates.y && point->y <= aabb->maxCoordinates.y &&
            point->z >= aabb->minCoordinates.z && point->z <= aabb->maxCoordinates.z);
}

bool aabb_contactAABB(const AABB *a, const AABB *b)
{
    if (a->maxCoordinates.x < b->minCoordinates.x || b->maxCoordinates.x < a->minCoordinates.x)
        return false;
    if (a->maxCoordinates.y < b->minCoordinates.y || b->maxCoordinates.y < a->minCoordinates.y)
        return false;
    if (a->maxCoordinates.z < b->minCoordinates.z || b->maxCoordinates.z < a->minCoordinates.z)
        return false;
    return true;
}

void aabb_contactAABBsetData(ContactData *contact, const AABB *a, const AABB *b)
{
    // Calculate overlap on each axis
    float overlapX = min2(a->maxCoordinates.x, b->maxCoordinates.x) - max2(a->minCoordinates.x, b->minCoordinates.x);
    float overlapY = min2(a->maxCoordinates.y, b->maxCoordinates.y) - max2(a->minCoordinates.y, b->minCoordinates.y);
    float overlapZ = min2(a->maxCoordinates.z, b->maxCoordinates.z) - max2(a->minCoordinates.z, b->minCoordinates.z);

    // Find the axis of least penetration
    if (overlapX < overlapY && overlapX < overlapZ)
    {
        contact->normal = (Vector3){a->maxCoordinates.x < b->maxCoordinates.x ? -1 : 1, 0, 0};
    }
    else if (overlapY < overlapX && overlapY < overlapZ)
    {
        contact->normal = (Vector3){0, a->maxCoordinates.y < b->maxCoordinates.y ? -1 : 1, 0};
    }
    else
    {
        contact->normal = (Vector3){0, 0, a->maxCoordinates.z < b->maxCoordinates.z ? -1 : 1};
    }

    // Calculate the contact point
    contact->point = (Vector3){
        (max2(a->minCoordinates.x, b->minCoordinates.x) + min2(a->maxCoordinates.x, b->maxCoordinates.x)) / 2.0f,
        (max2(a->minCoordinates.y, b->minCoordinates.y) + min2(a->maxCoordinates.y, b->maxCoordinates.y)) / 2.0f,
        (max2(a->minCoordinates.z, b->minCoordinates.z) + min2(a->maxCoordinates.z, b->maxCoordinates.z)) / 2.0f};
}

bool aabb_contactSphere(const AABB *aabb, const Sphere *sphere)
{
    // Find the point on the AABB closest to the center of the sphere
    Vector3 closestPoint = aabb_closestToPoint(aabb, &sphere->center);

    // Calculate the distance from the closest point to the center of the sphere
    Vector3 difference = vector3_difference(&closestPoint, &sphere->center);
    float distanceSquared = vector3_squaredMagnitude(&difference);

    // Check if the distance is less than or equal to the radius squared
    return distanceSquared <= (sphere->radius * sphere->radius);
}

void aabb_contactSphereSetData(ContactData *contact, const AABB *aabb, const Sphere *sphere)
{
    // the contact point is the point on the AABB closest to the center of the sphere
    contact->point = aabb_closestToPoint(aabb, &sphere->center);

    // Calculate the vector from the sphere center to the closest point on the AABB
    contact->normal = vector3_difference(&sphere->center, &contact->point);
    vector3_normalize(&contact->normal);
}

#endif