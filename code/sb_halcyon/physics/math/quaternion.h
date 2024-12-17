/**
 * @file
 *
 * Holds a quaternion.
 */

#ifndef QUATERNION_H
#define QUATERNION_H

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Quaternion;

// Function prototypes
void quaternion_set(Quaternion *q, float x, float y, float z, float w);
void quaternion_init(Quaternion *q);
void quaternion_clear(Quaternion *q);

void quaternion_setWithVector(Quaternion *q, float w, const Vector3 *v);

Quaternion quaternion_sum(const Quaternion *q, const Quaternion *r);
Quaternion quaternion_difference(const Quaternion *q, const Quaternion *r);
Quaternion quaternion_returnScaled(const Quaternion *q, float scalar);
Quaternion quaternion_returnProduct(const Quaternion *q, const Quaternion *r);
Vector3 quaternion_getVectorProduct(const Quaternion *q, const Vector3 *vector);

void quaternion_setIdentity(Quaternion *q);
Vector3 quaternion_returnVectorV(const Quaternion *q);
float quaternion_magnitude(const Quaternion *q);
float quaternion_squaredMagnitude(const Quaternion *q);
void quaternion_normalize(Quaternion *q);
void quaternion_invert(Quaternion *q);

Quaternion quaternion_returnUnit(const Quaternion *q);

Quaternion quaternion_getConjugate(const Quaternion *q);
Quaternion quaternion_getInverse(const Quaternion *q);
float quaternion_dotProduct(const Quaternion *q, const Quaternion *r);

bool quaternion_isFinite(const Quaternion *q);
bool quaternion_isUnit(const Quaternion *q);
bool quaternion_isValid(const Quaternion *q);
bool quaternion_equals(const Quaternion *q, const Quaternion *r);

void quaternion_setFromEulerAngles(Quaternion *quaternion, float angleX, float angleY, float angleZ);
Quaternion quaternion_getFromEulerAngles(float angleX, float angleY, float angleZ);
Quaternion quaternion_getFromVector(const Vector3 *rotation);
Quaternion quaternion_getFromMatrix(const Matrix3x3 *matrix);

void quaternion_setRotationAngleAxis(Quaternion *quaternion, float *angle, Vector3 *axis);
Matrix3x3 quaternion_getMatrix(const Quaternion *quaternion);

Quaternion quaternion_slerp(const Quaternion *q, const Quaternion *r, float t);

Quaternion quat_from_array(float arr[4]);

// Implementations

