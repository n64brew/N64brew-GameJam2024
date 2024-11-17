#include "player.hpp"

PlayerGameplayData::PlayerGameplayData(T3DVec3 pos, PlyNum team, std::array<int, MAXPLAYERS> health) :
    pos(pos), prevPos(pos), team(team), health(health), temperature(0) {}

PlayerOtherData::PlayerOtherData(T3DModel *model, T3DModel *shadowModel) :
    accel({0}),
    velocity({0}),
    direction(0),
    block({nullptr, rspq_block_free}),
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}),
    skel({new T3DSkeleton(t3d_skeleton_create(model)), t3d_skeleton_destroy}),
    animWalk({new T3DAnim(t3d_anim_create(model, "Walk")), t3d_anim_destroy}),
    screenPos({0})
    {
        assertf(skel.get(), "Player skel is null");
        assertf(matFP.get(), "Player matrix is null");

        rspq_block_begin();
            t3d_matrix_push(matFP.get());

                rdpq_mode_zbuf(true, true);

                t3d_model_draw_skinned(model, skel.get());

                // Outline
                t3d_state_set_vertex_fx(T3D_VERTEX_FX_OUTLINE, (int16_t)8, (int16_t)8);
                    rdpq_set_prim_color(RGBA32(0, 0, 0, 0xFF));

                    // Is this necessary?
                    rdpq_sync_pipe();

                    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
                    t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_CULL_FRONT | T3D_FLAG_DEPTH));

                    T3DModelIter it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
                    while(t3d_model_iter_next(&it))
                    {
                        t3d_model_draw_object(it.object, skel.get()->boneMatricesFP);
                    }
                t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);

                rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
                t3d_model_draw(shadowModel);

            t3d_matrix_pop(1);
        block = U::RSPQBlock(rspq_block_end(), rspq_block_free);

        t3d_anim_attach(animWalk.get(), skel.get());
    }
