#ifndef SCENERY_H
#define SCENERY_H

// structures

typedef struct
{

    uint32_t id;
    rspq_block_t *dl;
    T3DMat4FP *modelMat;
    T3DModel *model;

    float tile_offset;
    float transform_offset;

    Vector3 scale;
    Vector3 position;
    Vector3 rotation;

} Scenery;

// function prototypes

Scenery scenery_create(uint32_t id, const char *model_path);
void scenery_set(Scenery *scenery);
void scenery_draw(Scenery *scenery);
void scenery_delete(Scenery *scenery);

// function implementations

Scenery scenery_create(uint32_t id, const char *model_path)
{
    Scenery scenery = {
        .id = id,
        .model = t3d_model_load(model_path),
        .modelMat = malloc_uncached(sizeof(T3DMat4FP)), // needed for t3d

        .scale = {12.0f, 11.0f, 15.0f},
        .position = {0.0f, 150.0f, -50.0f},
        .rotation = {0.0f, 0.0f, 0.0f},
    };

    rspq_block_begin();
    t3d_matrix_set(scenery.modelMat, true);
    t3d_model_draw(scenery.model);
    scenery.dl = rspq_block_end();

    t3d_mat4fp_identity(scenery.modelMat);

    return scenery;
}

void scenery_set(Scenery *scenery)
{

    t3d_mat4fp_from_srt_euler(scenery->modelMat,
                              (float[3]){scenery->scale.x, scenery->scale.y, scenery->scale.z},
                              (float[3]){rad(scenery->rotation.x), rad(scenery->rotation.y), rad(scenery->rotation.z)},
                              (float[3]){scenery->position.x, scenery->position.y, scenery->position.z});
}

void scenery_draw(Scenery *scenery)
{
    rdpq_mode_zbuf(false, true);
    for (int i = 0; i < SCENERY_COUNT; i++)
    {
        rspq_block_run(scenery[i].dl);
    };
    rdpq_mode_zbuf(true, true);
}

void scenery_delete(Scenery *scenery)
{
    free_uncached(scenery->modelMat);
    t3d_model_free(scenery->model);
    rspq_block_free(scenery->dl);
}

#endif
