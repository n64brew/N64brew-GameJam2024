#include "./props.h"

#include <malloc.h>
#include <t3d/t3dmath.h>
#include <math.h>
#include "./collision/gjk.h"
#include "./spark_effect.h"
#include "./rampage.h"
#include "./math/quaternion.h"
#include <stdlib.h>

struct Vector3 local_centers[] = {
    {0.0f, SCALE_FIXED_POINT(0.150918f), 0.0f},
    {0.0f, SCALE_FIXED_POINT(0.644826f), 0.0f},
    {0.0f, SCALE_FIXED_POINT(0.644836f), SCALE_FIXED_POINT(-0.096465f)},
    {0.0f, SCALE_FIXED_POINT(0.655209f), 0.0f},
    {0.0f, SCALE_FIXED_POINT(0.290951f), SCALE_FIXED_POINT(-0.135703f)},
};

int props_sort(const void *a, const void *b) {
    return (int)(((struct SingleProp*)a)->position.x - ((struct SingleProp*)b)->position.x);
}

#define QUAT_SCALE  (1.0f / 32000.0f)

void props_unpack_quat(struct Vector3i16* input, T3DQuat* result) {
    result->v[0] = input->x * (1.0f / 32000.0f);
    result->v[1] = input->y * (1.0f / 32000.0f);
    result->v[2] = input->z * (1.0f / 32000.0f);
    result->v[3] = sqrtf(1.0f 
        - result->v[0] * result->v[0]
        - result->v[1] * result->v[1]
        - result->v[2] * result->v[2]
    );
}

void props_init(struct AllProps* props, const char* filename) {
    props->models[0] = t3d_model_load("rom:/rampage/blockade.t3dm");
    props->models[1] = t3d_model_load("rom:/rampage/directionsign.t3dm");
    props->models[2] = t3d_model_load("rom:/rampage/streetlight.t3dm");
    props->models[3] = t3d_model_load("rom:/rampage/trafficlight.t3dm");
    props->models[4] = t3d_model_load("rom:/rampage/van.t3dm");

    for (int i = 0; i < MAX_PROP_COUNT; i += 1) {
        rampage_model_separate_material(props->models[i], &props->split_models[i]);
    }

    FILE* file = asset_fopen(filename, NULL);

    fread(&props->prop_count, 2, 1, file);

    if (props->prop_count == 0) {
        props->props = NULL;
        return;
    }

    props->props = malloc(sizeof(struct SingleProp) * props->prop_count);
    props->x_values = malloc(sizeof(float) * props->prop_count);

    for (int i = 0; i < props->prop_count; i += 1) {
        struct SingleProp* prop = &props->props[i];
        fread(&prop->asset_index, 1, 1, file);

        prop->is_active = true;

        struct Vector3i16 pos;
        fread(&pos, sizeof(struct Vector3i16), 1, file);

        prop->position.x = pos.x;
        prop->position.y = pos.y;
        prop->position.z = pos.z;

        struct Vector3i16 rotation_parts;
        fread(&rotation_parts, sizeof(struct Vector3i16), 1, file);
        T3DQuat rotation;
        props_unpack_quat(&rotation_parts, &rotation);

        t3d_mat4fp_from_srt(&prop->mtx, (float*)&gOneVec, rotation.v, (float*)&prop->position);
        data_cache_hit_writeback_invalidate(&prop->mtx, sizeof(T3DMat4FP));

        props->x_values[i] = pos.x;

        struct Vector3 local_center;
        quatMultVector((struct Quaternion*)&rotation, &local_centers[prop->asset_index], &local_center);
        vector3Add(&local_center, &prop->position, &prop->position);
    }

    qsort(props->props, props->prop_count, sizeof(struct SingleProp), props_sort);

    for (int i = 0; i < props->prop_count; i += 1) {
        props->x_values[i] = props->props[i].position.x;
    }

    fclose(file);
}

void props_render(struct AllProps* props) {
    rspq_block_run(props->split_models[0].material);

    for (int i = 0; i < props->prop_count; i += 1) {
        struct SingleProp* prop = &props->props[i];

        if (!prop->is_active || prop->asset_index >= MAX_PROP_COUNT) {
            continue;
        }
        
        t3d_matrix_push(&prop->mtx);
        rspq_block_run(props->split_models[prop->asset_index].mesh);
        t3d_matrix_pop(1);
    }
}

int props_get_start_index(short* x_values, int prop_count, short x) {
    for (int i = 0; i < prop_count; i += 1) {
        if (x_values[i] > x) {
            return i;
        }
    }

    return prop_count;
}

void point_minkowski_sum(void* data, struct Vector3* direction, struct Vector3* output) {
    *output = *(struct Vector3*)data;
}

void props_check_collision(struct AllProps* props, struct dynamic_object* obj) {
    int start_index = props_get_start_index(props->x_values, props->prop_count, (short)obj->bounding_box.min.x);

    int end_index = start_index;

    while (end_index < props->prop_count && props->x_values[end_index] < obj->bounding_box.max.x) {
        end_index += 1;
    }

    for (int i = start_index; i < end_index; i += 1) {
        struct SingleProp* prop = &props->props[i];

        if (!prop->is_active || prop->position.z > obj->bounding_box.max.z || prop->position.z < obj->bounding_box.min.z) {
            continue;
        }

        struct Simplex simplex;
        if (gjkCheckForOverlap(&simplex, obj, dynamic_object_minkowski_sum, &prop->position, point_minkowski_sum, &gRight)) {
            prop->is_active = false;
            spark_effects_spawn(&prop->position);
        }
    }
}

void props_destroy(struct AllProps* props) {
    for (int i = 0; i < MAX_PROP_COUNT; i += 1) {
        rampage_model_free_split(&props->split_models[i]);
        t3d_model_free(props->models[i]);
    }

    free(props->props);
    free(props->x_values);
    props->props = NULL;
    props->prop_count = 0;
    props->x_values = NULL;
}