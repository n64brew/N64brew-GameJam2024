/**
 * @file
 *
 * holds a 3x3 matrix.
 */

#ifndef MATRIX3X3_H
#define MATRIX3X3_H

#include "vector3.h"

typedef struct
{
    Vector3 row[3];
} Matrix3x3;

// Function prototypes
void matrix3x3_init(Matrix3x3 *matrix);
void matrix3x3_clear(Matrix3x3 *matrix);

void matrix3x3_set(Matrix3x3 *matrix, float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3);
void matrix3x3_setWithValue(Matrix3x3 *matrix, float value);

Vector3 matrix3x3_returnColumn(const Matrix3x3 *matrix, int i);
Vector3 matrix3x3_returnRow(const Matrix3x3 *matrix, int i);

void matrix3x3_add(Matrix3x3 *matrix1, const Matrix3x3 *matrix2);
Matrix3x3 matrix3x3_sum(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2);

void matrix3x3_subtract(Matrix3x3 *matrix1, const Matrix3x3 *matrix2);
Matrix3x3 matrix3x3_difference(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2);

Matrix3x3 matrix3x3_returnScaled(const Matrix3x3 *matrix, float scalar);
void matrix3x3_scale(Matrix3x3 *matrix, float scalar);

Matrix3x3 matrix3x3_multiply(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2);
Vector3 matrix3x3_multiplyByVector(const Matrix3x3 *matrix, const Vector3 *vector);

Matrix3x3 matrix3x3_returnNegative(const Matrix3x3 *matrix);
Matrix3x3 matrix3x3_returnTranspose(const Matrix3x3 *matrix);
float matrix3x3_returnDeterminant(const Matrix3x3 *matrix);
float matrix3x3_returnTrace(const Matrix3x3 *matrix);
Matrix3x3 matrix3x3_returnInverse(const Matrix3x3 *matrix);
Matrix3x3 matrix3x3_returnAbsoluteMatrix(const Matrix3x3 *matrix);

void matrix3x3_setIdentity(Matrix3x3 *matrix);
Matrix3x3 matrix3x3_returnIdentity();

Matrix3x3 matrix3x3_computeSkewSymmetricMatrixForCrossProduct(const Vector3 *vector);

int matrix3x3_equals(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2);
int matrix3x3_notEquals(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2);

// Implementations

