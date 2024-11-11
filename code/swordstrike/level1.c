#include "types.h"
#include "functions.h"
#include <malloc.h>

struct floorPiece** loadLevel1(int* numFloors) {
    *numFloors = 18; // level 1 has 18 floors
    struct floorPiece** floors = malloc(sizeof(struct floorPiece*) * (*numFloors));

    // ROW 1
    floors[0] = malloc(sizeof(struct floorPiece));
    floors[0]->height = 5;
    floors[0]->width = 60;
    floors[0]->xPos = 280;
    floors[0]->yPos = 40;
    floors[0]->floorDroppable = true;
    defineFloor(floors[0]);

    floors[1] = malloc(sizeof(struct floorPiece));
    floors[1]->height = 5;
    floors[1]->width = 140;
    floors[1]->xPos = 100;
    floors[1]->yPos = 40;
    floors[1]->floorDroppable = true;
    defineFloor(floors[1]);

    floors[2] = malloc(sizeof(struct floorPiece));
    floors[2]->height = 5;
    floors[2]->width = 60;
    floors[2]->xPos = 0;
    floors[2]->yPos = 40;
    floors[2]->floorDroppable = true;
    defineFloor(floors[2]);

    // ROW 2
    floors[3] = malloc(sizeof(struct floorPiece));
    floors[3]->height = 5;
    floors[3]->width = 50;
    floors[3]->xPos = 290;
    floors[3]->yPos = 80;
    floors[3]->floorDroppable = true;
    defineFloor(floors[3]);

    floors[4] = malloc(sizeof(struct floorPiece));
    floors[4]->height = 5;
    floors[4]->width = 50;
    floors[4]->xPos = 210;
    floors[4]->yPos = 80;
    floors[4]->floorDroppable = true;
    defineFloor(floors[4]);

    floors[5] = malloc(sizeof(struct floorPiece));
    floors[5]->height = 5;
    floors[5]->width = 50;
    floors[5]->xPos = 120;
    floors[5]->yPos = 80;
    floors[5]->floorDroppable = true;
    defineFloor(floors[5]);

    floors[6] = malloc(sizeof(struct floorPiece));
    floors[6]->height = 5;
    floors[6]->width = 50;
    floors[6]->xPos = 30;
    floors[6]->yPos = 80;
    floors[6]->floorDroppable = true;
    defineFloor(floors[6]);

    // ROW 3
    floors[7] = malloc(sizeof(struct floorPiece));
    floors[7]->height = 5;
    floors[7]->width = 40;
    floors[7]->xPos = 300;
    floors[7]->yPos = 120;
    floors[7]->floorDroppable = true;
    defineFloor(floors[7]);

    floors[8] = malloc(sizeof(struct floorPiece));
    floors[8]->height = 5;
    floors[8]->width = 60;
    floors[8]->xPos = 180;
    floors[8]->yPos = 120;
    floors[8]->floorDroppable = true;
    defineFloor(floors[8]);

    floors[9] = malloc(sizeof(struct floorPiece));
    floors[9]->height = 5;
    floors[9]->width = 80;
    floors[9]->xPos = 60;
    floors[9]->yPos = 120;
    floors[9]->floorDroppable = true;
    defineFloor(floors[9]);

    floors[10] = malloc(sizeof(struct floorPiece));
    floors[10]->height = 5;
    floors[10]->width = 20;
    floors[10]->xPos = 0;
    floors[10]->yPos = 120;
    floors[10]->floorDroppable = true;
    defineFloor(floors[10]);

    // ROW 4
    floors[11] = malloc(sizeof(struct floorPiece));
    floors[11]->height = 5;
    floors[11]->width = 60;
    floors[11]->xPos = 240;
    floors[11]->yPos = 160;
    floors[11]->floorDroppable = true;
    defineFloor(floors[11]);

    floors[12] = malloc(sizeof(struct floorPiece));
    floors[12]->height = 5;
    floors[12]->width = 30;
    floors[12]->xPos = 170;
    floors[12]->yPos = 160;
    floors[12]->floorDroppable = true;
    defineFloor(floors[12]);

    floors[13] = malloc(sizeof(struct floorPiece));
    floors[13]->height = 5;
    floors[13]->width = 100;
    floors[13]->xPos = 30;
    floors[13]->yPos = 160;
    floors[13]->floorDroppable = true;
    defineFloor(floors[13]);

    // ROW 5
    floors[14] = malloc(sizeof(struct floorPiece));
    floors[14]->height = 5;
    floors[14]->width = 50;
    floors[14]->xPos = 290;
    floors[14]->yPos = 200;
    floors[14]->floorDroppable = true;
    defineFloor(floors[14]);

    floors[15] = malloc(sizeof(struct floorPiece));
    floors[15]->height = 5;
    floors[15]->width = 40;
    floors[15]->xPos = 210;
    floors[15]->yPos = 200;
    floors[15]->floorDroppable = true;
    defineFloor(floors[15]);

    floors[16] = malloc(sizeof(struct floorPiece));
    floors[16]->height = 5;
    floors[16]->width = 100;
    floors[16]->xPos = 70;
    floors[16]->yPos = 200;
    floors[16]->floorDroppable = true;
    defineFloor(floors[16]);

    floors[17] = malloc(sizeof(struct floorPiece));
    floors[17]->height = 5;
    floors[17]->width = 30;
    floors[17]->xPos = 0;
    floors[17]->yPos = 200;
    floors[17]->floorDroppable = true;
    defineFloor(floors[17]);
    
    return floors;
}
