#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <libdragon.h>
#include "globals.h"
#include "../../core.h"

struct point {
    int x;
    int y;
};

struct line {
    struct point p1;
    struct point p2;
};

struct floorPiece {
    int width;
    int height;
    int xPos;
    int yPos;

    bool floorDroppable;

    // collision box
    struct point leftTop;
    struct point leftBot;
    struct point rightTop;
    struct point rightBot;
    struct line colTop;
    struct line colRight;
    struct line colBot;
    struct line colLeft;
};

// ids:
// 0 = basic sword
// 1 = heavy sword
// 2 = bow (implement later)
struct weapon {
    int id;
    int xPos;
    int yPos;
    int width;
    int height;
    int attackTimer;
    int attackCooldown;

    // collision box
    struct point leftTop;
    struct point leftBot;
    struct point rightTop;
    struct point rightBot;
    struct line colTop;
    struct line colRight;
    struct line colBot;
    struct line colLeft;
};

struct player {
    PlyNum id;
    int width;
    int height;
    bool isAlive;
    int xPos;
    int yPos;
    int direction; // 0 = left, 1 = right
    color_t color;

    // movement
    float verticalVelocity;
    float horizontalVelocity;
    bool onFloor;
    bool floorDroppable;
    int dropdownCounter;
    int slideCooldown;
    int horizVelocityDir;
    bool moveLeft;
    bool moveRight;
    bool onTopOfEnemy;

    // weapon
    struct weapon weapon;
    int attackDirection;
    int attackTimer;
    int attackCooldown;

    // ai
    PlyNum ai_target;
    int ai_reactionspeed;

    // collision box
    struct point leftTop;
    struct point leftBot;
    struct point rightTop;
    struct point rightBot;
    struct line colTop;
    struct line colRight;
    struct line colBot;
    struct line colLeft;
};

#endif
