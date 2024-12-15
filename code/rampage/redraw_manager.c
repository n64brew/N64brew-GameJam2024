#include "./redraw_manager.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <t3d/t3dmath.h>

#include "./math/minmax.h"

struct RedrawEntity {
    struct RedrawRect prev_frames[2];
    struct RedrawRect current;
    uint8_t was_dirty[2];
    uint8_t is_dirty;
};

static struct RedrawEntity redraw_entities[MAX_REDRAW_ENTITIES];
int redraw_entity_count;
struct RedrawRect screen_rect;
int fullscreen_count = 2;
int frame_parity = 0;

bool rect_is_empty(struct RedrawRect* rect) {
    return rect->min[0] >= rect->max[0] || rect->min[1] >= rect->max[1];
}

void rect_union(struct RedrawRect* a, struct RedrawRect* b, struct RedrawRect* result) {
    for (int i = 0; i < 2; i += 1) {
        result->min[i] = MIN(a->min[i], b->min[i]);
        result->max[i] = MAX(a->max[i], b->max[i]);
    }
}

void rect_intersection(struct RedrawRect* a, struct RedrawRect* b, struct RedrawRect* result) {
    for (int i = 0; i < 2; i += 1) {
        result->min[i] = MAX(a->min[i], b->min[i]);
        result->max[i] = MIN(a->max[i], b->max[i]);
    }
}

bool rect_does_overlap(struct RedrawRect* a, struct RedrawRect* b) {
    for (int i = 0; i < 2; i += 1) {
        if (a->max[i] <= b->min[i] || b->max[i] <= a->min[i]) {
            return false;
        }
    }

    return true;
}

void redraw_manager_init(int screen_width, int screen_height) {
    screen_rect = (struct RedrawRect){
        .min = {0, 0},
        .max = {screen_width, screen_height},
    };
}

RedrawHandle redraw_aquire_handle() {
    redraw_entity_count += 1;
    return redraw_entity_count;
}

void redraw_update_dirty(RedrawHandle handle, struct RedrawRect* rect) {
    if (handle < 1 || handle >= MAX_REDRAW_ENTITIES) {
        return;
    }

    struct RedrawEntity* entity = &redraw_entities[handle - 1];

    if (rect) {
        rect_intersection(rect, &screen_rect, &entity->current);
    } else {
        entity->current.min[0] = 0;
        entity->current.min[1] = 0;

        entity->current.max[0] = 0;
        entity->current.max[1] = 0;
    }
    entity->is_dirty = 1;
}

int redraw_collect_rects(struct RedrawRect* rects) {
    int active_rect_count = 0;

    for (int i = 0; i < redraw_entity_count; i += 1) {
        struct RedrawEntity* entity = &redraw_entities[i];

        if (!entity->is_dirty && !entity->was_dirty[frame_parity]) {
            continue;
        }

        bool was_empty = rect_is_empty(&entity->prev_frames[frame_parity]);

        if (was_empty) {
            entity->prev_frames[frame_parity] = entity->current;
            entity->was_dirty[frame_parity] = entity->is_dirty;
            entity->is_dirty = false;
            continue;
        }

        struct RedrawRect* rect = &rects[active_rect_count];
        *rect = entity->prev_frames[frame_parity];

        entity->prev_frames[frame_parity] = entity->current;
        entity->was_dirty[frame_parity] = entity->is_dirty;
        entity->is_dirty = false;

        active_rect_count += 1;
    }

    return active_rect_count;
}

int rect_compare(const void *a, const void *b) {
    struct RedrawRect* aRect = (struct RedrawRect*)a;
    struct RedrawRect* bRect = (struct RedrawRect*)b;

    return aRect->min[0] - bRect->min[0];
}

int redraw_retrieve_dirty_rects(struct RedrawRect rects[MAX_REDRAW_ENTITIES]) {
    if (fullscreen_count > 0) {
        rects[0] = screen_rect;
        fullscreen_count -= 1;
        frame_parity = frame_parity ^ 1;
        return 1;
    }

    int result = redraw_collect_rects(rects);
    frame_parity = frame_parity ^ 1;
    return result;
}

static T3DVec3 box_corners[] = {
    {{-1.0f, 0.0f, -1.0f}},
    {{1.0f, 0.0f, -1.0f}},
    {{1.0f, 0.0f, 1.0f}},
    {{-1.0f, 0.0f, 1.0f}},

    {{-1.0f, 1.0f, -1.0f}},
    {{1.0f, 1.0f, -1.0f}},
    {{1.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}},
};

void redraw_get_screen_rect(T3DViewport* viewport, struct Vector3* world_pos, float radius, float min_y, float y_height, struct RedrawRect* result) {
    for (int i = 0; i < sizeof(box_corners) / sizeof(*box_corners); i += 1) {
        T3DVec3 billboardScreenPos;
        T3DVec3 billboardPos;

        T3DVec3* corner = &box_corners[i];

        billboardPos.x = world_pos->x + corner->x * radius;
        billboardPos.y = world_pos->y + corner->y * y_height;
        billboardPos.z = world_pos->z + corner->z * radius;

        t3d_viewport_calc_viewspace_pos(viewport, &billboardScreenPos, &billboardPos);

        short screen_pos[2];
        screen_pos[0] = floorf(billboardScreenPos.x);
        screen_pos[1] = floorf(billboardScreenPos.y);

        if (i == 0) {
            result->min[0] = screen_pos[0];
            result->min[1] = screen_pos[1];

            result->max[0] = screen_pos[0];
            result->max[1] = screen_pos[1];
        } else {
            for (int i = 0; i < 2; i += 1) {
                result->min[i] = MIN(result->min[i], screen_pos[i]);
                result->max[i] = MAX(result->max[i], screen_pos[i]);
            }
        }
    }
}