/* Initializes the quaternion to zero. */
void quaternion_init(Quaternion *q)
{
    quaternion_set(q, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* Initializes the quaternion with the component w and the vector v=(x y z). */
void quaternion_setWithVector(Quaternion *q, float w, const Vector3 *v)
{
    quaternion_set(q, v->x, v->y, v->z, w);
}

/* Sets all values in the quaternion. */
void quaternion_set(Quaternion *q, float x, float y, float z, float w)
{
    q->x = x;
    q->y = y;
    q->z = z;
    q->w = w;
}

/* Sets the quaternion to zero. */
void quaternion_clear(Quaternion *q)
{
    quaternion_set(q, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* Sets the quaternion to the identity quaternion. */
void quaternion_setIdentity(Quaternion *q)
{
    quaternion_set(q, 0.0f, 0.0f, 0.0f, 1.0f);
}

/* Returns the vector v=(x y z) of the quaternion. */
Vector3 quaternion_returnVectorV(const Quaternion *q)
{
    return (Vector3){q->x, q->y, q->z};
}

/* Returns the length of the quaternion. */
float quaternion_magnitude(const Quaternion *q)
{
    return sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
}

/* Returns the square of the length of the quaternion. */
float quaternion_squaredMagnitude(const Quaternion *q)
{
    return q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w;
}

/* Normalizes the quaternion. */
void quaternion_normalize(Quaternion *q)
{
    float l = quaternion_magnitude(q);
    assert(l > TOLERANCE);
    q->x /= l;
    q->y /= l;
    q->z /= l;
    q->w /= l;
}

/* Inverses the quaternion. */
void quaternion_invert(Quaternion *q)
{
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
}

/* Returns the unit quaternion. */
Quaternion quaternion_returnUnit(const Quaternion *q)
{
    float ql = quaternion_magnitude(q);
    assert(ql > TOLERANCE);
    return (Quaternion){q->x / ql, q->y / ql, q->z / ql, q->w / ql};
}

/* Returns the identity quaternion. */
Quaternion quaternion_identity()
{
    return (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
}

/* Returns the conjugate of the quaternion. */
Quaternion quaternion_getConjugate(const Quaternion *q)
{
    return (Quaternion){-q->x, -q->y, -q->z, q->w};
}

/* Returns the inverse of the quaternion. */
Quaternion quaternion_getInverse(const Quaternion *q)
{
    return quaternion_getConjugate(q);
}

/* Computes the dot product between two quaternions. */
float quaternion_dotProduct(const Quaternion *q, const Quaternion *r)
{
    return q->x * r->x + q->y * r->y + q->z * r->z + q->w * r->w;
}

/* Checks if the quaternion's values are finite. */
bool quaternion_isFinite(const Quaternion *q)
{
    return isFinite(q->x) && isFinite(q->y) && isFinite(q->z) && isFinite(q->w);
}

/* Checks if the quaternion is a unit quaternion. */
bool quaternion_isUnit(const Quaternion *q)
{
    float length = quaternion_magnitude(q);
    return fabsf(length - 1.0f) < 1e-5f;
}

/* Checks if the quaternion is valid. */
bool quaternion_isValid(const Quaternion *q)
{
    return quaternion_isFinite(q) && quaternion_isUnit(q);
}

/* Adds two quaternions. */
Quaternion quaternion_sum(const Quaternion *q, const Quaternion *r)
{
    return (Quaternion){q->x + r->x, q->y + r->y, q->z + r->z, q->w + r->w};
}

/* Subtracts r from q. */
Quaternion quaternion_difference(const Quaternion *q, const Quaternion *r)
{
    return (Quaternion){q->x - r->x, q->y - r->y, q->z - r->z, q->w - r->w};
}

/* Multiplies the quaternion by a scalar. */
Quaternion quaternion_returnScaled(const Quaternion *q, float scalar)
{
    return (Quaternion){q->x * scalar, q->y * scalar, q->z * scalar, q->w * scalar};
}

/* Multiplies two quaternions. */
Quaternion quaternion_returnProduct(const Quaternion *q, const Quaternion *r)
{
    return (Quaternion){
        q->w * r->x + r->w * q->x + q->y * r->z - q->z * r->y,
        q->w * r->y + r->w * q->y + q->z * r->x - q->x * r->z,
        q->w * r->z + r->w * q->z + q->x * r->y - q->y * r->x,
        q->w * r->w - q->x * r->x - q->y * r->y - q->z * r->z};
}

/* Multiplies the quaternion by a vector. */
Vector3 quaternion_getVectorProduct(const Quaternion *q, const Vector3 *vector)
{
    float prodX = q->w * vector->x + q->y * vector->z - q->z * vector->y;
    float prodY = q->w * vector->y + q->z * vector->x - q->x * vector->z;
    float prodZ = q->w * vector->z + q->x * vector->y - q->y * vector->x;
    float prodW = -q->x * vector->x - q->y * vector->y - q->z * vector->z;

    return (Vector3){
        q->w * prodX - prodY * q->z + prodZ * q->y - prodW * q->x,
        q->w * prodY - prodZ * q->x + prodX * q->z - prodW * q->y,
        q->w * prodZ - prodX * q->y + prodY * q->x - prodW * q->z};
}

/* Checks if two quaternions are equal. */
bool quaternion_equals(const Quaternion *q, const Quaternion *r)
{
    return (q->x == r->x && q->y == r->y && q->z == r->z && q->w == r->w);
}

/* Initializes the quaternion using Euler angles. */
void quaternion_setFromEulerAngles(Quaternion *quaternion, float angleX, float angleY, float angleZ)
{
    float angle = angleX * 0.5f;
    float sinX = fm_sinf(angle);
    float cosX = fm_cosf(angle);

    angle = angleY * 0.5f;
    float sinY = fm_sinf(angle);
    float cosY = fm_cosf(angle);

    angle = angleZ * 0.5f;
    float sinZ = fm_sinf(angle);
    float cosZ = fm_cosf(angle);

    float cosYcosZ = cosY * cosZ;
    float sinYcosZ = sinY * cosZ;
    float cosYsinZ = cosY * sinZ;
    float sinYsinZ = sinY * sinZ;

    quaternion->x = sinX * cosYcosZ - cosX * sinYsinZ;
    quaternion->y = cosX * sinYcosZ + sinX * cosYsinZ;
    quaternion->z = cosX * cosYsinZ - sinX * sinYcosZ;
    quaternion->w = cosX * cosYcosZ + sinX * sinYsinZ;

    /* Normalize the quaternion */
    // quaternion_normalize(quaternion);
}

/* Returns a quaternion constructed from Euler angles (in radians). */
Quaternion quaternion_getFromEulerAngles(float angleX, float angleY, float angleZ)
{
    Quaternion quaternion;
    quaternion_setFromEulerAngles(&quaternion, angleX, angleY, angleZ);
    return quaternion;
}

/* Returns a quaternion constructed from Euler angles (in radians). */
Quaternion quaternion_getFromVector(const Vector3 *rotation)
{
    Quaternion quaternion;
    quaternion_setFromEulerAngles(&quaternion, rotation->x, rotation->y, rotation->z);
    return quaternion;
}

/* Creates a unit quaternion from a rotation matrix. */
Quaternion quaternion_getFromMatrix(const Matrix3x3 *matrix)
{
    Quaternion quaternion;

    /* Get the trace of the matrix */
    float trace = matrix->row[0].x + matrix->row[1].y + matrix->row[2].z;

    float r;
    float s;

    if (trace < 0.0f)
    {
        if (matrix->row[1].y > matrix->row[0].x)
        {
            if (matrix->row[2].z > matrix->row[1].y)
            {
                r = sqrtf(matrix->row[2].z - matrix->row[0].x - matrix->row[1].y + 1.0f);
                s = 0.5f / r;

                /* Compute the quaternion */
                quaternion.x = (matrix->row[2].x + matrix->row[0].z) * s;
                quaternion.y = (matrix->row[1].z + matrix->row[2].y) * s;
                quaternion.z = 0.5f * r;
                quaternion.w = (matrix->row[1].x - matrix->row[0].y) * s;
            }
            else
            {
                r = sqrtf(matrix->row[1].y - matrix->row[2].z - matrix->row[0].x + 1.0f);
                s = 0.5f / r;

                /* Compute the quaternion */
                quaternion.x = (matrix->row[0].y + matrix->row[1].x) * s;
                quaternion.y = 0.5f * r;
                quaternion.z = (matrix->row[1].z + matrix->row[2].y) * s;
                quaternion.w = (matrix->row[0].z - matrix->row[2].x) * s;
            }
        }
        else if (matrix->row[2].z > matrix->row[0].x)
        {
            r = sqrtf(matrix->row[2].z - matrix->row[0].x - matrix->row[1].y + 1.0f);
            s = 0.5f / r;

            /* Compute the quaternion */
            quaternion.x = (matrix->row[2].x + matrix->row[0].z) * s;
            quaternion.y = (matrix->row[1].z + matrix->row[2].y) * s;
            quaternion.z = 0.5f * r;
            quaternion.w = (matrix->row[1].x - matrix->row[0].y) * s;
        }
        else
        {
            r = sqrtf(matrix->row[0].x - matrix->row[1].y - matrix->row[2].z + 1.0f);
            s = 0.5f / r;

            /* Compute the quaternion */
            quaternion.x = 0.5f * r;
            quaternion.y = (matrix->row[0].y + matrix->row[1].x) * s;
            quaternion.z = (matrix->row[2].x + matrix->row[0].z) * s;
            quaternion.w = (matrix->row[2].y - matrix->row[1].z) * s;
        }
    }
    else
    {
        r = sqrtf(trace + 1.0f);
        s = 0.5f / r;

        /* Compute the quaternion */
        quaternion.x = (matrix->row[2].y - matrix->row[1].z) * s;
        quaternion.y = (matrix->row[0].z - matrix->row[2].x) * s;
        quaternion.z = (matrix->row[1].x - matrix->row[0].y) * s;
        quaternion.w = 0.5f * r;
    }

    /* Normalize the quaternion to ensure it represents a valid rotation */
    quaternion_normalize(&quaternion);
    return quaternion;
}

/* Computes the rotation angle (in radians) and the rotation axis. */
void quaternion_setRotationAngleAxis(Quaternion *quaternion, float *angle, Vector3 *axis)
{
    /* Compute the rotation angle */
    *angle = acosf(quaternion->w) * 2.0f;

    /* Compute the 3D rotation axis */
    Vector3 rotationAxis = {quaternion->x, quaternion->y, quaternion->z};

    /* Normalize the rotation axis */
    vector3_normalize(&rotationAxis);

    /* Set the rotation axis values */
    vector3_set(axis, rotationAxis.x, rotationAxis.y, rotationAxis.z);
}

/* Returns the orientation matrix corresponding to this quaternion. */
Matrix3x3 quaternion_getMatrix(const Quaternion *quaternion)
{
    float nQ = quaternion->x * quaternion->x + quaternion->y * quaternion->y + quaternion->z * quaternion->z + quaternion->w * quaternion->w;
    float s = 0.0f;

    if (nQ > 0.0f)
    {
        s = 2.0f / nQ;
    }

    /* Computations used for optimization (less multiplications) */
    float xs = quaternion->x * s;
    float ys = quaternion->y * s;
    float zs = quaternion->z * s;
    float wxs = quaternion->w * xs;
    float wys = quaternion->w * ys;
    float wzs = quaternion->w * zs;
    float xxs = quaternion->x * xs;
    float xys = quaternion->x * ys;
    float xzs = quaternion->x * zs;
    float yys = quaternion->y * ys;
    float yzs = quaternion->y * zs;
    float zzs = quaternion->z * zs;

    /* Create the matrix corresponding to the quaternion */
    return (Matrix3x3){
        .row = {
            {1.0f - yys - zzs, xys - wzs, xzs + wys},
            {xys + wzs, 1.0f - xxs - zzs, yzs - wxs},
            {xzs - wys, yzs + wxs, 1.0f - xxs - yys}}};
}

Quaternion quaternion_slerp(const Quaternion *q, const Quaternion *r, float t)
{
    assert(t >= 0.0f && t <= 1.0f);

    float invert = 1.0f;

    /* Compute cos(theta) using the quaternion scalar product */
    float cosineTheta = quaternion_dotProduct(q, r);

    /* Take care of the sign of cosineTheta */
    if (cosineTheta < 0.0f)
    {
        cosineTheta = -cosineTheta;
        invert = -1.0f;
    }

    /* Because of precision, if cos(theta) is nearly 1,
    therefore theta is nearly 0 and we can write
    sin((1-t)*theta) as (1-t) and sin(t*theta) as t */
    float epsilon = 0.00001f;
    if (1.0f - cosineTheta < epsilon)
    {
        Quaternion q1 = quaternion_returnScaled(q, 1.0f - t);
        Quaternion q2 = quaternion_returnScaled(r, t * invert);
        return quaternion_sum(&q1, &q2);
    }

    /* Compute the theta angle */
    float theta = acosf(cosineTheta);

    /* Compute sin(theta) */
    float sineTheta = fm_sinf(theta);

    /* Compute the two coefficients that are in the spherical linear interpolation formula */
    float coeff1 = fm_sinf((1.0f - t) * theta) / sineTheta;
    float coeff2 = fm_sinf(t * theta) / sineTheta * invert;

    /* Compute and return the interpolated quaternion */
    Quaternion q1 = quaternion_returnScaled(q, coeff1);
    Quaternion q2 = quaternion_returnScaled(r, coeff2);
    return quaternion_sum(&q1, &q2);
}

Vector3 quaternion_rotateVector(Vector3 v, Quaternion q)
{
    Vector3 u = {q.x, q.y, q.z};
    float s = q.w;

    Vector3 rv1 = vector3_returnScaled(&u, 2.0f * vector3_returnDotProduct(&u, &v));
    Vector3 rv2 = vector3_returnScaled(&v, (s * s - vector3_returnDotProduct(&u, &u)));
    Vector3 crossProduct = vector3_returnCrossProduct(&u, &v);
    Vector3 rv3 = vector3_returnScaled(&crossProduct, 2.0f * s);

    Vector3 result = vector3_sum(&rv1, &rv2);
    result = vector3_sum(&result, &rv3);

    return result;
}

inline Quaternion quat_from_array(float arr[4])
{
    Quaternion result;
    result.x = arr[0];
    result.y = arr[1];
    result.z = arr[2];
    result.w = arr[3];
    return result;
}

#endif