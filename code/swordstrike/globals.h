#ifndef GLOBALS_H
#define GLOBALS_H

// PLAYER + PHYSICS GLOBALS
#define PLAYER_HORIZ_MOVE_SPEED 2
#define GRAVITY 0.4
#define HORIZ_RESISTANCE 0.4
#define JUMP_STRENGTH 6.4
#define MAX_VERT_VELOCITY 6.4
#define DROPDOWN_STRENGTH 1.5
#define SLIDE_HORIZ_STRENGTH 8.4 //must be a multiple of 0.4 or bad things will happen
#define SLIDE_VERT_STRENGTH 1
#define TOLERANCE 2 // general tolerance for bounding box detection
#define FLOOR_TOLERANCE 5 // tolerance for floor detection
#define SLIDE_COOLDOWN 10

// SCREEN EDGES
#define RIGHT_EDGE 315
#define LEFT_EDGE 5

#endif