#ifndef VECTOR_3_H
#define VECTOR_3_H

// structures

typedef struct Vector3
{
    float x;
    float y;
    float z;
} Vector3;

// Macros to use t3dmath if necessary
#define T3DVec3_to_Vector3(t3dVec) ((Vector3){(t3dVec).v[0], (t3dVec).v[1], (t3dVec).v[2]})
#define Vector3_to_T3DVec3(vec) ((T3DVec3){{(vec).x, (vec).y, (vec).z}})
#define Vector3_to_fast(vec) ((fm_vec3_t){{(vec).x, (vec).y, (vec).z}})
#define fast_to_Vector3(fast) ((Vector3){{(fast).x, (fast).y, (fast).z}})

// function prototypes

void vector3_init(Vector3 *v);
void vector3_clear(Vector3 *v);
void vector3_set(Vector3 *v, float x, float y, float z);
void vector3_setElement(Vector3 *v, int index, float value);
float vector3_returnElement(const Vector3 *v, int index);

void vector3_invert(Vector3 *v);
Vector3 vector3_getInverse(const Vector3 *v);

void vector3_add(Vector3 *v, const Vector3 *w);
Vector3 vector3_sum(const Vector3 *v, const Vector3 *w);
void vector3_subtract(Vector3 *v, const Vector3 *w);
Vector3 vector3_difference(const Vector3 *v, const Vector3 *w);

void vector3_scale(Vector3 *v, float scalar);
Vector3 vector3_returnScaled(const Vector3 *v, float scalar);
void vector3_divideByNumber(Vector3 *v, float number);
Vector3 vector3_returnQuotientByNumber(const Vector3 *v, float number);
Vector3 vector3_returnQuotientByVector(const Vector3 *v, const Vector3 *w);

void vector3_componentProduct(Vector3 *v, const Vector3 *w);
Vector3 vector3_returnComponentProduct(const Vector3 *v, const Vector3 *w);
void vector3_crossProduct(Vector3 *v, const Vector3 *w);
Vector3 vector3_returnCrossProduct(const Vector3 *v, const Vector3 *w);
float vector3_returnDotProduct(const Vector3 *v, const Vector3 *w);
void vector3_addScaledVector(Vector3 *v, const Vector3 *w, float scalar);

float vector3_magnitude(const Vector3 *v);
float vector3_squaredMagnitude(const Vector3 *v);
void vector3_normalize(Vector3 *v);
Vector3 vector3_returnNormalized(const Vector3 *v);
Vector3 vector3_returnAbsoluteVector(const Vector3 *v);

Vector3 vector3_min(const Vector3 *v, const Vector3 *w);
Vector3 vector3_max(const Vector3 *v, const Vector3 *w);
float vector3_returnMinValue(const Vector3 *v);
float vector3_returnMaxValue(const Vector3 *v);
int vector3_returnMinAxis(const Vector3 *v);
int vector3_returnMaxAxis(const Vector3 *v);

bool vector3_isUnit(const Vector3 *v);
bool vector3_isFinite(const Vector3 *v);
bool vector3_isZero(const Vector3 *v);
bool vector3_equals(const Vector3 *v, const Vector3 *w);
bool vector3_notEquals(const Vector3 *v, const Vector3 *w);
bool vector3_lessThan(const Vector3 *v, const Vector3 *w);
bool vector3_approxEquals(const Vector3 *v, const Vector3 *w);
Vector3 vector3_lerp(const Vector3 *v1, const Vector3 *v2, float t);

Vector3 vector3_from_array(float arr[3]);
Vector3 vector3_flip_coords(Vector3 vec);
Vector3 vector3_flip_up(Vector3 vec);
float vector3_squaredDistance(const Vector3 *v, const Vector3 *w);
float vector3_distance(const Vector3 *v, const Vector3 *w);
Vector3 vector3_average4(const Vector3 *v1, const Vector3 *v2, const Vector3 *v3, const Vector3 *v4);

inline void vector3_init(Vector3 *v)
{
    v->x = 0.0f;
    v->y = 0.0f;
    v->z = 0.0f;
}

inline void vector3_clear(Vector3 *v)
{
    vector3_init(v);
}

