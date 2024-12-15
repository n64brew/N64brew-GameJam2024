#ifndef SPHERE_H
#define SPHERE_H

// structures

typedef struct
{
    Vector3 center;
    float radius;
} Sphere;

bool sphere_contactSphere(const Sphere *s, const Sphere *t);
void sphere_collisionTestSphere(ContactData *contact, const Sphere *s, const Sphere *t);

bool sphere_contactSphere(const Sphere *s, const Sphere *t)
{
    float radiusSum = s->radius + t->radius;

    Vector3 diff = s->center;
    vector3_subtract(&diff, &t->center);
    // Check if the squared distance is less than or equal to the square of the sum of the radii
    return vector3_squaredMagnitude(&diff) <= radiusSum * radiusSum;
}

void sphere_collisionTestSphere(ContactData *contact, const Sphere *s, const Sphere *t)
{
    // Calculate the normal pointing towards the first sphere
    contact->normal = vector3_difference(&t->center, &s->center);
    vector3_normalize(&contact->normal);

    // Calculate the contact point in reference to the second sphere
    contact->point = (Vector3){
        t->center.x - contact->normal.x * t->radius,
        t->center.y - contact->normal.y * t->radius,
        t->center.z - contact->normal.z * t->radius};
}

#endif