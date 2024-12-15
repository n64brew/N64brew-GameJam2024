#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

// function prototypes

Vector3 vector3_multiplyByMatrix3x3(const Matrix3x3 *matrix, const Vector3 *vector);
Vector3 vector3_rotateByQuaternion(const Vector3 *v, const Quaternion *q);

Vector3 vector3_transformToLocalSpace(const Vector3 *global_point, Vector3 local_center, Vector3 rotation);
Vector3 vector3_transformToGlobalSpace(const Vector3 *local_point, Vector3 local_center, Vector3 rotation);

Vector3 vector3_reflect(const Vector3 *vector, const Vector3 *normal);

Vector3 vector3_degToRad(const Vector3 *rotation);
Vector3 vector3_clamp(const Vector3 *vector, float maxLength);

bool vector3_areParallel(const Vector3 *vector1, const Vector3 *vector2);
bool vector3_areOrthogonal(const Vector3 *vector1, const Vector3 *vector2);

void point_rotateZYX(Vector3 *point, const Vector3 *rotation);
void point_rotateXYZ(Vector3 *point, const Vector3 *rotation);
void point_transformToLocalSpace(Vector3 *global_point, const Vector3 *local_center, const Vector3 *local_rotation);
void point_transformToGlobalSpace(Vector3 *local_point, const Vector3 *local_center, const Vector3 *local_rotation);

Vector3 segment_closestToPoint(const Vector3 *seg_a, const Vector3 *seg_b, const Vector3 *point_c);
void segment_closestPointsWithSegment(const Vector3 *seg1_a, const Vector3 *seg1_b, const Vector3 *seg2_a, const Vector3 *seg2_b, Vector3 *closest_seg1, Vector3 *closest_seg2);
float segment_distanceToPoint(const Vector3 *a, const Vector3 *b, const Vector3 *p);

float line_distanceToPoint(const Vector3 *linePointA, const Vector3 *linePointB, const Vector3 *point);

float plane_intersectionWithSegment(const Vector3 *a, const Vector3 *b, float plane_displacement, const Vector3 *planeormal);

void triangle_getBarycentricCoordinates(const Vector3 *a, const Vector3 *b, const Vector3 *c, const Vector3 *p, float *u, float *v, float *w);

Matrix3x3 rotationMatrix_getFromEuler(const Vector3 *rotation);

void rotate_normal(Vector3 *vector, const Vector3 *rotation);

Vector3 vector3_fromQuaternion(Quaternion q);

// function implementations

inline Vector3 vector3_multiplyByMatrix3x3(const Matrix3x3 *matrix, const Vector3 *vector)
{
    return (Vector3){
        .x = matrix->row[0].x * vector->x + matrix->row[0].y * vector->y + matrix->row[0].z * vector->z,
        .y = matrix->row[1].x * vector->x + matrix->row[1].y * vector->y + matrix->row[1].z * vector->z,
        .z = matrix->row[2].x * vector->x + matrix->row[2].y * vector->y + matrix->row[2].z * vector->z};
}

inline Vector3 vector3_rotateByQuaternion(const Vector3 *v, const Quaternion *q)
{
    Vector3 u = {q->x, q->y, q->z};
    float s = q->w;

    Vector3 rv1 = vector3_returnScaled(&u, 2.0f * vector3_returnDotProduct(&u, v));
    Vector3 rv2 = vector3_returnScaled(v, (s * s - vector3_returnDotProduct(&u, &u)));
    Vector3 crossProduct = vector3_returnCrossProduct(&u, v);
    Vector3 rv3 = vector3_returnScaled(&crossProduct, 2.0f * s);

    Vector3 result = vector3_sum(&rv1, &rv2);

    return vector3_sum(&result, &rv3);
}

inline Vector3 vector3_transformToLocalSpace(const Vector3 *global_point, Vector3 local_center, Vector3 rotation)
{
    // Translate point by the inverse of Box's center
    Vector3 local_point = vector3_difference(global_point, &local_center);

    // Rotate the translated point by the inverse of Box's rotation
    Vector3 inverse_rotation = vector3_getInverse(&rotation);
    point_rotateXYZ(&local_point, &inverse_rotation);

    return local_point;
}

