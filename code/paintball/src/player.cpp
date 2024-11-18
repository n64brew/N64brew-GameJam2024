#include "player.hpp"

Player::GameplayData::GameplayData(T3DVec3 pos, PlyNum team) :
    pos(pos), prevPos(pos), team(team), firstHit(team), temperature(0) {}

Player::OtherData::OtherData(T3DModel *model, T3DModel *shadowModel) :
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

                T3DModelIter it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
                while(t3d_model_iter_next(&it))
                {
                    if(it.object->material) {
                        t3d_model_draw_material(it.object->material, nullptr);
                    }
                    t3d_model_draw_object(it.object, skel.get()->boneMatricesFP);
                }

                // Outline
                t3d_state_set_vertex_fx(T3D_VERTEX_FX_OUTLINE, (int16_t)8, (int16_t)8);
                    rdpq_set_prim_color(RGBA32(0, 0, 0, 0xFF));

                    // Is this necessary?
                    rdpq_sync_pipe();

                    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
                    t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_CULL_FRONT | T3D_FLAG_DEPTH));

                    it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
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


void Player::render(Player::GameplayData &playerGameplay, Player::OtherData &playerOther, uint32_t id, T3DViewport &viewport, float deltaTime)
{
    double interpolate = core_get_subtick();
    T3DVec3 currentPos {0};
    t3d_vec3_lerp(currentPos, playerGameplay.prevPos, playerGameplay.pos, interpolate);
    t3d_vec3_add(currentPos, currentPos, (T3DVec3){0, 0, 0});

    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    assertf(playerOther.matFP.get(), "Player %lu matrix is null", id);
    assertf(playerOther.block.get(), "Player %lu block is null", id);

    t3d_anim_update(playerOther.animWalk.get(), deltaTime);
    t3d_skeleton_update(playerOther.skel.get());

    t3d_mat4fp_from_srt_euler(
        playerOther.matFP.get(),
        (float[3]){0.12f, 0.12f, 0.12f},
        (float[3]){0.0f, playerOther.direction, 0},
        currentPos.v
    );

    rdpq_sync_pipe();
    rdpq_set_prim_color(colors[playerGameplay.team]);

    rdpq_set_env_color(colors[playerGameplay.firstHit]);
    rspq_block_run(playerOther.block.get());

    T3DVec3 billboardPos = (T3DVec3){{
        currentPos.v[0],
        currentPos.v[1] + 40,
        currentPos.v[2]
    }};

    t3d_viewport_calc_viewspace_pos(viewport,  playerOther.screenPos, billboardPos);
}

void Player::renderUI(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id)
{
    constexpr int margin = ScreenWidth / 16;
    constexpr int textHalfWidth = 10;
    constexpr int textHalfHeight = 6;
    int x = floorf(playerOther.screenPos.v[0]) - textHalfWidth;
    int y = floorf(playerOther.screenPos.v[1]) - textHalfHeight;

    if (x < margin) x = margin;
    if (x > (ScreenWidth-margin-2*textHalfWidth) ) x = (ScreenWidth-margin-2*textHalfWidth);

    if (y < (margin + textHalfHeight)) y = (margin + textHalfHeight);
    if (y > (ScreenHeight-margin-2*textHalfHeight) ) y = (ScreenHeight-margin-2*textHalfHeight);

    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise

    rdpq_textparms_t fontParams {
        .style_id = playerGameplay.team,
        .width = 20,
        .align = ALIGN_CENTER, .disable_aa_fix = true };
    rdpq_text_printf(
        &fontParams,
        MainFont,
        x,
        y,
        "P%lu %4.2f",
        id + 1,
        playerGameplay.temperature
    );
}