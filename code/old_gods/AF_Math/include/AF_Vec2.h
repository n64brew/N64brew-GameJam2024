/*
====================
VEC2_H
Author Name: jhalldevelop
Description: This header file contains functions for performing operations 
on 2D vectors, including addition, subtraction, multiplication, 
division, normalization, and distance calculations.
====================
*/

#ifndef VEC2_H
#define VEC2_H
#include "AF_Math_Define.h"
#include "AF_Math.h"
#ifdef __cplusplus
extern "C" {
#endif

    /*
    ====================
    Vec2 Struct
    Defines a 3D vector with x, y components.
    ====================
    */
    typedef struct {
        AF_FLOAT x, y;
    } Vec2;

    /*
    ====================
    Vec2ADD
    Function for adding two 2D vectors.
    ====================
    */
    static inline Vec2 Vec2_ADD(Vec2 v1, Vec2 v2)
    {
        Vec2 result = {0,0};
        result.x = v1.x + v2.x;
        result.y = v1.y + v2.y;
        return result;  
    } 

    /*
    ====================
    Vec2_MINUS
    Function for subtracting the second 2D vector from the first.
    ====================
    */
    static inline Vec2 Vec2_MINUS(Vec2 v1, Vec2 v2)
    {
        Vec2 result = {0,0};
        result.x = v1.x - v2.x;
        result.y = v1.y - v2.y;
        return result;
    }

    /*
    ====================
    Vec2_MULT_SCALAR
    Function for multiplying a 2D vector by a scalar.
    ====================
    */
    static inline Vec2 Vec2_MULT_SCALAR(Vec2 v, AF_FLOAT f)
    {
        Vec2 result;
        result.x = v.x * f;
        result.y = v.y * f;
        return result;
    }

    /*
    ====================
    Vec2_MULT
    Function for multiplying two 2D vectors.
    ====================
    */
    static inline Vec2 Vec2_MULT(Vec2 v1, Vec2 v2)
    {
        Vec2 result;
        result.x = v1.x * v2.x;
        result.y = v1.y * v2.y;
        return result;
    }

    /*
    ====================
    Vec2_DIV_SCALAR
    Function for dividing a 2D vector by a scalar.
    ====================
    */
    static inline Vec2 Vec2_DIV_SCALAR(Vec2 v, AF_FLOAT f)    
    {
        Vec2 result;
        result.x = v.x / f;
        result.y = v.y / f;
        return result;
    }

    /*
    ====================
    Vec2_DIV
    Function for dividing the first 2D vector by the second.
    ====================
    */
    static inline Vec2 Vec2_DIV(Vec2 v1, Vec2 v2)
    {
        Vec2 result;
        result.x = v1.x / v2.x;
        result.y = v1.y / v2.y;
        return result;
    }

    /*
    ====================
    Vec2_EQUAL
    Function for checking the equality of two 2D vectors.
    ====================
    */
    static inline int Vec2_EQUAL(const Vec2 v1, const Vec2 v2)
    {
        return (v1.x == v2.x && v1.y == v2.y);
    }

    /*
    ====================
    Vec2_DOT
    Function for calculating the dot product of two 2D vectors.
    ====================
    */
    static inline AF_FLOAT Vec2_DOT(Vec2 v1, Vec2 v2)
    {
        AF_FLOAT _dot = 0;
        _dot += v1.x * v2.x;
        _dot += v1.y * v2.y;
        return _dot;
    }

    /*
    ====================
    Vec2_NORMALIZE
    Function for normalizing a 2D vector. If the magnitude is zero, the vector is not modified.
    ====================
    */
    static inline Vec2 Vec2_NORMALIZE(Vec2 v)
    {   
        AF_FLOAT magnitude = AF_Math_Sqrt((AF_FLOAT)v.x * v.x + v.y * v.y);
        AF_FLOAT epsilon = AF_EPSILON; // Threshold for considering magnitude as zero
        if (magnitude < epsilon) {
            // Return the unmodified vector if its magnitude is zero
            return v;
        }
        Vec2 result = { v.x / magnitude, v.y / magnitude };
        return result;
    }

    /*
    ====================
    Vec2_MAGNITUDE
    Function for calculating the magnitude of a 2D vector.
    ====================
    */
    static inline AF_FLOAT Vec2_MAGNITUDE(Vec2 v)
    {
        AF_FLOAT magnitude = 0;
        magnitude += v.x * v.x;
        magnitude += v.y * v.y;
        return AF_Math_Sqrt((AF_FLOAT)magnitude);
    }

    /*
    ====================
    Vec2_DISTANCE
    Function for calculating the distance between two 2D vectors.
    ====================
    */
    static inline AF_FLOAT Vec2_DISTANCE(Vec2 v1, Vec2 v2)
    {
        AF_FLOAT distance = 0;
        distance += (v1.x - v2.x) * (v1.x - v2.x);
        distance += (v1.y - v2.y) * (v1.y - v2.y);
        return AF_Math_Sqrt((AF_FLOAT)distance);
    }

    /*
    ====================
    Vec2_ROTATE_TOWARDS
    Function for calculating a rotation angle value between two 2D points.
    ====================
    */
    static inline AF_FLOAT Vec2_ROTATE_TOWARDS(Vec2 _pos1, Vec2 _pos2)
    {
        // Rotate towards pos2
        AF_FLOAT rotDistX = (_pos1.x - _pos2.x);
        AF_FLOAT rotDistY = (_pos1.y - _pos2.y);
        AF_FLOAT lookatAngle = AF_Math_Atan2(rotDistX, rotDistY);
        return lookatAngle;
    }

#ifdef __cplusplus
}
#endif

#endif  // Vec2_H
