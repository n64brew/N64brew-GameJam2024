/*
===============================================================================
AF_RANDOM_H

Implementation of helper functions for generating random numbers
===============================================================================
*/
#ifndef AF_RANDOM_H
#define AF_RANDOM_H

#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
====================
AF_RANDOM_INIT
Initialise the random engine from the standard lib
====================
*/
static inline void AF_RANDOM_INIT(){
	// seed the random runmber generatorwith the current time.
	srand(time(NULL));
}

/*
====================
AF_RANDOM_RANGE
Random range that takes in min and max floats and produces return float 
====================
*/
static inline AF_FLOAT AF_RANDOM_RANGE(AF_FLOAT _min, AF_FLOAT _max){

	// Generate the random number from randge
	AF_FLOAT randomNum = _min + (AF_FLOAT)rand() / ((AF_FLOAT)RAND_MAX / (_max - _min));
	return randomNum;
}

#ifdef __cplusplus
}
#endif

#endif //AF_RANDOM_H
