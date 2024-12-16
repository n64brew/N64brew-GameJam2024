#ifndef TYPES_H
#define TYPES_H

#include <unistd.h>
#include <time.h>
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "../../core.h"

typedef enum buffers_s{
    DOUBLE_BUFFERED = 2,
    TRIPLE_BUFFERED = 3
} buffers_t;

typedef struct T3DTransform_s{
    T3DVec3 scale, rot, pos;
} T3DTransform;

#endif