#ifndef __RAMPAGE_PROPS_H__
#define __RAMPAGE_PROPS_H__

#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <stdbool.h>
#include "./math/vector3.h"
#include "./collision/dynamic_object.h"
#include "./assets.h"

#define MAX_PROP_COUNT  5

struct SingleProp {
    struct Vector3 position;
    T3DMat4FP mtx;
    uint8_t is_active;
    uint8_t asset_index;
};

struct AllProps {
    struct SingleProp* props;
    short* x_values;
    short prop_count;

    T3DModel* models[MAX_PROP_COUNT];
    struct RampageSplitMesh split_models[MAX_PROP_COUNT];
};

void props_init(struct AllProps* props, const char* filename);
void props_render(struct AllProps* props);
void props_check_collision(struct AllProps* props, struct dynamic_object* obj);
void props_destroy(struct AllProps* props);

#endif