inline void vector3_set(Vector3 *v, float x, float y, float z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

inline void vector3_invert(Vector3 *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
}

inline Vector3 vector3_getInverse(const Vector3 *v)
{

    return (Vector3){-v->x, -v->y, -v->z};
}

inline void vector3_copy(Vector3 *v, const Vector3 *w)
{
    v->x = w->x;
    v->y = w->y;
    v->z = w->z;
}

inline void vector3_add(Vector3 *v, const Vector3 *w)
{
    v->x += w->x;
    v->y += w->y;
    v->z += w->z;
}

inline Vector3 vector3_sum(const Vector3 *v, const Vector3 *w)
{
    return (Vector3){v->x + w->x, v->y + w->y, v->z + w->z};
}

inline void vector3_subtract(Vector3 *v, const Vector3 *w)
{
    v->x -= w->x;
    v->y -= w->y;
    v->z -= w->z;
}

inline Vector3 vector3_difference(const Vector3 *v, const Vector3 *w)
{
    return (Vector3){v->x - w->x, v->y - w->y, v->z - w->z};
}

inline void vector3_scale(Vector3 *v, float scalar)
{
    v->x *= scalar;
    v->y *= scalar;
    v->z *= scalar;
}

inline Vector3 vector3_returnScaled(const Vector3 *v, float scalar)
{
    return (Vector3){v->x * scalar, v->y * scalar, v->z * scalar};
}

void vector3_divideByNumber(Vector3 *v, float number)
{
    assert(number > FLT_EPSILON);
    v->x /= number;
    v->y /= number;
    v->z /= number;
}

Vector3 vector3_returnQuotientByNumber(const Vector3 *v, float number)
{
    assert(number > FLT_EPSILON);
    return (Vector3){v->x / number, v->y / number, v->z / number};
}

Vector3 vector3_returnQuotientByVector(const Vector3 *v, const Vector3 *w)
{
    assert(w->x > FLT_EPSILON);
    assert(w->y > FLT_EPSILON);
    assert(w->z > FLT_EPSILON);
    return (Vector3){v->x / w->x, v->y / w->y, v->z / w->z};
}

inline void vector3_componentProduct(Vector3 *v, const Vector3 *w)
{
    v->x *= w->x;
    v->y *= w->y;
    v->z *= w->z;
}

inline Vector3 vector3_returnComponentProduct(const Vector3 *v, const Vector3 *w)
{
    return (Vector3){v->x * w->x, v->y * w->y, v->z * w->z};
}

inline void vector3_crossProduct(Vector3 *v, const Vector3 *w)
{
    v->x = v->y * w->z - v->z * w->y;
    v->y = v->z * w->x - v->x * w->z;
    v->z = v->x * w->y - v->y * w->x;
}

Vector3 vector3_returnCrossProduct(const Vector3 *v, const Vector3 *w)
{
    return (Vector3){v->y * w->z - v->z * w->y, v->z * w->x - v->x * w->z, v->x * w->y - v->y * w->x};
}

inline float vector3_returnDotProduct(const Vector3 *v, const Vector3 *w)
{
    return v->x * w->x + v->y * w->y + v->z * w->z;
}

inline void vector3_addScaledVector(Vector3 *v, const Vector3 *w, float scalar)
{
    v->x += w->x * scalar;
    v->y += w->y * scalar;
    v->z += w->z * scalar;
}

inline float vector3_magnitude(const Vector3 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

inline float vector3_squaredMagnitude(const Vector3 *v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

void vector3_normalize(Vector3 *v)
{
    float m = vector3_magnitude(v);
    if (m > 0)
    {
        m = 1.0f / m;
        vector3_scale(v, m);
    }
}

Vector3 vector3_returnNormalized(const Vector3 *v)
{
    Vector3 result = *v;
    vector3_normalize(&result);
    return result;
}

Vector3 vector3_returnAbsoluteVector(const Vector3 *v)
{
    return (Vector3){fabsf(v->x), fabsf(v->y), fabsf(v->z)};
}

inline int vector3_returnMinAxis(const Vector3 *v)
{
    return (v->x < v->y ? (v->x < v->z ? 0 : 2) : (v->y < v->z ? 1 : 2));
}

inline int vector3_returnMaxAxis(const Vector3 *v)
{
    return (v->x < v->y ? (v->y < v->z ? 2 : 1) : (v->x < v->z ? 2 : 0));
}

bool vector3_isUnit(const Vector3 *v)
{
    return approxEqual(vector3_squaredMagnitude(v), 1.0f);
}

bool vector3_isFinite(const Vector3 *v)
{
    return isFinite(v->x) && isFinite(v->y) && isFinite(v->z);
}

bool vector3_isZero(const Vector3 *v)
{
    return approxEqual(vector3_squaredMagnitude(v), 0.0f);
}

inline bool vector3_equals(const Vector3 *v, const Vector3 *w)
{
    return (v->x == w->x && v->y == w->y && v->z == w->z);
}

inline bool vector3_notEquals(const Vector3 *v, const Vector3 *w)
{
    return !vector3_equals(v, w);
}

inline bool vector3_lessThan(const Vector3 *v, const Vector3 *w)
{
    return (v->x == w->x ? (v->y == w->y ? v->z < w->z : v->y < w->y) : v->x < w->x);
}

bool vector3_approxEquals(const Vector3 *v, const Vector3 *w)
{
    return approxEqual(v->x, w->x) && approxEqual(v->y, w->y) && approxEqual(v->z, w->z);
}

inline float vector3_returnElement(const Vector3 *v, int index)
{
    return ((const float *)v)[index];
}

inline void vector3_setElement(Vector3 *v, int index, float value)
{
    ((float *)v)[index] = value;
}

inline Vector3 vector3_min(const Vector3 *v, const Vector3 *w)
{
    return (Vector3){min2(v->x, w->x), min2(v->y, w->y), min2(v->z, w->z)};
}

inline Vector3 vector3_max(const Vector3 *v, const Vector3 *w)
{
    return (Vector3){max2(v->x, w->x), max2(v->y, w->y), max2(v->z, w->z)};
}

inline float vector3_returnMinValue(const Vector3 *v)
{
    return min2(min2(v->x, v->y), v->z);
}

inline float vector3_returnMaxValue(const Vector3 *v)
{
    return max3(v->x, v->y, v->z);
}

Vector3 vector3_lerp(const Vector3 *v1, const Vector3 *v2, float t)
{
    Vector3 result;
    result.x = v1->x + (v2->x - v1->x) * t;
    result.y = v1->y + (v2->y - v1->y) * t;
    result.z = v1->z + (v2->z - v1->z) * t;
    return result;
}

inline Vector3 vector3_from_array(float arr[3])
{
    Vector3 result;
    result.x = arr[0];
    result.y = arr[1];
    result.z = arr[2];
    return result;
}

// Function to convert a vector from a right-handed Y-up coordinate system to a left-handed Z-up coordinate system
inline Vector3 vector3_flip_coords(Vector3 vec)
{
    Vector3 result;
    result.x = vec.x;
    result.y = vec.z;
    result.z = -vec.y;
    return result;
}

// Function to convert a vector from a right-handed Y-up coordinate system to a right-handed Z-up coordinate system
inline Vector3 vector3_flip_up(Vector3 vec)
{
    Vector3 result;
    result.x = vec.x;
    result.y = -vec.z;
    result.z = vec.y;
    return result;
}

// Function to convert T3D AABB coordinates to engine's format
inline Vector3 vector3_from_int16(const int16_t int_arr[3])
{
    Vector3 vec;
    vec.x = (float)int_arr[0];
    vec.y = -(float)int_arr[2];
    vec.z = (float)int_arr[1];
    return vec;
}

float vector3_squaredDistance(const Vector3 *v, const Vector3 *w)
{
    Vector3 diff = vector3_difference(v, w);
    return vector3_returnDotProduct(&diff, &diff);
}

float vector3_distance(const Vector3 *v, const Vector3 *w)
{
    return sqrtf(vector3_squaredDistance(v, w));
}

Vector3 vector3_average4(const Vector3 *v1, const Vector3 *v2, const Vector3 *v3, const Vector3 *v4)
{
    Vector3 avg;
    avg.x = (v1->x + v2->x + v3->x + v4->x) / 4.0f;
    avg.y = (v1->y + v2->y + v3->y + v4->y) / 4.0f;
    avg.z = (v1->z + v2->z + v3->z + v4->z) / 4.0f;
    return avg;
}

#endif