inline Vector3 vector3_transformToGlobalSpace(const Vector3 *local_point, Vector3 local_center, Vector3 rotation)
{
    Vector3 global_point = *local_point;
    // Apply rotation to the local point to get it in the global space orientation
    point_rotateZYX(&global_point, &rotation);

    // Translate the rotated point by adding the Box's center
    global_point = vector3_sum(&global_point, &local_center);

    return global_point;
}

inline Vector3 vector3_degToRad(const Vector3 *rotation)
{
    Vector3 result;
    result.x = rad(rotation->x);
    result.y = rad(rotation->y);
    result.z = rad(rotation->z);
    return result;
}

inline bool vector3_areParallel(const Vector3 *vector1, const Vector3 *vector2)
{
    Vector3 crossProduct = {
        vector1->y * vector2->z - vector1->z * vector2->y,
        vector1->z * vector2->x - vector1->x * vector2->z,
        vector1->x * vector2->y - vector1->y * vector2->x};
    return (crossProduct.x * crossProduct.x + crossProduct.y * crossProduct.y + crossProduct.z * crossProduct.z) < 0.00001f;
}

inline bool vector3_areOrthogonal(const Vector3 *vector1, const Vector3 *vector2)
{
    float dotProduct = vector1->x * vector2->x + vector1->y * vector2->y + vector1->z * vector2->z;
    return fabsf(dotProduct) < 0.001f;
}

/* clamp a vector to a maximum length */
inline Vector3 vector3_clamp(const Vector3 *vector, float maxLength)
{
    float lengthSquare = vector->x * vector->x + vector->y * vector->y + vector->z * vector->z;
    if (lengthSquare > maxLength * maxLength)
    {
        float length = sqrt(lengthSquare);
        return (Vector3){vector->x * maxLength / length, vector->y * maxLength / length, vector->z * maxLength / length};
    }
    return *vector;
}

inline Vector3 vector3_reflect(const Vector3 *vector, const Vector3 *normal)
{
    Vector3 scaled_normal = vector3_returnScaled(normal, vector3_returnDotProduct(vector, normal) * 2.0f);
    return vector3_difference(vector, &scaled_normal);
}

/* compute and return a point on segment from "seg_a" and "seg_b" that is closest to point "point_c" */
inline Vector3 segment_closestToPoint(const Vector3 *seg_a, const Vector3 *seg_b, const Vector3 *point_c)
{
    Vector3 ab = {seg_b->x - seg_a->x, seg_b->y - seg_a->y, seg_b->z - seg_a->z};
    Vector3 ac = {point_c->x - seg_a->x, point_c->y - seg_a->y, point_c->z - seg_a->z};
    float abLengthSquare = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z;

    // If the segment has almost zero length
    if (abLengthSquare < 1e-6f)
    {
        // Return one end-point of the segment as the closest point
        return *seg_a;
    }

    // Project point C onto "AB" line
    float t = (ac.x * ab.x + ac.y * ab.y + ac.z * ab.z) / abLengthSquare;

    // If projected point onto the line is outside the segment, clamp it to the segment
    if (t < 0.0f)
        t = 0.0f;
    if (t > 1.0f)
        t = 1.0f;

    // Return the closest point on the segment
    return (Vector3){seg_a->x + t * ab.x, seg_a->y + t * ab.y, seg_a->z + t * ab.z};
}

