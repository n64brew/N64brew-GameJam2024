#ifndef __COMMON_H
#define __COMMON_H

#include <libdragon.h>

enum Direction {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

int randomRange(int min, int max);

#endif // __COMMON_H