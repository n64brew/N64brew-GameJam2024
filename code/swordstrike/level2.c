#include "types.h"
#include "functions.h"
#include <malloc.h>

struct floorPiece** loadLevel2(int* numFloors) {
    *numFloors = 5; // level 2 has 5 floors
    struct floorPiece** floors = malloc(sizeof(struct floorPiece*) * (*numFloors));

    // ROW 1
    floors[0] = malloc(sizeof(struct floorPiece));
    floors[0]->height = 5;
    floors[0]->width = 235;
    floors[0]->xPos = 40;
    floors[0]->yPos = 80;
    floors[0]->floorDroppable = true;
    defineFloor(floors[0]);

    // ROW 2
    floors[1] = malloc(sizeof(struct floorPiece));
    floors[1]->height = 5;
    floors[1]->width = 140;
    floors[1]->xPos = 0;
    floors[1]->yPos = 120;
    floors[1]->floorDroppable = true;
    defineFloor(floors[1]);

    floors[2] = malloc(sizeof(struct floorPiece));
    floors[2]->height = 5;
    floors[2]->width = 140;
    floors[2]->xPos = 180;
    floors[2]->yPos = 120;
    floors[2]->floorDroppable = true;
    defineFloor(floors[2]);

    // ROW 3
    floors[3] = malloc(sizeof(struct floorPiece));
    floors[3]->height = 5;
    floors[3]->width = 235;
    floors[3]->xPos = 40;
    floors[3]->yPos = 160;
    floors[3]->floorDroppable = true;
    defineFloor(floors[3]);

    // ROW 4
    floors[4] = malloc(sizeof(struct floorPiece));
    floors[4]->height = 5;
    floors[4]->width = 315;
    floors[4]->xPos = 0;
    floors[4]->yPos = 200;
    floors[4]->floorDroppable = false;
    defineFloor(floors[4]);

    return floors;
}