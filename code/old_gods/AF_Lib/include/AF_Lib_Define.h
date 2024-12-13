/*
===============================================================================
AF_LIB_DEFINE_H
define extra types e.g. BOOL which doesn't exist in c

===============================================================================
*/
#ifndef AF_LIB_DEFINE_H
#define AF_LIB_DEFINE_H
#include <stdint.h>
// Define Static

// Define Bool
#define TRUE 1
#define FALSE 0
typedef char BOOL;		// 1 byte
typedef char PACKED_CHAR;	// 1 byte
typedef uint16_t PACKED_UINT16;	// 2 bytes
typedef uint32_t PACKED_UINT32;	// 4 bytes

#define AF_FLOAT float
#if USE_FIXED
    //#define AF_FLOAT int8_t //uint16_t
    //#define AF_EPSILON 1 << 10
#else
    //#define AF_FLOAT float
    #define AF_EPSILON 1e-6
#endif				


typedef char flag_t;		// 1 byte char to hold up to 8 flags
// We can store up to 8 bit flags in a char
#define FLAG_HAS 	0x01 	// Has bit flag 	0000 0001
#define FLAG_ENABLED 	0x02 	// Enabled bit flag	0000 0010
//#define FLAG_3 0x04		// Flag 3		0000 0100
//#define FLAG_4 0x08		// Flag 4		0000 1000
//#define FLAG_5 0x10		// Flag 5		0001 0000
//#define FLAG_6 0x20		// Flag 6		0010 0000
//#define FLAG_7 0x40		// Flag 7 		0100 0000
//#define FLAG_8 0x80		// Flag 8		1000 0000

// Helper sizeof to find the size of an array, as sizeof only tells you the size of the array pointer
#endif
