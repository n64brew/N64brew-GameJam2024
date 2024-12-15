/*
===============================================================================
AF_MATH_H
AUTHOR: jhalldevelop
Extra helper functions that can be called to perform math operations.
Compile flag to switch between fixed and floating-point operations.
Fixed operations are not implemented yet.
===============================================================================
*/
#ifndef AF_MATH_H
#define AF_MATH_H
#include "AF_Math_Define.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif



/// @brief Interpolates between two angles (radians) by 't', from T3D
static inline float AF_Math_Lerp_Angle(float a, float b, float t) {
  float angleDiff = fmodf((b - a), PI*2);
  float shortDist = fmodf(angleDiff*2, PI*2) - angleDiff;
  return a + shortDist * t;
}

// Conditional compilation for fixed-point arithmetic
#ifdef USE_FIXED

/*
=========================
AF_Math_Sqrt
Calculates the square root of a given value using fixed-point arithmetic.
Currently a placeholder implementation that needs to be completed.
=========================
*/
static inline AF_FLOAT AF_Math_Sqrt(AF_FLOAT _value){
    printf("AF_Math_Sqrt: TODO\n");
    AF_FLOAT returnValue = _value * _value / _value;	
    return returnValue;
}

/*
=========================
AF_Math_Atan2
Calculates the arctangent of two values, returning the angle in radians.
Currently a placeholder implementation that needs to be completed.
=========================
*/
static inline AF_FLOAT AF_Math_Atan2(AF_FLOAT _value1, AF_FLOAT _value2){
    printf("AF_Math_Atan2: TODO\n");	
    return _value1 + _value2;
}

/*
=========================
AF_Math_Tan
Calculates the tangent of a given angle in radians.
Currently a placeholder implementation that needs to be completed.
=========================
*/
static inline AF_FLOAT AF_Math_Tan(AF_FLOAT _value1){
    printf("AF_Math_Tan: TODO\n");	
    return _value1 + _value1;
}

/*
=========================
AF_Math_Sin
Calculates the sine of a given angle in radians.
Currently a placeholder implementation that needs to be completed.
=========================
*/
static inline AF_FLOAT AF_Math_Sin(AF_FLOAT _value){
    printf("AF_Math_Sin: TODO\n");
    return _value + _value;
}

/*
=========================
AF_Math_Cos
Calculates the cosine of a given angle in radians.
Currently a placeholder implementation that needs to be completed.
=========================
*/
static inline AF_FLOAT AF_Math_Cos(AF_FLOAT _value){
    printf("AF_Math_Cos: TODO\n");
    return _value + _value;
}

/*
=========================
AF_Math_Radians
Converts degrees to radians.
Currently a placeholder implementation that needs to be completed.
=========================
*/
static inline AF_FLOAT AF_Math_Radians(AF_FLOAT _degrees){
    printf("AF_Math_Radians: TODO \n");
    return _degrees + _degrees; 
}

#else


#define AF_PI 3.14159265358979323846  // Define the value of Pi

#include "math.h"  // Standard math library

/*
=========================
AF_Math_Sqrt
Calculates the square root of a given value using floating-point arithmetic.
=========================
*/
static inline AF_FLOAT AF_Math_Sqrt(AF_FLOAT _value){
    return sqrt(_value);
}

/*
=========================
AF_Math_Atan2
Calculates the arctangent of two values, returning the angle in radians using floating-point arithmetic.
=========================
*/
static inline AF_FLOAT AF_Math_Atan2(AF_FLOAT _value1, AF_FLOAT _value2){
    return atan2(_value1, _value2);
}

/*
=========================
AF_Math_Tan
Calculates the tangent of a given angle in radians using floating-point arithmetic.
=========================
*/
static inline AF_FLOAT AF_Math_Tan(AF_FLOAT _value1){
    return tan(_value1);
}

/*
=========================
AF_Math_Sin
Calculates the sine of a given angle in radians using floating-point arithmetic.
=========================
*/
static inline AF_FLOAT AF_Math_Sin(AF_FLOAT _value){
    return sin(_value);
}

/*
=========================
AF_Math_Cos
Calculates the cosine of a given angle in radians using floating-point arithmetic.
=========================
*/
static inline AF_FLOAT AF_Math_Cos(AF_FLOAT _value){
    return cos(_value);
}

/*
=========================
AF_Math_Radians
Converts degrees to radians using floating-point arithmetic.
=========================
*/
static inline AF_FLOAT AF_Math_Radians(AF_FLOAT _degrees){
    return (_degrees * AF_PI) / 180; 
}


#endif

#ifdef __cplusplus
}
#endif

#endif  // AF_MATH_H
