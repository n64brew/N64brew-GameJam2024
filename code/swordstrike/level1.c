#include "types.h"
#include "functions.h"
#include <malloc.h>

struct floorPiece** loadLevel1(int* numFloors) {
    *numFloors = 7; // level has 7 floors
    struct floorPiece** floors = malloc(sizeof(struct floorPiece*) * (*numFloors));
    
    // ROW 1
    floors[0] = malloc(sizeof(struct floorPiece));
    floors[0]->height = 5;
    floors[0]->width = 140;
    floors[0]->xPos = 0;
    floors[0]->yPos = 40;
    floors[0]->floorDroppable = true;
    defineFloor(floors[0]);

    floors[1] = malloc(sizeof(struct floorPiece));
    floors[1]->height = 5;
    floors[1]->width = 140;
    floors[1]->xPos = 180;
    floors[1]->yPos = 40;
    floors[1]->floorDroppable = true;
    defineFloor(floors[1]);

    // ROW 2
    floors[2] = malloc(sizeof(struct floorPiece));
    floors[2]->height = 5;
    floors[2]->width = 235;
    floors[2]->xPos = 40;
    floors[2]->yPos = 80;
    floors[2]->floorDroppable = true;
    defineFloor(floors[2]);

    // ROW 3
    floors[3] = malloc(sizeof(struct floorPiece));
    floors[3]->height = 5;
    floors[3]->width = 140;
    floors[3]->xPos = 0;
    floors[3]->yPos = 120;
    floors[3]->floorDroppable = true;
    defineFloor(floors[3]);

    floors[4] = malloc(sizeof(struct floorPiece));
    floors[4]->height = 5;
    floors[4]->width = 140;
    floors[4]->xPos = 180;
    floors[4]->yPos = 120;
    floors[4]->floorDroppable = true;
    defineFloor(floors[4]);

    // ROW 4
    floors[5] = malloc(sizeof(struct floorPiece));
    floors[5]->height = 5;
    floors[5]->width = 235;
    floors[5]->xPos = 40;
    floors[5]->yPos = 160;
    floors[5]->floorDroppable = true;
    defineFloor(floors[5]);

    // ROW 5
    floors[6] = malloc(sizeof(struct floorPiece));
    floors[6]->height = 5;
    floors[6]->width = 315;
    floors[6]->xPos = 0;
    floors[6]->yPos = 200;
    floors[6]->floorDroppable = false;
    defineFloor(floors[6]);

    return floors;
}