#include "player.hpp"

PlayerGameplayData::PlayerGameplayData(T3DVec3 pos, PlyNum team, std::array<int, MAXPLAYERS> health) :
    pos(pos), prevPos(pos), team(team), health(health) {}

PlayerOtherData::PlayerOtherData(T3DModel *model) :
    accel({0}),
    velocity({0}),
    direction(0),
    block({nullptr, rspq_block_free}),
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}),
    skel({new T3DSkeleton(t3d_skeleton_create(model)), t3d_skeleton_destroy}) {
        assertf(skel.get(), "Player skel is null");
        assertf(matFP.get(), "Player matrix is null");

        t3d_skeleton_update(skel.get());

        rspq_block_begin();
            t3d_matrix_push(matFP.get());
            t3d_model_draw_skinned(model, skel.get());
            t3d_matrix_pop(1);
        block = U::RSPQBlock(rspq_block_end(), rspq_block_free);
    }
