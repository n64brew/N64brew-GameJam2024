/*
===============================================================================
AF_MATH_DEFINE_H
AUTHOR: jhalldevelop
define extra types e.g. BOOL which doesn't exist in c

===============================================================================
*/
#ifndef AF_MATH_DEFINE_H  
#define AF_MATH_DEFINE_H
#include <stdint.h>

#define PI 3.141592653589793
// Switch to fixed point math if set at compile time
#if USE_FIXED
    #define AF_FLOAT int8_t //uint16_t
    #define AF_EPSILON 1 << 10
#else
    #define AF_FLOAT float
    #define AF_EPSILON 1e-6
#endif	

#endif
