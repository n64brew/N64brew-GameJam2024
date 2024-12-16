#ifndef PLANE_H
#define PLANE_H

// structures

typedef struct
{
    Vector3 normal;     // Normal vector of the plane
    float displacement; // Displacement from the origin along the normal
} Plane;

// function prototypes

Vector3 plane_getNormalFromRotation(const Vector3 *rotation);
float plane_getDisplacement(const Vector3 *normal, const Vector3 *point);
void plane_setFromRotationAndPoint(Plane *plane, const Vector3 *rotation, const Vector3 *point);
void plane_setFromNormalAndPoint(Plane *plane, const Vector3 *normal, const Vector3 *point);
float plane_distanceToPoint(const Plane *plane, const Vector3 *point);

// function implementations

Vector3 plane_getNormalFromRotation(const Vector3 *rotation)
{
    Vector3 normal = {0.0f, 0.0f, 1.0f};
    Vector3 rad_rotation = vector3_degToRad(rotation);
    Quaternion q_rotation = quaternion_getFromVector(&rad_rotation);
    normal = vector3_rotateByQuaternion(&normal, &q_rotation);

    return normal;
}

float plane_getDisplacement(const Vector3 *normal, const Vector3 *point)
{

    return vector3_returnDotProduct(normal, point);
}

void plane_setFromRotationAndPoint(Plane *plane, const Vector3 *rotation, const Vector3 *point)
{
    plane->normal = plane_getNormalFromRotation(rotation);
    plane->displacement = plane_getDisplacement(&plane->normal, point);
}

void plane_setFromNormalAndPoint(Plane *plane, const Vector3 *normal, const Vector3 *point)
{
    plane->normal = *normal;
    plane->displacement = plane_getDisplacement(normal, point);
}

float plane_distanceToPoint(const Plane *plane, const Vector3 *point)
{
    return vector3_returnDotProduct(&plane->normal, point) - plane->displacement;
}

bool plane_contactSphere(const Plane *plane, const Sphere *sphere)
{
    // Project the center of the sphere onto the plane's normal vector
    float distance = vector3_returnDotProduct(&plane->normal, &sphere->center) - plane->displacement;

    // Check if the distance is less than the radius of the sphere
    return fabsf(distance) <= sphere->radius;
}

void plane_contactSphereGetData(ContactData *contact, const Plane *plane, const Sphere *sphere)
{
    // Project the center of the sphere onto the plane's normal vector
    float distance = vector3_returnDotProduct(&plane->normal, &sphere->center) - plane->displacement;

    contact->normal = plane->normal;

    // Calculate the contact point on the plane
    Vector3 scaled_normal = vector3_returnScaled(&plane->normal, distance);
    contact->point = vector3_difference(&sphere->center, &scaled_normal);
}

#endif
