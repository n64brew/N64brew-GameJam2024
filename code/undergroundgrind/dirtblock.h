#include <t3d/t3d.h>
#include <libdragon.h>
#include "../../core.h"
#include <t3d/t3dmodel.h>

typedef struct
{
    T3DMat4FP *blockMatFP;
    rspq_block_t *dplDirtBlock;
    T3DVec3 dirtBlockPos;
    PlyNum destroyingPlayer;
    bool isContainingChest;
    bool isDestroyed;
    int damage;
} DirtBlock;

void initDirtBlock(DirtBlock *dirtBlock, T3DModel *dirtBlockModel, float blockScale, color_t color, T3DVec3 position)
{
    dirtBlock->blockMatFP = malloc_uncached(sizeof(T3DMat4FP));
    dirtBlock->dirtBlockPos = position;

    t3d_mat4fp_from_srt_euler(dirtBlock->blockMatFP, (float[3]){blockScale, blockScale, blockScale}, (float[3]){0, 0, 0}, dirtBlock->dirtBlockPos.v);

    rspq_block_begin();
      t3d_matrix_push(dirtBlock->blockMatFP);
      rdpq_set_prim_color(color);
      t3d_model_draw(dirtBlockModel);
      t3d_matrix_pop(1);
    dirtBlock->dplDirtBlock = rspq_block_end();

    dirtBlock->damage = 0;
    dirtBlock->isDestroyed = false;
    dirtBlock->isContainingChest = false;
}

void cleanupDirtBlock(DirtBlock *dirtBlock)
{
    rspq_block_free(dirtBlock->dplDirtBlock);

    free_uncached(dirtBlock->blockMatFP);
}
