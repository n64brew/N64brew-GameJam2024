#include "map.hpp"

MapRenderer::MapRenderer() :
    surface {FMT_CI4, MapWidth, MapHeight},
    matFP {
        (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)),
        free_uncached
    },
    block {nullptr, rspq_block_free}
{
    assertf(matFP.get(), "Map matrix is null");

    t3d_mat4fp_from_srt_euler(
        matFP.get(),
        (float[3]){0.3f, 0.3f, 0.3f},
        (float[3]){0, 0, 0},
        (float[3]){0, 0, -10}
    );
    rspq_block_begin();
        t3d_matrix_push(matFP.get());
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));

        t3d_matrix_pop(1);
    block = U::RSPQBlock(rspq_block_end(), rspq_block_free);
}

void MapRenderer::render() {

}