/* segment_closestPointInBetween
compute the closest points between two segments */
inline void segment_closestPointsWithSegment(const Vector3 *seg1_a, const Vector3 *seg1_b,
                                             const Vector3 *seg2_a, const Vector3 *seg2_b,
                                             Vector3 *closest_seg1, Vector3 *closest_seg2)
{

    Vector3 d1 = {seg1_b->x - seg1_a->x, seg1_b->y - seg1_a->y, seg1_b->z - seg1_a->z};
    Vector3 d2 = {seg2_b->x - seg2_a->x, seg2_b->y - seg2_a->y, seg2_b->z - seg2_a->z};
    Vector3 r = {seg1_a->x - seg2_a->x, seg1_a->y - seg2_a->y, seg1_a->z - seg2_a->z};
    float a = d1.x * d1.x + d1.y * d1.y + d1.z * d1.z;
    float e = d2.x * d2.x + d2.y * d2.y + d2.z * d2.z;
    float f = d2.x * r.x + d2.y * r.y + d2.z * r.z;
    float s, t;

    // If both segments degenerate into points
    if (a <= 1e-6f && e <= 1e-6f)
    {
        *closest_seg1 = *seg1_a;
        *closest_seg2 = *seg2_a;
        return;
    }
    if (a <= 1e-6f)
    { // If first segment degenerates into a point
        s = 0.0f;
        // Compute the closest point on second segment
        t = clamp(f / e, 0.0f, 1.0f);
    }
    else
    {
        float c = d1.x * r.x + d1.y * r.y + d1.z * r.z;
        // If the second segment degenerates into a point
        if (e <= 1e-6f)
        {
            t = 0.0f;
            s = clamp(-c / a, 0.0f, 1.0f);
        }
        else
        {
            float b = d1.x * d2.x + d1.y * d2.y + d1.z * d2.z;
            float denom = a * e - b * b;
            // If the segments are not parallel
            if (denom != 0.0f)
            {
                // Compute the closest point on line 1 to line 2 and
                // clamp to first segment.
                s = clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            }
            else
            {
                // Pick an arbitrary point on first segment
                s = 0.0f;
            }
            // Compute the point on line 2 closest to the closest point
            // we have just found
            t = (b * s + f) / e;
            // If this closest point is inside second segment (t in [0, 1]), we are done.
            // Otherwise, we clamp the point to the second segment and compute again the
            // closest point on segment 1
            if (t < 0.0f)
            {
                t = 0.0f;
                s = clamp(-c / a, 0.0f, 1.0f);
            }
            else if (t > 1.0f)
            {
                t = 1.0f;
                s = clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    // Compute the closest points on both segments
    *closest_seg1 = (Vector3){seg1_a->x + d1.x * s, seg1_a->y + d1.y * s, seg1_a->z + d1.z * s};
    *closest_seg2 = (Vector3){seg2_a->x + d2.x * t, seg2_a->y + d2.y * t, seg2_a->z + d2.z * t};
}

/* triangle_getBarycentricCoordinates
compute the barycentric coordinates u, v, w of a point p inside the triangle (a, b, c) */
inline void triangle_getBarycentricCoordinates(const Vector3 *a, const Vector3 *b, const Vector3 *c,
                                               const Vector3 *p, float *u, float *v, float *w)
{
    Vector3 v0 = {b->x - a->x, b->y - a->y, b->z - a->z};
    Vector3 v1 = {c->x - a->x, c->y - a->y, c->z - a->z};
    Vector3 v2 = {p->x - a->x, p->y - a->y, p->z - a->z};

    float d00 = v0.x * v0.x + v0.y * v0.y + v0.z * v0.z;
    float d01 = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
    float d11 = v1.x * v1.x + v1.y * v1.y + v1.z * v1.z;
    float d20 = v2.x * v0.x + v2.y * v0.y + v2.z * v0.z;
    float d21 = v2.x * v1.x + v2.y * v1.y + v2.z * v1.z;

    float denom = d00 * d11 - d01 * d01;
    *v = (d11 * d20 - d01 * d21) / denom;
    *w = (d00 * d21 - d01 * d20) / denom;
    *u = 1.0f - *v - *w;
}

/* plane_intersectionWithSegment
compute the intersection between a plane and a segment */
inline float plane_intersectionWithSegment(const Vector3 *a, const Vector3 *b, float plane_displacement, const Vector3 *planeormal)
{
    const float parallelEpsilon = 0.0001f;
    float t = -1.0f;

    Vector3 ab = {b->x - a->x, b->y - a->y, b->z - a->z};
    float nDotAB = planeormal->x * ab.x + planeormal->y * ab.y + planeormal->z * ab.z;

    // If the segment is not parallel to the plane
    if (fabsf(nDotAB) > parallelEpsilon)
    {
        t = (plane_displacement - (planeormal->x * a->x + planeormal->y * a->y + planeormal->z * a->z)) / nDotAB;
    }

    return t;
}

/* line_distanceToPoint
compute the distance between a point "point" and a line given by the points "linePointA" and "linePointB" */
inline float line_distanceToPoint(const Vector3 *linePointA, const Vector3 *linePointB, const Vector3 *point)
{
    float distAB = sqrt((linePointB->x - linePointA->x) * (linePointB->x - linePointA->x) +
                        (linePointB->y - linePointA->y) * (linePointB->y - linePointA->y) +
                        (linePointB->z - linePointA->z) * (linePointB->z - linePointA->z));

    if (distAB < 1e-6f)
    {
        return sqrt((point->x - linePointA->x) * (point->x - linePointA->x) +
                    (point->y - linePointA->y) * (point->y - linePointA->y) +
                    (point->z - linePointA->z) * (point->z - linePointA->z));
    }

    Vector3 crossProduct = {(point->y - linePointA->y) * (point->z - linePointB->z) - (point->z - linePointA->z) * (point->y - linePointB->y),
                            (point->z - linePointA->z) * (point->x - linePointB->x) - (point->x - linePointA->x) * (point->z - linePointB->z),
                            (point->x - linePointA->x) * (point->y - linePointB->y) - (point->y - linePointA->y) * (point->x - linePointB->x)};

    float crossLength = sqrt(crossProduct.x * crossProduct.x + crossProduct.y * crossProduct.y + crossProduct.z * crossProduct.z);

    return crossLength / distAB;
}

inline float segment_distanceToPoint(const Vector3 *a, const Vector3 *b, const Vector3 *p)
{
    // Calculate the vector from a to b and from a to p
    Vector3 ab = vector3_difference(b, a);
    Vector3 ap = vector3_difference(p, a);

    // Project the vector ap onto ab and clamp the value of t
    float t = vector3_returnDotProduct(&ap, &ab) / vector3_returnDotProduct(&ab, &ab);
    t = max2(0, min2(1, t)); // Clamp t to the range [0, 1]

    // Calculate the closest point on the segment
    Vector3 scaled_ab = vector3_returnScaled(&ab, t);
    Vector3 closestPoint = vector3_sum(a, &scaled_ab);

    // Calculate the distance from p to the closest point
    Vector3 diff = vector3_difference(p, &closestPoint);
    return vector3_magnitude(&diff);
}

Matrix3x3 rotationMatrix_getFromEuler(const Vector3 *rotation)
{
    float rad_x = rad(rotation->x);
    float cos_rad_x = fm_cosf(rad_x);
    float sin_rad_x = fm_sinf(rad_x);

    float rad_y = rad(rotation->y);
    float cos_rad_y = fm_cosf(rad_y);
    float sin_rad_y = fm_sinf(rad_y);

    float rad_z = rad(rotation->z);
    float cos_rad_z = fm_cosf(rad_z);
    float sin_rad_z = fm_sinf(rad_z);

    Matrix3x3 R_x = {
        .row = {
            {1, 0, 0},
            {0, cos_rad_x, -sin_rad_x},
            {0, sin_rad_x, cos_rad_x}}};

    Matrix3x3 R_y = {
        .row = {
            {cos_rad_y, 0, sin_rad_y},
            {0, 1, 0},
            {-sin_rad_y, 0, cos_rad_y}}};

    Matrix3x3 R_z = {
        .row = {
            {cos_rad_z, -sin_rad_z, 0},
            {sin_rad_z, cos_rad_z, 0},
            {0, 0, 1}}};

    Matrix3x3 R_temp = matrix3x3_multiply(&R_x, &R_y);
    Matrix3x3 R = matrix3x3_multiply(&R_temp, &R_z);

    return R;
}

// this function delivers an elegant simplification saving significant performance
// in comparison with the standard way to do it using matrix and vector operations;
//
// Matrix3x3 matrix = rotationMatrix_getFromEuler(rotation);
// point = matrix3x3_multiplyByVector(matrix, point);
//
// as with the rotation matrix algorithm, this does not work when the point lays on any axis
// for that you must use a quaternion rotation
void point_rotateZYX(Vector3 *point, const Vector3 *rotation)
{
    float rad_x = rad(rotation->x);
    float cos_rad_x = fm_cosf(rad_x);
    float sin_rad_x = fm_sinf(rad_x);

    float rad_y = rad(rotation->y);
    float cos_rad_y = fm_cosf(rad_y);
    float sin_rad_y = fm_sinf(rad_y);

    float rad_z = rad(rotation->z);
    float cos_rad_z = fm_cosf(rad_z);
    float sin_rad_z = fm_sinf(rad_z);

    // Rotate around Z axis
    float xZ = point->x * cos_rad_z - point->y * sin_rad_z;
    float yZ = point->x * sin_rad_z + point->y * cos_rad_z;

    // Rotate around Y axis
    float xY = xZ * cos_rad_y + point->z * sin_rad_y;
    float zY = -xZ * sin_rad_y + point->z * cos_rad_y;

    // Rotate around X axis
    point->y = yZ * cos_rad_x - zY * sin_rad_x;
    point->z = yZ * sin_rad_x + zY * cos_rad_x;
    point->x = xY;
}

void point_rotateXYZ(Vector3 *point, const Vector3 *rotation)
{
    float rad_x = rad(rotation->x);
    float cos_rad_x = fm_cosf(rad_x);
    float sin_rad_x = fm_sinf(rad_x);

    float rad_y = rad(rotation->y);
    float cos_rad_y = fm_cosf(rad_y);
    float sin_rad_y = fm_sinf(rad_y);

    float rad_z = rad(rotation->z);
    float cos_rad_z = fm_cosf(rad_z);
    float sin_rad_z = fm_sinf(rad_z);

    // Rotate around X axis (inverse order)
    float yX = point->y * cos_rad_x + point->z * sin_rad_x;
    float zX = -point->y * sin_rad_x + point->z * cos_rad_x;

    // Rotate around Y axis (inverse order)
    float xY = point->x * cos_rad_y - zX * sin_rad_y;
    float zY = point->x * sin_rad_y + zX * cos_rad_y;

    // Rotate around Z axis (inverse order)
    float xZ = xY * cos_rad_z + yX * sin_rad_z;
    float yZ = -xY * sin_rad_z + yX * cos_rad_z;

    point->x = xZ;
    point->y = yZ;
    point->z = zY;
}

void point_transformToLocalSpace(Vector3 *global_point, const Vector3 *local_center, const Vector3 *local_rotation)
{
    // Translate point by the inverse of Box's center
    vector3_subtract(global_point, local_center);

    // Rotate the translated point by the negative of Box's rotation
    Vector3 inverse_rotation = vector3_getInverse(local_rotation);
    point_rotateZYX(global_point, &inverse_rotation);
}

void point_transformToGlobalSpace(Vector3 *local_point, const Vector3 *local_center, const Vector3 *local_rotation)
{
    // Apply rotation to the local point to get it in the global space orientation
    Vector3 inverse_rotation = vector3_getInverse(local_rotation);
    point_rotateXYZ(local_point, &inverse_rotation);

    // Translate the rotated point by adding the Box's center
    vector3_add(local_point, local_center);
}

// another very convenient and very difficult to figure out algorithm
void rotate_normal(Vector3 *vector, const Vector3 *rotation)
{
    Vector3 rad_rotation = vector3_degToRad(rotation);
    Quaternion q_rotation = quaternion_getFromVector(&rad_rotation);
    *vector = vector3_rotateByQuaternion(vector, &q_rotation);
}

void rotate_vector(Vector3 *vector, const Vector3 *rotation)
{
    Vector3 rad_rotation = vector3_degToRad(rotation);
    Quaternion q_rotation = quaternion_getFromVector(&rad_rotation);
    *vector = vector3_rotateByQuaternion(vector, &q_rotation);
}

inline Vector3 vector3_fromQuaternion(Quaternion q)
{
    Vector3 euler;

    // Roll (X-axis rotation)
    float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    euler.x = atan2f(sinr_cosp, cosr_cosp);

    // Pitch (Y-axis rotation)
    float sinp = 2 * (q.w * q.y - q.z * q.x);
    if (fabs(sinp) >= 1)
        euler.y = copysignf(M_PI / 2, sinp); // Use 90 degrees if out of range
    else
        euler.y = asinf(sinp);

    // Yaw (Z-axis rotation)
    float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    euler.z = atan2f(siny_cosp, cosy_cosp);

    return euler;
}

#endif
