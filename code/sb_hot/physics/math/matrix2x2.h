#ifndef MATRIX2X2_H
#define MATRIX2X2_H

// Libraries
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "vector2.h" // Asegúrate de que esto incluya la definición de Vector2

typedef struct
{
    Vector2 row[2];
} Matrix2x2;

// Function prototypes
void matrix2x2_init(Matrix2x2 *matrix);
void matrix2x2_clear(Matrix2x2 *matrix);

void matrix2x2_set(Matrix2x2 *matrix, float a1, float a2, float b1, float b2);
void matrix2x2_setWithValue(Matrix2x2 *matrix, float value);

Vector2 matrix2x2_returnColumn(const Matrix2x2 *matrix, int i);
Vector2 matrix2x2_returnRow(const Matrix2x2 *matrix, int i);

Matrix2x2 matrix2x2_sum(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2);
void matrix2x2_add(Matrix2x2 *matrix1, const Matrix2x2 *matrix2);

Matrix2x2 matrix2x2_difference(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2);
void matrix2x2_subtract(Matrix2x2 *matrix1, const Matrix2x2 *matrix2);

Matrix2x2 matrix2x2_returnScaled(const Matrix2x2 *matrix, float scalar);
void matrix2x2_scale(Matrix2x2 *matrix, float scalar);

Matrix2x2 matrix2x2_returnProduct(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2);
Vector2 matrix2x2_returnProductByVector(const Matrix2x2 *matrix, const Vector2 *vector);

Matrix2x2 matrix2x2_returnNegative(const Matrix2x2 *matrix);
Matrix2x2 matrix2x2_returnTranspose(const Matrix2x2 *matrix);
float matrix2x2_returnDeterminant(const Matrix2x2 *matrix);
float matrix2x2_returnTrace(const Matrix2x2 *matrix);
Matrix2x2 matrix2x2_returnInverse(const Matrix2x2 *matrix);
Matrix2x2 matrix2x2_returnAbsoluteMatrix(const Matrix2x2 *matrix);

void matrix2x2_setIdentity(Matrix2x2 *matrix);
Matrix2x2 matrix2x2_returnIdentity();

int matrix2x2_equals(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2);
int matrix2x2_notEquals(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2);

// Implementations

