/*
===============================================================================
AF_TIME_H

Implimentation of helper functions for time
Definition of the AF_Time struct
===============================================================================
*/
#ifndef AF_TIME_H
#define AF_TIME_H
#include <time.h>

#ifdef __cplusplus
extern "C" {    
#endif

/*
====================
AF_Time
struct to hold the data needed for monitoring time in the game
====================
*/
typedef struct {
	uint32_t currentFrame;		// The current frame.
	uint32_t currentTick;
	double timeSinceLastFrame;	// Time in ms since the last frame
	double currentTime;		// Time captured by the system clock. 
    double lastTime;		// record of the previous time captured
    double cpuTimeElapsed;		// Time measured in cpu ticks since last frame
} AF_Time;

/*
====================
AF_Time_Init
Initialise the struct variables. Don't need to take in any variables except the current time, 
pass back a new copy of initialised data in the struct.
====================
*/
static inline AF_Time AF_Time_Init(const float _currentTime){

	AF_Time returnTime = {
	.currentFrame = 0,
	.currentTick = 0,
	.timeSinceLastFrame = 0.0f,
	.currentTime = _currentTime,
	.lastTime = 0.0f,
	.cpuTimeElapsed = 0.0f
	};

	return returnTime;
}

/*
====================
AF_Time_Update
Update the time variables
====================
*/

static inline AF_Time AF_Time_Update(const AF_Time _time){
	AF_Time returnTime = {
	.currentFrame = _time.currentFrame + 1,
	.currentTick = _time.currentTick +1,
	.timeSinceLastFrame = _time.timeSinceLastFrame,
	.currentTime = _time.currentTime,
	.lastTime = _time.lastTime,
	.cpuTimeElapsed = _time.cpuTimeElapsed
	};

	return returnTime;

}

static inline double AF_Time_GetTime(){
	return ((double)(clock()) / CLOCKS_PER_SEC);
}

#ifdef __cplusplus
}
#endif

#endif //AF_TIME_H
