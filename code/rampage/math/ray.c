#include "ray.h"

float rayDetermineDistance(struct Ray* ray, struct Vector3* point) {
    struct Vector3 relative;
    vector3Sub(point, &ray->origin, &relative);
    return vector3Dot(&relative, &ray->dir);
}