/* Initializes all values in the matrix to zero. */
void matrix2x2_init(Matrix2x2 *matrix)
{
    matrix2x2_set(matrix, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* Initializes the matrix with a given value. */
void matrix2x2_setWithValue(Matrix2x2 *matrix, float value)
{
    matrix2x2_set(matrix, value, value, value, value);
}

/* Sets all values in the matrix. */
void matrix2x2_set(Matrix2x2 *matrix, float a1, float a2, float b1, float b2)
{
    matrix->row[0].x = a1;
    matrix->row[0].y = a2;
    matrix->row[1].x = b1;
    matrix->row[1].y = b2;
}

/* Sets the matrix to zero. */
void matrix2x2_clear(Matrix2x2 *matrix)
{
    matrix2x2_set(matrix, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* Returns a column of the matrix. */
Vector2 matrix2x2_returnColumn(const Matrix2x2 *matrix, int i)
{
    assert(i >= 0 && i < 2);
    return (Vector2){matrix->row[0].x, matrix->row[1].x};
}

/* Returns a row of the matrix. */
Vector2 matrix2x2_returnRow(const Matrix2x2 *matrix, int i)
{
    assert(i >= 0 && i < 2);
    return matrix->row[i];
}

/* Returns the transpose of the matrix. */
Matrix2x2 matrix2x2_returnTranspose(const Matrix2x2 *matrix)
{
    return (Matrix2x2){
        .row = {
            {matrix->row[0].x, matrix->row[1].x},
            {matrix->row[0].y, matrix->row[1].y}}};
}

/* Returns the determinant of the matrix. */
float matrix2x2_returnDeterminant(const Matrix2x2 *matrix)
{
    return matrix->row[0].x * matrix->row[1].y - matrix->row[1].x * matrix->row[0].y;
}

/* Returns the trace of the matrix. */
float matrix2x2_returnTrace(const Matrix2x2 *matrix)
{
    return matrix->row[0].x + matrix->row[1].y;
}

/* Sets the matrix to the identity matrix. */
void matrix2x2_setIdentity(Matrix2x2 *matrix)
{
    matrix2x2_set(matrix, 1.0f, 0.0f, 0.0f, 1.0f);
}

/* Returns the 2x2 identity matrix. */
Matrix2x2 matrix2x2_returnIdentity()
{
    Matrix2x2 identityMatrix;
    matrix2x2_setIdentity(&identityMatrix);
    return identityMatrix;
}

/* Returns the 2x2 zero matrix. */
Matrix2x2 matrix2x2_zero()
{
    Matrix2x2 zeroMatrix;
    matrix2x2_clear(&zeroMatrix);
    return zeroMatrix;
}

/* Returns the inverse of the matrix. */
Matrix2x2 matrix2x2_returnInverse(const Matrix2x2 *matrix)
{
    float determinant = matrix2x2_returnDeterminant(matrix);
    assert(determinant > FLT_EPSILON);

    float invDeterminant = 1.0f / determinant;

    return (Matrix2x2){
        .row = {
            {matrix->row[1].y * invDeterminant, -matrix->row[0].y * invDeterminant},
            {-matrix->row[1].x * invDeterminant, matrix->row[0].x * invDeterminant}}};
}

/* Returns the matrix with absolute values. */
Matrix2x2 matrix2x2_returnAbsoluteMatrix(const Matrix2x2 *matrix)
{
    return (Matrix2x2){
        .row = {
            {fabsf(matrix->row[0].x), fabsf(matrix->row[0].y)},
            {fabsf(matrix->row[1].x), fabsf(matrix->row[1].y)}}};
}

/* Adds two matrices. */
Matrix2x2 matrix2x2_sum(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2)
{
    return (Matrix2x2){
        .row = {
            {matrix1->row[0].x + matrix2->row[0].x, matrix1->row[0].y + matrix2->row[0].y},
            {matrix1->row[1].x + matrix2->row[1].x, matrix1->row[1].y + matrix2->row[1].y}}};
}

/* Subtracts matrix2 from matrix1. */
Matrix2x2 matrix2x2_difference(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2)
{
    return (Matrix2x2){
        .row = {
            {matrix1->row[0].x - matrix2->row[0].x, matrix1->row[0].y - matrix2->row[0].y},
            {matrix1->row[1].x - matrix2->row[1].x, matrix1->row[1].y - matrix2->row[1].y}}};
}

/* Returns the negative of the matrix. */
Matrix2x2 matrix2x2_returnNegative(const Matrix2x2 *matrix)
{
    return (Matrix2x2){
        .row = {
            {-matrix->row[0].x, -matrix->row[0].y},
            {-matrix->row[1].x, -matrix->row[1].y}}};
}

/* Multiplies the matrix by a scalar. */
Matrix2x2 matrix2x2_returnScaled(const Matrix2x2 *matrix, float scalar)
{
    return (Matrix2x2){
        .row = {
            {matrix->row[0].x * scalar, matrix->row[0].y * scalar},
            {matrix->row[1].x * scalar, matrix->row[1].y * scalar}}};
}

/* Multiplies two matrices. */
Matrix2x2 matrix2x2_returnProduct(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2)
{
    return (Matrix2x2){
        .row = {
            {matrix1->row[0].x * matrix2->row[0].x + matrix1->row[0].y * matrix2->row[1].x,
             matrix1->row[0].x * matrix2->row[0].y + matrix1->row[0].y * matrix2->row[1].y},
            {matrix1->row[1].x * matrix2->row[0].x + matrix1->row[1].y * matrix2->row[1].x,
             matrix1->row[1].x * matrix2->row[0].y + matrix1->row[1].y * matrix2->row[1].y}}};
}

/* Multiplies the matrix by a vector. */
Vector2 matrix2x2_returnProductByVector(const Matrix2x2 *matrix, const Vector2 *vector)
{
    return (Vector2){
        .x = matrix->row[0].x * vector->x + matrix->row[0].y * vector->y,
        .y = matrix->row[1].x * vector->x + matrix->row[1].y * vector->y};
}

/* Checks if two matrices are equal. */
int matrix2x2_equals(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2)
{
    return (matrix1->row[0].x == matrix2->row[0].x && matrix1->row[0].y == matrix2->row[0].y &&
            matrix1->row[1].x == matrix2->row[1].x && matrix1->row[1].y == matrix2->row[1].y);
}

/* Checks if two matrices are not equal. */
int matrix2x2_notEquals(const Matrix2x2 *matrix1, const Matrix2x2 *matrix2)
{
    return !matrix2x2_equals(matrix1, matrix2);
}

/* Adds matrix2 to matrix1 and assigns the result to matrix1. */
void matrix2x2_add(Matrix2x2 *matrix1, const Matrix2x2 *matrix2)
{
    matrix1->row[0].x += matrix2->row[0].x;
    matrix1->row[0].y += matrix2->row[0].y;
    matrix1->row[1].x += matrix2->row[1].x;
    matrix1->row[1].y += matrix2->row[1].y;
}

/* Subtracts matrix2 from matrix1 and assigns the result to matrix1. */
void matrix2x2_subtract(Matrix2x2 *matrix1, const Matrix2x2 *matrix2)
{
    matrix1->row[0].x -= matrix2->row[0].x;
    matrix1->row[0].y -= matrix2->row[0].y;
    matrix1->row[1].x -= matrix2->row[1].x;
    matrix1->row[1].y -= matrix2->row[1].y;
}

/* Multiplies matrix1 by a scalar and assigns the result to matrix1. */
void matrix2x2_scale(Matrix2x2 *matrix, float scalar)
{
    matrix->row[0].x *= scalar;
    matrix->row[0].y *= scalar;
    matrix->row[1].x *= scalar;
    matrix->row[1].y *= scalar;
}

#endif // MATRIX2X2_H
