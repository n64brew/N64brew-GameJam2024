/**
 * @file
 *
 * holds a vector in 2 dimensions.
 */

#ifndef VECTOR2_H
#define VECTOR2_H

// Vector2 structure
typedef struct
{
    float x;
    float y;
} Vector2;

// Function prototypes

void vector2_init(Vector2 *v);
void vector2_clear(Vector2 *v);
void vector2_set(Vector2 *v, float x, float y);

void vector2_setValue(Vector2 *vector, int index, float value);
float vector2_returnValue(const Vector2 *vector, int index);

void vector2_add(Vector2 *v, const Vector2 *vector2);
void vector2_subtract(Vector2 *v, const Vector2 *vector2);
void vector2_scale(Vector2 *vector, float number);
void vector2_divideByNumber(Vector2 *vector, float number);

float vector2_magnitude(const Vector2 *vector);
float vector2_squaredMagnitude(const Vector2 *vector);

Vector2 vector2_returnUnit(const Vector2 *vector);
Vector2 vector2_returnUnitOrthogonalVector(const Vector2 *vector);

float vector2_dotProduct(const Vector2 *v, const Vector2 *vector2);
void vector2_normalize(Vector2 *vector);
Vector2 vector2_returnAbsoluteVector(const Vector2 *vector);

Vector2 vector2_min(const Vector2 *v, const Vector2 *vector2);
Vector2 vector2_max(const Vector2 *v, const Vector2 *vector2);

int vector2_returnMinAxis(const Vector2 *vector);
int vector2_returnMaxAxis(const Vector2 *vector);

bool vector2_isUnit(const Vector2 *vector);
bool vector2_isFinite(const Vector2 *vector);
bool vector2_isZero(const Vector2 *vector);

bool vector2_equals(const Vector2 *v, const Vector2 *vector2);
bool vector2_notEquals(const Vector2 *v, const Vector2 *vector2);
bool vector2_lessThan(const Vector2 *v, const Vector2 *vector2);
bool vector2_approxEqual(const Vector2 *v, const Vector2 *w, float epsilon);

// Function implementations

/* Initializes all components of the vector to zero. */
void vector2_init(Vector2 *v)
{
    v->x = 0.0f;
    v->y = 0.0f;
}

/* Clears all components of the vector to zero. */
void vector2_clear(Vector2 *v)
{
    vector2_init(v);
}

/* Sets the components of the vector to the specified values. */
void vector2_set(Vector2 *v, float x, float y)
{
    v->x = x;
    v->y = y;
}

float vector2_magnitude(const Vector2 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y);
}

float vector2_squaredMagnitude(const Vector2 *v)
{
    return v->x * v->x + v->y * v->y;
}

Vector2 vector2_returnUnit(const Vector2 *v)
{
    Vector2 vector = {0.0f, 0.0f};
    float l = vector2_magnitude(v);

    if (l < FLT_EPSILON)
        return vector;

    vector2_set(&vector, v->x / l, v->y / l);

    return vector;
}

Vector2 vector2_returnUnitOrthogonalVector(const Vector2 *v)
{
    float l = vector2_magnitude(v);
    assert(l > FLT_EPSILON);
    // Assuming clockwise rotation for orthogonal vector
    Vector2 vector = {-v->y / l, v->x / l};
    return vector;
}

bool vector2_isUnit(const Vector2 *vector)
{
    return fabs(vector2_squaredMagnitude(vector) - 1.0) < FLT_EPSILON;
}

bool vector2_isFinite(const Vector2 *vector)
{
    return isFinite(vector->x) && isFinite(vector->y);
}

bool vector2_isZero(const Vector2 *vector)
{
    return vector2_squaredMagnitude(vector) < FLT_EPSILON;
}

float vector2_dotProduct(const Vector2 *v, const Vector2 *w)
{
    return v->x * w->x + v->y * w->y;
}

void vector2_normalize(Vector2 *v)
{
    float l = vector2_magnitude(v);
    if (l < FLT_EPSILON)
        return;
    v->x /= l;
    v->y /= l;
}

Vector2 vector2_returnAbsoluteVector(const Vector2 *v)
{
    Vector2 result;
    result.x = fabsf(v->x);
    result.y = fabsf(v->y);
    return result;
}

int vector2_returnMinAxis(const Vector2 *v)
{
    return (v->x < v->y ? 0 : 1);
}

int vector2_returnMaxAxis(const Vector2 *v)
{
    return (v->x < v->y ? 1 : 0);
}

bool vector2_equals(const Vector2 *v, const Vector2 *w)
{
    return (v->x == w->x && v->y == w->y);
}

bool vector2_notEquals(const Vector2 *v, const Vector2 *w)
{
    return !vector2_equals(v, w);
}

bool vector2_lessThan(const Vector2 *v, const Vector2 *w)
{
    return (v->x == w->x ? v->y < w->y : v->x < w->x);
}

bool vector2_approxEqual(const Vector2 *v, const Vector2 *w, float epsilon)
{
    return approxEqual(v->x, w->x) && approxEqual(v->y, w->y);
}

void vector2_add(Vector2 *v, const Vector2 *w)
{
    v->x += w->x;
    v->y += w->y;
}

void vector2_subtract(Vector2 *v, const Vector2 *w)
{
    v->x -= w->x;
    v->y -= w->y;
}

void vector2_scale(Vector2 *v, float scalar)
{
    v->x *= scalar;
    v->y *= scalar;
}

void vector2_divideByNumber(Vector2 *v, float number)
{
    assert(number > FLT_EPSILON);
    v->x /= number;
    v->y /= number;
}

float vector2_returnValue(const Vector2 *v, int index)
{
    if (index == 0)
        return v->x;
    else
        return v->y;
}

void vector2_setValue(Vector2 *v, int index, float value)
{
    if (index == 0)
        v->x = value;
    else
        v->y = value;
}

Vector2 vector2_min(const Vector2 *v, const Vector2 *w)
{
    Vector2 result;
    result.x = min2(v->x, w->x);
    result.y = min2(v->y, w->y);
    return result;
}

Vector2 vector2_max(const Vector2 *v, const Vector2 *w)
{
    Vector2 result;
    result.x = max2(v->x, w->x);
    result.y = max2(v->y, w->y);
    return result;
}

#endif // VECTOR2_H
