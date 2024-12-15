/*
    ====================
    VEC4_H
    Author Name: jhalldevelop
    This header file defines 3D vector operations such as addition, subtraction,
    dot product, cross product, projection, normalization, magnitude, and orthogonalization.
    The functions are implemented as inline to improve performance by avoiding function call overhead.
    The vector structure is designed to store 3D vectors (x, y, z, 1) and operates with AF_FLOAT types.
    ====================
*/
#ifndef VEC4_H
#define VEC4_H
#include "AF_Math_Define.h"
#include "AF_Math.h"
#ifdef __cplusplus
extern "C" {
#endif

    /*
    ====================
    Vec4 Struct
    Defines a 3D vector with x, y, z, w components.
    ====================
    */
    typedef struct {
        AF_FLOAT x, y, z, w;
    } Vec4;

    /*
    ====================
    Vec4_ZERO
    Returns a zero-initialized vector (0, 0, 0, 0).
    ====================
    */
    static inline Vec4 Vec4_ZERO(void){
        Vec4 returnVec = {0, 0, 0, 0};
        return returnVec;
    }

    /*
    ====================
    Vec4_ADD
    Adds two vectors element-wise and returns the result.
    ====================
    */
    static inline Vec4 Vec4_ADD(Vec4 v1, Vec4 v2)
    {
        Vec4 result = {0, 0, 0, 0};
        result.x = v1.x + v2.x;
        result.y = v1.y + v2.y;
        result.z = v1.z + v2.z;
        result.w = v1.w + v2.w;
        return result;  
    } 

    /*
    ====================
    Vec4_MINUS
    Subtracts vector v2 from v1 element-wise and returns the result.
    ====================
    */
    static inline Vec4 Vec4_MINUS(Vec4 v1, Vec4 v2)
    {
        Vec4 result = {0, 0, 0, 0};
        result.x = v1.x - v2.x;
        result.y = v1.y - v2.y;
        result.z = v1.z - v2.z;
        result.w = v1.w - v2.w;
        return result;
    }

    /*
    ====================
    Vec4_MULT_SCALAR
    Multiplies each element of vector v by a scalar value f.
    ====================
    */
    static inline Vec4 Vec4_MULT_SCALAR(Vec4 v, AF_FLOAT f)
    {
        Vec4 result = {0, 0, 0, 0};
        result.x = v.x * f;
        result.y = v.y * f;
        result.z = v.z * f;
        result.w = v.w * f;
        return result;
    }

    /*
    ====================
    Vec4_MULT
    Multiplies two vectors element-wise and returns the result.
    ====================
    */
    static inline Vec4 Vec4_MULT(Vec4 v1, Vec4 v2)
    {
        Vec4 result = {0, 0, 0, 0};
        result.x = v1.x * v2.x;
        result.y = v1.y * v2.y;
        result.z = v1.z * v2.z;
        result.w = v1.w * v2.w;
        return result;
    }

    /*
    ====================
    Vec4_DIV_SCALAR
    Divides each element of vector v by scalar f.
    ====================
    */
    static inline Vec4 Vec4_DIV_SCALAR(Vec4 v, AF_FLOAT f)    
    {
        Vec4 result = {0, 0, 0, 0};
        result.x = v.x / f;
        result.y = v.y / f;
        result.z = v.z / f;
        result.w = v.w / f;
        return result;
    }

    /*
    ====================
    Vec4_EQUAL
    Returns 1 (true) if two vectors are equal element-wise, otherwise returns 0 (false).
    ====================
    */
    static inline char Vec4_EQUAL(Vec4 v1, Vec4 v2)
    {
        return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w);
    }

    /*
    ====================
    Vec4_DIV
    Divides vector v1 by vector v2 element-wise.
    ====================
    */
    static inline Vec4 Vec4_DIV(Vec4 v1, Vec4 v2)
    {
        Vec4 result = {0, 0, 0, 0};
        result.x = v1.x / v2.x;
        result.y = v1.y / v2.y;
        result.z = v1.z / v2.z;
        result.w = v1.w / v2.w;
        return result;
    }

    /*
    ====================
    Vec4_DOT
    Computes the dot product of two vectors.
    ====================
    */
    static inline AF_FLOAT Vec4_DOT(Vec4 v1, Vec4 v2)
    {
        AF_FLOAT _dot = 0;
        _dot += v1.x * v2.x;
        _dot += v1.y * v2.y;
        _dot += v1.z * v2.z;
        _dot += v1.w * v2.w;
        return _dot;
    }

    /*
    ====================
    Vec4_CROSS
    Computes the cross product of two 3D vectors (ignoring the w component).
    The result will have a w component of 0.
    ====================
    */
    static inline Vec4 Vec4_CROSS(Vec4 v1, Vec4 v2)
    {
        Vec4 cross = {0, 0, 0, 0};
        cross.x = v1.y * v2.z - v1.z * v2.y;
        cross.y = v1.z * v2.x - v1.x * v2.z;
        cross.z = v1.x * v2.y - v1.y * v2.x;
        cross.w = 0; // Cross product can only be calculated with 3D vectors
        return cross;
    }

    /*
    ====================
    Vec4_NORMALIZE
    Normalizes the vector (i.e., scales it to have a magnitude of 1).
    If the vector is of zero magnitude, it is returned unmodified.
    ====================
    */
    static inline Vec4 Vec4_NORMALIZE(Vec4 v)
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
    Vec4_MAGNITUDE
    Returns the magnitude (length) of the vector.
    ====================
    */
    static inline AF_FLOAT Vec4_MAGNITUDE(Vec4 v)
    {
        AF_FLOAT magnitude = 0;
        magnitude += v.x * v.x;
        magnitude += v.y * v.y;
        magnitude += v.z * v.z;
        magnitude += v.w * v.w;
        AF_FLOAT sqrMag = AF_Math_Sqrt(magnitude); 
        return sqrMag;
    }

    /*
    ====================
    Vec4_DISTANCE
    Computes the Euclidean distance between two vectors.
    ====================
    */
    static inline AF_FLOAT Vec4_DISTANCE(Vec4 v1, Vec4 v2)
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
    Vec4_PROJECTION
    Projects vector v1 onto vector v2. 
    If the denominator is zero, a zero vector is returned.
    ====================
    */
    static inline Vec4 Vec4_PROJECTION(Vec4 v1, Vec4 v2)
    {
        // project v1 onto v2 using the formula: P = (P.Q/|Q|^2) * Q

        // Dot v1 . v2
        AF_FLOAT nom = Vec4_DOT(v1, v2);

        // Magnitude squared of v2
        AF_FLOAT denom = Vec4_MAGNITUDE(v2);
        denom *= denom;

        // Check for divide by zero
        AF_FLOAT epsilon = AF_EPSILON; // Threshold for considering magnitude as zero
        if (denom < epsilon) {
            Vec4 returnVec = {0, 0, 0, 0};
            return returnVec;
        }

        AF_FLOAT scalar = nom / denom;
        
        // scalar * v2
        Vec4 v3 = {0, 0, 0, 0};
        v3 = Vec4_MULT_SCALAR(v2, scalar);
        return v3;
    }

    /*
    ====================
    Vec4_ORTHOGONALISE
    Orthogonalizes vector v1 relative to vector v2 by subtracting
    the projection of v1 onto v2 from v1 itself.
    ====================
    */
    static inline Vec4 Vec4_ORTHOGONALISE(Vec4 v1, Vec4 v2)
    {
        Vec4 proj = Vec4_PROJECTION(v1, v2);
        Vec4 orthogonal = Vec4_MINUS(v1, proj);
        return orthogonal;
    }

#ifdef __cplusplus
}
#endif
#endif /* VEC4_H */
