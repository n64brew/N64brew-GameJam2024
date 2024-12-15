#ifndef MAP_H
#define MAP_H

extern map_data map;

void map_init(map_data *map);
void map_cleanup(map_data *map);

// Allocates and sets matrix, loads model and creates RSPQ block
void map_init(map_data *map)
{
    map->mtxFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(map->mtxFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0, 0, -10});
    map->model = t3d_model_load("rom:/sb_holes/map.t3dm");
    rspq_block_begin();
    t3d_matrix_set(map->mtxFP, true);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(map->model);
    map->block = rspq_block_end();
}

void map_cleanup(map_data *map)
{
    rspq_block_free(map->block);
    t3d_model_free(map->model);
    free_uncached(map->mtxFP);
}
#endif // MAP_H