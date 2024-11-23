#ifndef __RAMPAGE_PROPS_H__
#define __RAMPAGE_PROPS_H__

#include <t3d/t3dmath.h>
#include <stdbool.h>
#include "./math/vector3.h"

struct single_prop {
    struct Vector3 position;
    T3DMat4FP mtx;
    bool is_active;
};

struct prop_index_cell {
    uint8_t start_index;
    uint8_t end_index;
};

struct all_props {
    struct single_prop* props;
    int prop_count;

    short index_cell_width;
    short index_cell_height;

    short index_cell_count_x;
    short index_cell_count_y;

    short index_cell_min_x;
    short index_cell_min_y;
    struct prop_index_cell* index_cells;
};

#endif