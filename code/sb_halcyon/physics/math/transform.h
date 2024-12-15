/**
 * @file
 *
 * Holds the transform matrix.
 */

#ifndef TRANSFORM_H
#define TRANSFORM_H

typedef struct
{
    Vector3 position;
    Quaternion orientation;
} Transform;

void transform_init(Transform *transform);
void transform_initWithMatrix(Transform *transform, const Vector3 *position, const Matrix3x3 *orientation);
void transform_initWithQuaternion(Transform *transform, const Vector3 *position, const Quaternion *orientation);

Vector3 transform_getPosition(const Transform *transform);
void transform_setPosition(Transform *transform, const Vector3 *position);
Quaternion transform_getOrientation(const Transform *transform);
void transform_setOrientation(Transform *transform, const Quaternion *orientation);

void transform_setIdentity(Transform *transform);
Transform transform_returnIdentity();

Transform transform_getInverse(const Transform *transform);
Transform transform_getInterpolated(const Transform *oldTransform, const Transform *newTransform, float factor);

Vector3 transform_getProductVector(const Transform *transform, const Vector3 *vector);
Transform transform_product(const Transform *t1, const Transform *t2);

bool transform_isValid(const Transform *transform);
bool transform_equals(const Transform *t1, const Transform *t2);
bool transform_notEquals(const Transform *t1, const Transform *t2);
void transform_toString(const Transform *transform, char *buffer, size_t bufferSize);

/* Initializes the transform to the identity transform. */
void transform_init(Transform *transform)
{
    vector3_clear(&transform->position);
    quaternion_setIdentity(&transform->orientation);
}

/* Initializes the transform with a given position and orientation matrix. */
void transform_initWithMatrix(Transform *transform, const Vector3 *position, const Matrix3x3 *orientation)
{
    transform->position = *position;
    transform->orientation = quaternion_getFromMatrix(orientation);
}

/* Initializes the transform with a given position and orientation quaternion. */
void transform_initWithQuaternion(Transform *transform, const Vector3 *position, const Quaternion *orientation)
{
    transform->position = *position;
    transform->orientation = *orientation;
}

/* Returns the position of the transform. */
Vector3 transform_getPosition(const Transform *transform)
{
    return transform->position;
}

/* Sets the position of the transform. */
void transform_setPosition(Transform *transform, const Vector3 *position)
{
    transform->position = *position;
}

/* Returns the orientation quaternion of the transform. */
Quaternion transform_getOrientation(const Transform *transform)
{
    return transform->orientation;
}

/* Sets the orientation quaternion of the transform. */
void transform_setOrientation(Transform *transform, const Quaternion *orientation)
{
    transform->orientation = *orientation;
}

/* Sets the transform to the identity transform. */
void transform_setIdentity(Transform *transform)
{
    vector3_set(&transform->position, 0.0f, 0.0f, 0.0f);
    quaternion_setIdentity(&transform->orientation);
}

/* Returns the inverse of the transform. */
Transform transform_getInverse(const Transform *transform)
{
    Quaternion invQuaternion = quaternion_getInverse(&transform->orientation);
    Vector3 invertedPosition = vector3_getInverse(&transform->position);
    Vector3 invPosition = quaternion_getVectorProduct(&invQuaternion, &invertedPosition);
    Transform inverseTransform = {invPosition, invQuaternion};
    return inverseTransform;
}

/* Returns an interpolated transform between two transforms. */
Transform transform_getInterpolated(const Transform *oldTransform, const Transform *newTransform, float factor)
{
    Vector3 scaledOldPosition = vector3_returnScaled(&oldTransform->position, 1.0f - factor);
    Vector3 scaledNewPosition = vector3_returnScaled(&newTransform->position, factor);
    Vector3 interPosition = vector3_sum(&scaledOldPosition, &scaledNewPosition);
    Quaternion interOrientation = quaternion_slerp(&oldTransform->orientation, &newTransform->orientation, factor);
    Transform interpolatedTransform = {interPosition, interOrientation};
    return interpolatedTransform;
}

/* Returns the identity transform. */
Transform transform_returnIdentity()
{
    Transform identityTransform;
    transform_setIdentity(&identityTransform);
    return identityTransform;
}

/* Returns true if the transform is valid. */
bool transform_isValid(const Transform *transform)
{
    return vector3_isFinite(&transform->position) && quaternion_isValid(&transform->orientation);
}

/* Returns the transformed vector. */
Vector3 transform_getProductVector(const Transform *transform, const Vector3 *vector)
{
    Vector3 rotatedVector = quaternion_getVectorProduct(&transform->orientation, vector);
    return vector3_sum(&rotatedVector, &transform->position);
}

/* Multiplies two transforms. */
Transform transform_product(const Transform *t1, const Transform *t2)
{
    Vector3 rotatedPosition = quaternion_getVectorProduct(&t1->orientation, &t2->position);
    Vector3 newPosition = vector3_sum(&t1->position, &rotatedPosition);
    Quaternion newOrientation = quaternion_returnProduct(&t1->orientation, &t2->orientation);
    Transform result = {newPosition, newOrientation};
    return result;
}

/* Returns true if the two transforms are equal. */
bool transform_equals(const Transform *t1, const Transform *t2)
{
    return vector3_equals(&t1->position, &t2->position) && quaternion_equals(&t1->orientation, &t2->orientation);
}

/* Returns true if the two transforms are different. */
bool transform_notEquals(const Transform *t1, const Transform *t2)
{
    return !transform_equals(t1, t2);
}

#endif
