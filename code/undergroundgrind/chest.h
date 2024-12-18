#include <t3d/t3d.h>
#include <libdragon.h>
#include "../../core.h"
#include <t3d/t3dmodel.h>

typedef struct
{
    T3DMat4FP *chestMatFP;
    rspq_block_t *dplChestBlock;
    T3DVec3 chestPos;
    int containingDirtBlock;
} Chest;

void initChest(Chest *chest, T3DModel *chestModel, float scale, color_t color, T3DVec3 position, int dirtBlockNumber)
{
    chest->chestMatFP = malloc_uncached(sizeof(T3DMat4FP));
    chest->chestPos = position;

    t3d_mat4fp_from_srt_euler(chest->chestMatFP, (float[3]){scale, scale, scale}, (float[3]){0, 0, 0}, chest->chestPos.v);

    rspq_block_begin();
      t3d_matrix_push(chest->chestMatFP);
      rdpq_set_prim_color(color);
      t3d_model_draw(chestModel);
      t3d_matrix_pop(1);
    chest->dplChestBlock = rspq_block_end();

    chest->containingDirtBlock = dirtBlockNumber;
}

void cleanupChest(Chest *chest)
{
    rspq_block_free(chest->dplChestBlock);

    free_uncached(chest->chestMatFP);
}
