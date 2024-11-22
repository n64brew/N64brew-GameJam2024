#include "player.hpp"

Player::GameplayData::GameplayData(T3DVec3 pos, PlyNum team) :
    pos(pos), prevPos(pos), team(team), firstHit(team), temperature(0), fragCount(0) {}

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

    t3d_viewport_calc_viewspace_pos(viewport, playerOther.screenPos, billboardPos);
}

void Player::renderUI(GameplayData &playerGameplay, OtherData &playerOther, uint32_t id, sprite_t *arrowSprite)
{
    constexpr int threshold = ScreenWidth / 16;
    constexpr int margin = ScreenWidth / 16;
    int x = floorf(playerOther.screenPos.v[0]);
    int y = floorf(playerOther.screenPos.v[1]);
    float theta = 0.f;

    if (x < -threshold) {
        x = margin;
        theta = T3D_PI / 2;
    }

    if (x > (ScreenWidth+threshold) ) {
        x = (ScreenWidth-margin);
        theta = 3* T3D_PI / 2;
    }

    if (y < -threshold) {
        y = margin;
        theta = 2 * T3D_PI;
    }

    if (y > (ScreenHeight+threshold) ) {
        y = (ScreenHeight-margin);
        theta = T3D_PI;
    }

    if (theta == 0.f) {
        return;
    }

    rdpq_sync_pipe();
    rdpq_sync_tile();

    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    rdpq_set_mode_standard();
    rdpq_mode_antialias(AA_NONE);
    rdpq_mode_alphacompare(1);
    rdpq_mode_combiner(RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, TEX0)));
    rdpq_mode_filter(FILTER_BILINEAR);

    rdpq_set_prim_color(colors[playerGameplay.team]);

    rdpq_blitparms_t params {
        .width = 32,
        .height = 32,
        .cx = 16,
        .cy = 16,
        .theta = theta,
    };
    rdpq_sprite_blit(arrowSprite, x, y, &params);
}