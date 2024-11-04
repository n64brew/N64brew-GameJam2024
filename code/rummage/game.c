#include "game.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>


T3DModel* roomModel;
T3DMat4FP* roomMatFP;
rspq_block_t* roomDpl;


void game_init()
{
    roomModel = t3d_model_load("rom:/rummage/room.t3dm");
    roomMatFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(roomMatFP, (float[3]){1, 1, 1}, (float[3]){0, 0, 0}, (float[3]){0, 0, 0});
    rspq_block_begin();
        t3d_matrix_push(roomMatFP);
        t3d_model_draw(roomModel);
        t3d_matrix_pop(1);
    roomDpl = rspq_block_end();
}


void game_logic(float deltatime)
{
}

void game_render(float deltatime)
{
    rspq_block_run(roomDpl);
}


void game_cleanup()
{
    rspq_block_free(roomDpl);
    free_uncached(roomMatFP);
    t3d_model_free(roomModel);
}