/* Initializes the matrix to zero. */
void matrix3x3_init(Matrix3x3 *matrix)
{
    matrix3x3_set(matrix, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* Initializes the matrix with a given value. */
void matrix3x3_setWithValue(Matrix3x3 *matrix, float value)
{
    matrix3x3_set(matrix, value, value, value, value, value, value, value, value, value);
}

/* Sets all values in the matrix. */
void matrix3x3_set(Matrix3x3 *matrix, float a1, float a2, float a3,
                   float b1, float b2, float b3, float c1, float c2, float c3)
{
    matrix->row[0].x = a1;
    matrix->row[0].y = a2;
    matrix->row[0].z = a3;
    matrix->row[1].x = b1;
    matrix->row[1].y = b2;
    matrix->row[1].z = b3;
    matrix->row[2].x = c1;
    matrix->row[2].y = c2;
    matrix->row[2].z = c3;
}

/* Sets the matrix to zero. */
void matrix3x3_clear(Matrix3x3 *matrix)
{
    vector3_set(&matrix->row[0], 0.0f, 0.0f, 0.0f);
    vector3_set(&matrix->row[1], 0.0f, 0.0f, 0.0f);
    vector3_set(&matrix->row[2], 0.0f, 0.0f, 0.0f);
}

/* Returns a column of the matrix. */
Vector3 matrix3x3_returnColumn(const Matrix3x3 *matrix, int i)
{
    assert(i >= 0 && i < 3);
    return (Vector3){matrix->row[0].x, matrix->row[1].x, matrix->row[2].x};
}

/* Returns a row of the matrix. */
Vector3 matrix3x3_returnRow(const Matrix3x3 *matrix, int i)
{
    assert(i >= 0 && i < 3);
    return matrix->row[i];
}

/* Returns the transpose of the matrix. */
Matrix3x3 matrix3x3_returnTranspose(const Matrix3x3 *matrix)
{
    return (Matrix3x3){
        .row = {
            {matrix->row[0].x, matrix->row[1].x, matrix->row[2].x},
            {matrix->row[0].y, matrix->row[1].y, matrix->row[2].y},
            {matrix->row[0].z, matrix->row[1].z, matrix->row[2].z}}};
}

/* Returns the determinant of the matrix. */
float matrix3x3_returnDeterminant(const Matrix3x3 *matrix)
{
    return (matrix->row[0].x * (matrix->row[1].y * matrix->row[2].z - matrix->row[2].y * matrix->row[1].z) -
            matrix->row[0].y * (matrix->row[1].x * matrix->row[2].z - matrix->row[2].x * matrix->row[1].z) +
            matrix->row[0].z * (matrix->row[1].x * matrix->row[2].y - matrix->row[2].x * matrix->row[1].y));
}

/* Returns the trace of the matrix. */
float matrix3x3_returnTrace(const Matrix3x3 *matrix)
{
    return (matrix->row[0].x + matrix->row[1].y + matrix->row[2].z);
}

/* Returns the inverse of the matrix. */
Matrix3x3 matrix3x3_returnInverse(const Matrix3x3 *matrix)
{
    float determinant = matrix3x3_returnDeterminant(matrix);
    // Check if the determinant is equal to zero
    assert(determinant != 0.0f);

    float invDeterminant = 1.0f / determinant;

    Matrix3x3 tempMatrix;
    matrix3x3_set(&tempMatrix,
                  (matrix->row[1].y * matrix->row[2].z - matrix->row[2].y * matrix->row[1].z),
                  -(matrix->row[0].y * matrix->row[2].z - matrix->row[2].y * matrix->row[0].z),
                  (matrix->row[0].y * matrix->row[1].z - matrix->row[0].z * matrix->row[1].y),
                  -(matrix->row[1].x * matrix->row[2].z - matrix->row[2].x * matrix->row[1].z),
                  (matrix->row[0].x * matrix->row[2].z - matrix->row[2].x * matrix->row[0].z),
                  -(matrix->row[0].x * matrix->row[1].z - matrix->row[1].x * matrix->row[0].z),
                  (matrix->row[1].x * matrix->row[2].y - matrix->row[2].x * matrix->row[1].y),
                  -(matrix->row[0].x * matrix->row[2].y - matrix->row[2].x * matrix->row[0].y),
                  (matrix->row[0].x * matrix->row[1].y - matrix->row[0].y * matrix->row[1].x));

    // Return the inverse matrix
    return matrix3x3_returnScaled(&tempMatrix, invDeterminant);
}

/* Returns the matrix with absolute values. */
Matrix3x3 matrix3x3_returnAbsoluteMatrix(const Matrix3x3 *matrix)
{
    return (Matrix3x3){
        .row = {
            {fabsf(matrix->row[0].x), fabsf(matrix->row[0].y), fabsf(matrix->row[0].z)},
            {fabsf(matrix->row[1].x), fabsf(matrix->row[1].y), fabsf(matrix->row[1].z)},
            {fabsf(matrix->row[2].x), fabsf(matrix->row[2].y), fabsf(matrix->row[2].z)}}};
}

/* Sets the matrix to the identity matrix. */
void matrix3x3_setIdentity(Matrix3x3 *matrix)
{
    matrix3x3_set(matrix, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

/* Returns the 3x3 identity matrix. */
Matrix3x3 matrix3x3_returnIdentity()
{
    Matrix3x3 identityMatrix;
    matrix3x3_setIdentity(&identityMatrix);
    return identityMatrix;
}

/* Returns a skew-symmetric matrix for cross product. */
Matrix3x3 matrix3x3_computeSkewSymmetricMatrixForCrossProduct(const Vector3 *vector)
{
    return (Matrix3x3){
        .row = {
            {0.0f, -vector->z, vector->y},
            {vector->z, 0.0f, -vector->x},
            {-vector->y, vector->x, 0.0f}}};
}

/* Adds two matrices. */
Matrix3x3 matrix3x3_sum(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2)
{
    return (Matrix3x3){
        .row = {
            {matrix1->row[0].x + matrix2->row[0].x, matrix1->row[0].y + matrix2->row[0].y, matrix1->row[0].z + matrix2->row[0].z},
            {matrix1->row[1].x + matrix2->row[1].x, matrix1->row[1].y + matrix2->row[1].y, matrix1->row[1].z + matrix2->row[1].z},
            {matrix1->row[2].x + matrix2->row[2].x, matrix1->row[2].y + matrix2->row[2].y, matrix1->row[2].z + matrix2->row[2].z}}};
}

/* Subtracts matrix2 from matrix1. */
Matrix3x3 matrix3x3_difference(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2)
{
    return (Matrix3x3){
        .row = {
            {matrix1->row[0].x - matrix2->row[0].x, matrix1->row[0].y - matrix2->row[0].y, matrix1->row[0].z - matrix2->row[0].z},
            {matrix1->row[1].x - matrix2->row[1].x, matrix1->row[1].y - matrix2->row[1].y, matrix1->row[1].z - matrix2->row[1].z},
            {matrix1->row[2].x - matrix2->row[2].x, matrix1->row[2].y - matrix2->row[2].y, matrix1->row[2].z - matrix2->row[2].z}}};
}

/* Returns the negative of the matrix. */
Matrix3x3 matrix3x3_returnNegative(const Matrix3x3 *matrix)
{
    return (Matrix3x3){
        .row = {
            {-matrix->row[0].x, -matrix->row[0].y, -matrix->row[0].z},
            {-matrix->row[1].x, -matrix->row[1].y, -matrix->row[1].z},
            {-matrix->row[2].x, -matrix->row[2].y, -matrix->row[2].z}}};
}

/* Multiplies the matrix by a scalar. */
Matrix3x3 matrix3x3_returnScaled(const Matrix3x3 *matrix, float scalar)
{
    return (Matrix3x3){
        .row = {
            {matrix->row[0].x * scalar, matrix->row[0].y * scalar, matrix->row[0].z * scalar},
            {matrix->row[1].x * scalar, matrix->row[1].y * scalar, matrix->row[1].z * scalar},
            {matrix->row[2].x * scalar, matrix->row[2].y * scalar, matrix->row[2].z * scalar}}};
}

/* Multiplies two matrices. */
Matrix3x3 matrix3x3_multiply(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2)
{
    Matrix3x3 result;
    result.row[0].x = matrix1->row[0].x * matrix2->row[0].x + matrix1->row[0].y * matrix2->row[1].x + matrix1->row[0].z * matrix2->row[2].x;
    result.row[0].y = matrix1->row[0].x * matrix2->row[0].y + matrix1->row[0].y * matrix2->row[1].y + matrix1->row[0].z * matrix2->row[2].y;
    result.row[0].z = matrix1->row[0].x * matrix2->row[0].z + matrix1->row[0].y * matrix2->row[1].z + matrix1->row[0].z * matrix2->row[2].z;

    result.row[1].x = matrix1->row[1].x * matrix2->row[0].x + matrix1->row[1].y * matrix2->row[1].x + matrix1->row[1].z * matrix2->row[2].x;
    result.row[1].y = matrix1->row[1].x * matrix2->row[0].y + matrix1->row[1].y * matrix2->row[1].y + matrix1->row[1].z * matrix2->row[2].y;
    result.row[1].z = matrix1->row[1].x * matrix2->row[0].z + matrix1->row[1].y * matrix2->row[1].z + matrix1->row[1].z * matrix2->row[2].z;

    result.row[2].x = matrix1->row[2].x * matrix2->row[0].x + matrix1->row[2].y * matrix2->row[1].x + matrix1->row[2].z * matrix2->row[2].x;
    result.row[2].y = matrix1->row[2].x * matrix2->row[0].y + matrix1->row[2].y * matrix2->row[1].y + matrix1->row[2].z * matrix2->row[2].y;
    result.row[2].z = matrix1->row[2].x * matrix2->row[0].z + matrix1->row[2].y * matrix2->row[1].z + matrix1->row[2].z * matrix2->row[2].z;

    return result;
}

/* Multiplies the matrix by a vector. */
Vector3 matrix3x3_multiplyByVector(const Matrix3x3 *matrix, const Vector3 *vector)
{
    return (Vector3){
        .x = matrix->row[0].x * vector->x + matrix->row[0].y * vector->y + matrix->row[0].z * vector->z,
        .y = matrix->row[1].x * vector->x + matrix->row[1].y * vector->y + matrix->row[1].z * vector->z,
        .z = matrix->row[2].x * vector->x + matrix->row[2].y * vector->y + matrix->row[2].z * vector->z};
}

/* Checks if two matrices are equal. */
int matrix3x3_equals(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2)
{
    return (matrix1->row[0].x == matrix2->row[0].x && matrix1->row[0].y == matrix2->row[0].y && matrix1->row[0].z == matrix2->row[0].z &&
            matrix1->row[1].x == matrix2->row[1].x && matrix1->row[1].y == matrix2->row[1].y && matrix1->row[1].z == matrix2->row[1].z &&
            matrix1->row[2].x == matrix2->row[2].x && matrix1->row[2].y == matrix2->row[2].y && matrix1->row[2].z == matrix2->row[2].z);
}

/* Checks if two matrices are not equal. */
int matrix3x3_notEquals(const Matrix3x3 *matrix1, const Matrix3x3 *matrix2)
{
    return !matrix3x3_equals(matrix1, matrix2);
}

/* Adds matrix2 to matrix1 and assigns the result to matrix1. */
void matrix3x3_add(Matrix3x3 *matrix1, const Matrix3x3 *matrix2)
{
    matrix1->row[0].x += matrix2->row[0].x;
    matrix1->row[0].y += matrix2->row[0].y;
    matrix1->row[0].z += matrix2->row[0].z;
    matrix1->row[1].x += matrix2->row[1].x;
    matrix1->row[1].y += matrix2->row[1].y;
    matrix1->row[1].z += matrix2->row[1].z;
    matrix1->row[2].x += matrix2->row[2].x;
    matrix1->row[2].y += matrix2->row[2].y;
    matrix1->row[2].z += matrix2->row[2].z;
}

/* Subtracts matrix2 from matrix1 and assigns the result to matrix1. */
void matrix3x3_subtract(Matrix3x3 *matrix1, const Matrix3x3 *matrix2)
{
    matrix1->row[0].x -= matrix2->row[0].x;
    matrix1->row[0].y -= matrix2->row[0].y;
    matrix1->row[0].z -= matrix2->row[0].z;
    matrix1->row[1].x -= matrix2->row[1].x;
    matrix1->row[1].y -= matrix2->row[1].y;
    matrix1->row[1].z -= matrix2->row[1].z;
    matrix1->row[2].x -= matrix2->row[2].x;
    matrix1->row[2].y -= matrix2->row[2].y;
    matrix1->row[2].z -= matrix2->row[2].z;
}

/* Multiplies matrix1 by a scalar and assigns the result to matrix1. */
void matrix3x3_scale(Matrix3x3 *matrix, float scalar)
{
    matrix->row[0].x *= scalar;
    matrix->row[0].y *= scalar;
    matrix->row[0].z *= scalar;
    matrix->row[1].x *= scalar;
    matrix->row[1].y *= scalar;
    matrix->row[1].z *= scalar;
    matrix->row[2].x *= scalar;
    matrix->row[2].y *= scalar;
    matrix->row[2].z *= scalar;
}

#endif // MATRIX3X3_H
