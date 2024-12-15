/*
====================
MAT4_H
Author Name: jhalldevelop
Description: This header file contains functions for performing operations 
on 4x4 vectors, including addition, subtraction, multiplication, 
division, normalization, and distance calculations.
====================
*/
#ifndef MAT4_H
#define MAT4_H
#include "AF_Math_Define.h"
#include "AF_Math.h"
#include "AF_Vec4.h"
#ifdef __cplusplus
extern "C" {
#endif

    /*
    ====================
    Mat4 Struct
    Defines a 3D vector with 4xVec4 components.
    ====================
    */
    typedef struct {
        Vec4 rows[4];
    } Mat4;
   
   /*
    ====================
    Mat4_IDENTITY
    Create an identity matrix.
    ====================
    */
    static inline Mat4 Mat4_IDENTITY(void){
        Mat4 returnMatrix = {{
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}            // Row order position
        }};
        return returnMatrix;
    }

    /*
    ====================
    Mat4_ZERO
    Create a zero matrix.
    ====================
    */
    static inline Mat4 Mat4_ZERO(void){
        Mat4 returnMatrix = {{
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}            // Row order position
        }};
        return returnMatrix;
    }

    /*
    ====================
    Mat4_ADD_M4
    Add two 4x4 matrices.
    ====================
    */
    static inline Mat4 Mat4_ADD_M4(Mat4 _leftM4, Mat4 _rightM4)
    {
        Mat4 result = Mat4_ZERO();

        result.rows[0] = Vec4_ADD(_leftM4.rows[0], _rightM4.rows[0]);
        result.rows[1] = Vec4_ADD(_leftM4.rows[1], _rightM4.rows[1]);
        result.rows[2] = Vec4_ADD(_leftM4.rows[2], _rightM4.rows[2]);
        result.rows[3] = Vec4_ADD(_leftM4.rows[3], _rightM4.rows[3]);
        return result;  
    }

    /*
    ====================
    Mat4_MINUS_M4
    Subtract one 4x4 matrix from another.
    ====================
    */
    static inline Mat4 Mat4_MINUS_M4(Mat4 _leftM4, Mat4 _rightM4)
    {
        Mat4 result = Mat4_ZERO();

        result.rows[0] = Vec4_MINUS(_leftM4.rows[0], _rightM4.rows[0]);
        result.rows[1] = Vec4_MINUS(_leftM4.rows[1], _rightM4.rows[1]);
        result.rows[2] = Vec4_MINUS(_leftM4.rows[2], _rightM4.rows[2]);
        result.rows[3] = Vec4_MINUS(_leftM4.rows[3], _rightM4.rows[3]);
        return result;  
    }

    /*
    ====================
    Mat4_MULT_SCALAR
    Multiply a 4x4 matrix by a scalar.
    ====================
    */
    static inline Mat4 Mat4_MULT_SCALAR (Mat4 _matrix, AF_FLOAT _f)
    {
        Mat4 result = Mat4_ZERO();

        result.rows[0] = Vec4_MULT_SCALAR(_matrix.rows[0], _f);
        result.rows[1] = Vec4_MULT_SCALAR(_matrix.rows[1], _f);
        result.rows[2] = Vec4_MULT_SCALAR(_matrix.rows[2], _f);
        result.rows[3] = Vec4_MULT_SCALAR(_matrix.rows[3], _f);
        return result;  
    }

    /*
    ====================
    Mat4_MULT_M4
    Multiply two 4x4 matrices.
    ====================
    */
    static inline Mat4 Mat4_MULT_M4(Mat4 _leftM4, Mat4 _rightM4)
    {
        Mat4 result = Mat4_ZERO();

        result.rows[0] = Vec4_MULT(_leftM4.rows[0], _rightM4.rows[0]);
        result.rows[1] = Vec4_MULT(_leftM4.rows[1], _rightM4.rows[1]);
        result.rows[2] = Vec4_MULT(_leftM4.rows[2], _rightM4.rows[2]);
        result.rows[3] = Vec4_MULT(_leftM4.rows[3], _rightM4.rows[3]);
        return result;  
    }

    /*
    ====================
    Mat4_DOT_M4
    Calculate the dot product of two 4x4 matrices.
    ====================
    */
    static inline Mat4 Mat4_DOT_M4(Mat4 left, Mat4 right) {
        Mat4 result = Mat4_ZERO();

        for (int i = 0; i < 4; ++i) {
            result.rows[i].x = left.rows[i].x * right.rows[0].x +
                               left.rows[i].y * right.rows[1].x +
                               left.rows[i].z * right.rows[2].x +
                               left.rows[i].w * right.rows[3].x;

            result.rows[i].y = left.rows[i].x * right.rows[0].y +
                               left.rows[i].y * right.rows[1].y +
                               left.rows[i].z * right.rows[2].y +
                               left.rows[i].w * right.rows[3].y;

            result.rows[i].z = left.rows[i].x * right.rows[0].z +
                               left.rows[i].y * right.rows[1].z +
                               left.rows[i].z * right.rows[2].z +
                               left.rows[i].w * right.rows[3].z;

            result.rows[i].w = left.rows[i].x * right.rows[0].w +
                               left.rows[i].y * right.rows[1].w +
                               left.rows[i].z * right.rows[2].w +
                               left.rows[i].w * right.rows[3].w;
        }

        return result;
    }

    /*
    ====================
    Mat4_DIV_SCALAR
    Divide a 4x4 matrix by a scalar.
    ====================
    */
    static inline Mat4 Mat4_DIV_SCALAR(Mat4 _v, AF_FLOAT _f)    
    {
        Mat4 result = Mat4_ZERO();

        result.rows[0] = Vec4_DIV_SCALAR(_v.rows[0], _f);
        result.rows[1] = Vec4_DIV_SCALAR(_v.rows[1], _f);
        result.rows[2] = Vec4_DIV_SCALAR(_v.rows[2], _f);
        result.rows[3] = Vec4_DIV_SCALAR(_v.rows[3], _f);
        return result;
    }

    /*
    ====================
    Mat4_DIV_M4
    Divide one 4x4 matrix by another.
    ====================
    */
    static inline Mat4 Mat4_DIV_M4(Mat4 _leftM4, Mat4 _rightM4)
    {
        Mat4 result = Mat4_ZERO();

        result.rows[0] = Vec4_DIV(_leftM4.rows[0], _rightM4.rows[0]);
        result.rows[1] = Vec4_DIV(_leftM4.rows[1], _rightM4.rows[1]);
        result.rows[2] = Vec4_DIV(_leftM4.rows[2], _rightM4.rows[2]);
        result.rows[3] = Vec4_DIV(_leftM4.rows[3], _rightM4.rows[3]);
        return result;
    }

    /*
    ====================
    Mat4_NORMALIZE
    Normalize a vector.
    If the magnitude of the vector is zero, the vector is not modified.
    ====================
    */
    static inline Vec4 Mat4_NORMALIZE(Vec4 v)
    {   
        AF_FLOAT magnitude = AF_Math_Sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
        AF_FLOAT epsilon = AF_EPSILON; // Threshold for considering magnitude as zero
        if (magnitude < epsilon) {
            // Return a default unit vector or other appropriate error handling
            return v;
        }
        Vec4 result = { v.x / magnitude, v.y / magnitude, v.z / magnitude, v.w / magnitude };
        return result;
    }

    /*
    ====================
    Mat4_MAGNITUDE
    Calculate the magnitude of a vector.
    ====================
    */
    static inline AF_FLOAT Mat4_MAGNITUDE(Vec4 v)
    {
        AF_FLOAT magnitude = 0;
        magnitude += v.x * v.x;
        magnitude += v.y * v.y;
        magnitude += v.z * v.z;
        magnitude += v.w * v.w;
        return AF_Math_Sqrt(magnitude);
    }

    /*
    ====================
    Mat4_DISTANCE
    Calculate the distance between two vectors.
    ====================
    */
    static inline AF_FLOAT Mat4_DISTANCE(Vec4 v1, Vec4 v2)
    {
        AF_FLOAT distance = 0;
        distance += (v1.x - v2.x) * (v1.x - v2.x);
        distance += (v1.y - v2.y) * (v1.y - v2.y);
        distance += (v1.z - v2.z) * (v1.z - v2.z);
        distance += (v1.w - v2.w) * (v1.w - v2.w);
        return AF_Math_Sqrt(distance);
    }

    /*
    ====================
    Mat4_SCALE_V4
    Scale a 4x4 matrix by a vector.
    This function scales each row of the matrix by the corresponding component of the vector.
    ====================
    */
    static inline Mat4 Mat4_SCALE_V4(Mat4 _matrix, Vec4 _scale)
    {
        Mat4 result = Mat4_ZERO();

        for (int i = 0; i < 4; ++i) {
            result.rows[i].x = _matrix.rows[i].x * _scale.x;
            result.rows[i].y = _matrix.rows[i].y * _scale.y;
            result.rows[i].z = _matrix.rows[i].z * _scale.z;
            result.rows[i].w = _matrix.rows[i].w * _scale.w;
        }

        return result;
    }

    /*
    ====================
    Mat4_ROTATE_V4
    Rotate a 4x4 matrix around a specified axis by a given angle (in radians).
    This function uses the axis-angle representation for rotation.
    ====================
    */
    static inline Mat4 Mat4_ROTATE_V4(Mat4 _matrix, Vec4 _axis, AF_FLOAT _angle)
    {
        // Normalize the rotation axis
        Vec4 normalizedAxis = Mat4_NORMALIZE(_axis);
        AF_FLOAT cosAngle = AF_Math_Cos(_angle);
        AF_FLOAT sinAngle = AF_Math_Sin(_angle);
        AF_FLOAT oneMinusCos = 1.0f - cosAngle;

        // Create the rotation matrix
        Mat4 rotationMatrix = {{
            { cosAngle + normalizedAxis.x * normalizedAxis.x * oneMinusCos,
              normalizedAxis.x * normalizedAxis.y * oneMinusCos - normalizedAxis.z * sinAngle,
              normalizedAxis.x * normalizedAxis.z * oneMinusCos + normalizedAxis.y * sinAngle,
              0 },
            { normalizedAxis.y * normalizedAxis.x * oneMinusCos + normalizedAxis.z * sinAngle,
              cosAngle + normalizedAxis.y * normalizedAxis.y * oneMinusCos,
              normalizedAxis.y * normalizedAxis.z * oneMinusCos - normalizedAxis.x * sinAngle,
              0 },
            { normalizedAxis.z * normalizedAxis.x * oneMinusCos - normalizedAxis.y * sinAngle,
              normalizedAxis.z * normalizedAxis.y * oneMinusCos + normalizedAxis.x * sinAngle,
              cosAngle + normalizedAxis.z * normalizedAxis.z * oneMinusCos,
              0 },
            { 0, 0, 0, 1 } // Homogeneous coordinate row
        }};

        // Multiply the input matrix by the rotation matrix
        return Mat4_MULT_M4(_matrix, rotationMatrix);
    }

    /*
    ====================
    Mat4_TRANSFORM_V4
    Transform a 4D vector by a 4x4 matrix.
    This function applies the transformation defined by the matrix to the vector.
    ====================
    */
    static inline Vec4 Mat4_TRANSFORM_V4(Mat4 _matrix, Vec4 _vector)
    {
        Vec4 result;
        result.x = _matrix.rows[0].x * _vector.x + _matrix.rows[0].y * _vector.y + _matrix.rows[0].z * _vector.z + _matrix.rows[0].w * _vector.w;
        result.y = _matrix.rows[1].x * _vector.x + _matrix.rows[1].y * _vector.y + _matrix.rows[1].z * _vector.z + _matrix.rows[1].w * _vector.w;
        result.z = _matrix.rows[2].x * _vector.x + _matrix.rows[2].y * _vector.y + _matrix.rows[2].z * _vector.z + _matrix.rows[2].w * _vector.w;
        result.w = _matrix.rows[3].x * _vector.x + _matrix.rows[3].y * _vector.y + _matrix.rows[3].z * _vector.z + _matrix.rows[3].w * _vector.w;
        return result;
    }

    /*
    =========================
    AF_Math_Lookat
    Creates a view matrix that transforms coordinates from world space to view space.
    Parameters:
        _target: The target position to look at.
        _position: The position of the camera.
        _up: The up direction vector.
    Returns:
        A 4x4 view matrix.
    =========================
    */
   
    /*
    static inline Mat4 Mat4_Lookat(Vec3 _target, Vec3 _position, Vec3 _up){
        Vec3 normZ = Vec3_NORMALIZE(Vec3_MINUS(_target, _position));
        Vec3 xaxis = Vec3_NORMALIZE(Vec3_CROSS(normZ, _up));
        Vec3 yaxis = Vec3_CROSS(xaxis, normZ);
        Vec3 zaxis = {-normZ.x, -normZ.y, -normZ.z};

        Vec4 row1 = {xaxis.x, yaxis.x, zaxis.x, 0};
        Vec4 row2 = {xaxis.y, yaxis.y, zaxis.y, 0};
        Vec4 row3 = {xaxis.z, yaxis.z, zaxis.z, 0};

        AF_FLOAT dotX = -Vec3_DOT(xaxis, _position);
        AF_FLOAT dotY = -Vec3_DOT(yaxis, _position);
        AF_FLOAT dotZ = -Vec3_DOT(zaxis, _position);

        Vec4 row4 = {dotX, dotY, dotZ, 1};

        Mat4 viewMatrix;
        viewMatrix.rows[0] = row1;
        viewMatrix.rows[1] = row2;
        viewMatrix.rows[2] = row3;
        viewMatrix.rows[3] = row4;
        return viewMatrix;
    }
    */

#ifdef __cplusplus
}
#endif
#endif // MAT4_H
