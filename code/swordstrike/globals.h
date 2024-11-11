#ifndef GLOBALS_H
#define GLOBALS_H

// PLAYER + PHYSICS GLOBALS
#define PLAYER_HORIZ_MOVE_SPEED 2
#define GRAVITY 0.4
#define HORIZ_RESISTANCE 0.4
#define JUMP_STRENGTH 7.0
#define MAX_VERT_VELOCITY 7.0
#define DROPDOWN_STRENGTH 1.5
#define SLIDE_HORIZ_STRENGTH 8.0 //must be a multiple of 0.4 or bad things will happen
#define SLIDE_VERT_STRENGTH 1
// #define VERT_KNOCKBACK -2.0
// #define HORIZ_KNOCKBACK -2.0
#define TOLERANCE 2 // general tolerance for bounding box detection
#define VERT_TOLERANCE 2  // tolerance for vertical overlap check
#define SLIDE_COOLDOWN 15

// SCREEN EDGES
#define RIGHT_EDGE 315
#define LEFT_EDGE 5

#endif