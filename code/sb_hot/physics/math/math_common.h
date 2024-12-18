#ifndef PHYSICS_MATH_COMMON_H
#define PHYSICS_MATH_COMMON_H

#define PI 3.141592653589f
#define PI_TIMES_2 6.28318530f

#define TOLERANCE 1e-6f

// ---------- Mathematics functions ---------- //

float qi_sqrt(float number);

float rad(float angle);
float deg(float rad);

int clamp_int(int value, int lowerLimit, int upperLimit);
float clamp(float value, float lowerLimit, float upperLimit);

float max2(float a, float b);
float min2(float a, float b);

float min3(float a, float b, float c);
float max3(float a, float b, float c);

bool sameSign(float a, float b);
bool approxEqual(float a, float b);

bool isFinite(float x);

/* quick inverse square root */
inline float qi_sqrt(float number)
{
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;

    union
    {
        float f;
        long l;
    } converter;

    converter.f = y;                               // Almacena el float en la unión
    converter.l = 0x5f3759df - (converter.l >> 1); // Manipula los bits como long
    y = converter.f;                               // Recupera el resultado como float

    y = y * (threehalfs - (x2 * y * y)); // 1st iteration
    y = y * (threehalfs - (x2 * y * y)); // 2nd iteration (puede eliminarse)

    return y;
}

/* degrees to radians */
inline float rad(float angle)
{
    return PI / 180 * angle;
}

/* radians to degrees */
inline float deg(float rad)
{
    return 180 / PI * rad;
}

/* return the result of the "value" clamped by "lowerLimit" and "upperLimit" */
inline int clamp_int(int value, int lowerLimit, int upperLimit)
{
    assert(lowerLimit <= upperLimit);
    return (value < lowerLimit) ? lowerLimit : (value > upperLimit) ? upperLimit
                                                                    : value;
}

/* return the result of the "value" clamped by "lowerLimit" and "upperLimit" */
inline float clamp(float value, float lowerLimit, float upperLimit)
{
    assert(lowerLimit <= upperLimit);
    return (value < lowerLimit) ? lowerLimit : (value > upperLimit) ? upperLimit
                                                                    : value;
}

/* return higher value*/
inline float max2(float a, float b)
{
    return (a > b) ? a : b;
}

/* return lower value*/
inline float min2(float a, float b)
{
    return (a < b) ? a : b;
}

/* return the minimum value among three values */
inline float min3(float a, float b, float c)
{
    return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

/* return the maximum value among three values */
inline float max3(float a, float b, float c)
{
    return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

/* return true if two values have the same sign */
inline bool sameSign(float a, float b)
{
    return a * b >= 0.0f;
}

/* Function to test if two real numbers are (almost) equal
We test if two numbers a and b are such that (a-b) are in [-EPSILON; EPSILON] */
inline bool approxEqual(float a, float b)
{
    return (fabsf(a - b) < FLT_EPSILON);
}

inline bool isFinite(float x)
{
    return (x == x) && (x != INFINITY) && (x != -INFINITY);
}

#endif
