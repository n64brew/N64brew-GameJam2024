#ifndef __REDRAW_MANAGER_H__
#define __REDRAW_MANAGER_H__

#include "./math/vector3.h"
#include <t3d/t3d.h>

struct RedrawRect {
    short min[2];
    short max[2];
};

typedef int RedrawHandle;

#define MAX_REDRAW_ENTITIES     64

void redraw_manager_init(int screen_width, int screen_height);

RedrawHandle redraw_aquire_handle();
void redraw_update_dirty(RedrawHandle handle, struct RedrawRect* rect);

int redraw_retrieve_dirty_rects(struct RedrawRect rects[MAX_REDRAW_ENTITIES]);

void redraw_get_screen_rect(T3DViewport* viewport, struct Vector3* world_pos, float radius, float min_y, float y_height, struct RedrawRect* result);

bool rect_does_overlap(struct RedrawRect* a, struct RedrawRect* b);

#endif