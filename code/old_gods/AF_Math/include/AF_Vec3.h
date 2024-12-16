/*
    ====================
    VEC3_H
    Author Name: jhalldevelop
    This header file defines 3D vector operations such as addition, subtraction,
    dot product, cross product, projection, normalization, magnitude, and orthogonalization.
    The functions are implemented as inline to improve performance by avoiding function call overhead.
    The vector structure is designed to store 3D vectors (x, y, z) and operates with AF_FLOAT types.
    ====================
*/
#ifndef VEC3_H
#define VEC3_H
#include "AF_Math_Define.h"
#include "AF_Math.h"
#ifdef __cplusplus
extern "C" {
#endif

    /*
    ====================
    Vec3 Struct
    Defines a 3D vector with x, y, and z components.
    ====================
    */
    typedef struct {
        AF_FLOAT x, y, z;
    } Vec3;

    /*
    ====================
    Vec3_ZERO
    Function for creating a zero vector (0, 0, 0).
    ====================
    */
    static inline Vec3 Vec3_ZERO(void){
        Vec3 returnVec = {0, 0, 0};
        return returnVec;
    }

    /*
    ====================
    Vec3_ADD
    Function for adding two 3D vectors.
    ====================
    */
    static inline Vec3 Vec3_ADD(Vec3 v1, Vec3 v2) {
        Vec3 result;
        result.x = v1.x + v2.x;
        result.y = v1.y + v2.y;
        result.z = v1.z + v2.z;
        return result;  
    }

    /*
    ====================
    Vec3_MINUS
    Function for subtracting one 3D vector from another.
    ====================
    */
    static inline Vec3 Vec3_MINUS(Vec3 v1, Vec3 v2) {
        Vec3 result;
        result.x = v1.x - v2.x;
        result.y = v1.y - v2.y;
        result.z = v1.z - v2.z;
        return result;
    }

    /*
    ====================
    Vec3_MULT_SCALAR
    Function for multiplying a 3D vector by a scalar.
    ====================
    */
    static inline Vec3 Vec3_MULT_SCALAR(Vec3 v, AF_FLOAT f) {
        Vec3 result;
        result.x = v.x * f;
        result.y = v.y * f;
        result.z = v.z * f;
        return result;
    }

    /*
    ====================
    Vec3_MULT
    Function for element-wise multiplication of two 3D vectors.
    ====================
    */
    static inline Vec3 Vec3_MULT(Vec3 v1, Vec3 v2) {
        Vec3 result;
        result.x = v1.x * v2.x;
        result.y = v1.y * v2.y;
        result.z = v1.z * v2.z;
        return result;
    }

    /*
    ====================
    Vec3_DIV_SCALAR
    Function for dividing a 3D vector by a scalar.
    ====================
    */
    static inline Vec3 Vec3_DIV_SCALAR(Vec3 v, AF_FLOAT f) {
        Vec3 result;
        result.x = v.x / f;
        result.y = v.y / f;
        result.z = v.z / f;
        return result;
    }

    /*
    ====================
    Vec3_DIV
    Function for element-wise division of two 3D vectors.
    ====================
    */
    static inline Vec3 Vec3_DIV(Vec3 v1, Vec3 v2) {
        Vec3 result;
        result.x = v1.x / v2.x;
        result.y = v1.y / v2.y;
        result.z = v1.z / v2.z;
        return result;
    }

    /*
    ====================
    Vec3_EQUAL
    Function to check if two 3D vectors are equal.
    ====================
    */
    static inline int Vec3_EQUAL(const Vec3 v1, const Vec3 v2) {
        return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
    }

    /*
    ====================
    Vec3_DOT
    Function for calculating the dot product of two 3D vectors.
    ====================
    */
    static inline AF_FLOAT Vec3_DOT(Vec3 v1, Vec3 v2) {
        AF_FLOAT _dot = 0;
        _dot += v1.x * v2.x;
        _dot += v1.y * v2.y;
        _dot += v1.z * v2.z;
        return _dot;
    }

    /*
    ====================
    Vec3_CROSS
    Function for calculating the cross product of two 3D vectors.
    ====================
    */
    static inline Vec3 Vec3_CROSS(Vec3 v1, Vec3 v2) {
        Vec3 cross;
        cross.x = v1.y * v2.z - v1.z * v2.y;
        cross.y = v1.z * v2.x - v1.x * v2.z;
        cross.z = v1.x * v2.y - v1.y * v2.x;
        return cross;
    }

    /*
    ====================
    Vec3_NORMALIZE
    Function for normalizing a 3D vector.
    If the magnitude is zero, the vector remains unchanged.
    ====================
    */
    static inline Vec3 Vec3_NORMALIZE(Vec3 v) {
        AF_FLOAT magnitude = AF_Math_Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        AF_FLOAT epsilon = AF_EPSILON;
        if (magnitude < epsilon) {
            return v;
        }
        Vec3 result = { v.x / magnitude, v.y / magnitude, v.z / magnitude };
        return result;
    }

    /*
    ====================
    Vec3_MAGNITUDE
    Function for calculating the magnitude (length) of a 3D vector.
    ====================
    */
    static inline AF_FLOAT Vec3_MAGNITUDE(Vec3 v) {
        AF_FLOAT magnitude = 0;
        magnitude += v.x * v.x;
        magnitude += v.y * v.y;
        magnitude += v.z * v.z;
        return AF_Math_Sqrt(magnitude);
    }

    /*
    ====================
    Vec3_DISTANCE
    Function for calculating the distance between two 3D vectors.
    ====================
    */
    static inline AF_FLOAT Vec3_DISTANCE(Vec3 v1, Vec3 v2) {
        AF_FLOAT distance = 0;
        distance += (v1.x - v2.x) * (v1.x - v2.x);
        distance += (v1.y - v2.y) * (v1.y - v2.y);
        distance += (v1.z - v2.z) * (v1.z - v2.z);
        return AF_Math_Sqrt(distance);
    }

    /*
    ====================
    Vec3_PROJECTION
    Function for projecting one 3D vector onto another.
    If the denominator is zero, returns a zero vector.
    ====================
    */
    static inline Vec3 Vec3_PROJECTION(Vec3 v1, Vec3 v2) {
        AF_FLOAT nom = Vec3_DOT(v1, v2);
        AF_FLOAT denom = Vec3_MAGNITUDE(v2);
        denom *= denom;

        AF_FLOAT epsilon = AF_EPSILON;
        if (denom < epsilon) {
            Vec3 v3 = {0, 0, 0};
            return v3;
        }

        AF_FLOAT scalar = nom / denom;
        Vec3 v3 = Vec3_MULT_SCALAR(v2, scalar);
        return v3;
    }

    /*
    ====================
    Vec3_ORTHOGANLIZE
    Function for orthogonalizing three 3D vectors using the Gram-Schmidt process.
    ====================
    */
    static inline void Vec3_ORTHOGANLIZE(Vec3* v1, Vec3* v2, Vec3* v3) {
        AF_FLOAT scaling_factor;

        *v1 = Vec3_NORMALIZE(*v1);

        scaling_factor = Vec3_DOT(*v2, *v1);
        v2->x -= scaling_factor * v1->x;
        v2->y -= scaling_factor * v1->y;
        v2->z -= scaling_factor * v1->z;
        *v2 = Vec3_NORMALIZE(*v2);

        scaling_factor = Vec3_DOT(*v3, *v1);
        v3->x -= scaling_factor * v1->x;
        v3->y -= scaling_factor * v1->y;
        v3->z -= scaling_factor * v1->z;

        scaling_factor = Vec3_DOT(*v3, *v2);
        v3->x -= scaling_factor * v2->x;
        v3->y -= scaling_factor * v2->y;
        v3->z -= scaling_factor * v2->z;

        *v3 = Vec3_NORMALIZE(*v3);
    }

    /* =====================
    Vec3_Lerp
    Lerp between two points
    ======================== */
    static inline Vec3 Vec3_Lerp(Vec3 start, Vec3 end, float t) {
        Vec3 result;
        result.x = start.x + (end.x - start.x) * t;
        result.y = start.y + (end.y - start.y) * t;
        result.z = start.z + (end.z - start.z) * t;
        return result;
    }
    /*
    =========================
    area_of_triangle
    Calculates the area of a triangle defined by three vertices.
    Uses the determinant method to compute the area.
    Parameters:
        _v1: First vertex of the triangle
        _v2: Second vertex of the triangle
        _v3: Third vertex of the triangle
    Returns:
        The area of the triangle.
    =========================
    */
    static inline AF_FLOAT area_of_triangle(const Vec3 _v1, const Vec3 _v2, const Vec3 _v3)
    {
        return fabs((_v1.x * (_v2.y - _v3.y) + _v2.x * (_v3.y - _v1.y) + _v3.x * (_v1.y - _v2.y)) / 2);
    }



#ifdef __cplusplus
}
#endif

#endif  // VEC3_H
