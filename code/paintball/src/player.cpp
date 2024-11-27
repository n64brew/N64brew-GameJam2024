#include "player.hpp"

Player::GameplayData::GameplayData(T3DVec3 pos, PlyNum team) :
    pos(pos), prevPos(pos), team(team), firstHit(team), temperature(0), fragCount(0) {}

Player::OtherData::OtherData(T3DModel *model, T3DModel *shadowModel) :
    accel({0}),
    velocity({0}),
    direction(0),
    block({nullptr, rspq_block_free}),
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}),
    skel(model),
    animWalk(model, "Walk"),
    screenPos({0}),
    displayTemperature(0),
    timer(0)
    {
        debugf("Creating player\n");
        assertf(skel.get(), "Player skel is null");
        assertf(animWalk.get(), "Player animWalk is null");
        assertf(matFP.get(), "Player matrix is null");

        rspq_block_begin();
            t3d_matrix_push(matFP.get());
                rdpq_mode_zbuf(true, true);

                T3DModelIter it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
                while(t3d_model_iter_next(&it))
                {
                    if(it.object->material) {
                        t3d_model_draw_material(it.object->material, nullptr);
                        rdpq_change_other_modes_raw(SOM_COVERAGE_DEST_MASK, SOM_COVERAGE_DEST_ZAP);
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
    t3d_vec3_add(currentPos, currentPos, (T3DVec3){0, 2.f, 0});

    color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    assertf(playerOther.matFP.get(), "Player %lu matrix is null", id);
    assertf(playerOther.block.get(), "Player %lu block is null", id);
    assertf(playerOther.animWalk.get(), "Player %lu animWalk is null", id);
    assertf(playerOther.skel.get(), "Player %lu skel is null", id);

    t3d_anim_update(playerOther.animWalk.get(), deltaTime);
    t3d_skeleton_update(playerOther.skel.get());

    bool hidden = false;
    float temp = playerGameplay.temperature;
    if (temp > 1.f) {
        temp = 1.f;
        playerOther.timer += deltaTime;
        if (playerOther.timer >= 0.1) {
            hidden = true;
        }
        if (playerOther.timer >= 0.2) {
            playerOther.timer -= 0.2;
        }
    };

    if (hidden) {
        if (colors[playerGameplay.team].r == 0) colors[playerGameplay.team].r = 200;
        if (colors[playerGameplay.team].g == 0) colors[playerGameplay.team].g = 200;
        if (colors[playerGameplay.team].b == 0) colors[playerGameplay.team].b = 200;
    };

    float factor = temp > 0.5 ? temp * temp * temp * 0.02f : 0.f;

    playerOther.displayTemperature = t3d_lerp(playerOther.displayTemperature, factor, 0.2);

    t3d_mat4fp_from_srt_euler(
        playerOther.matFP.get(),
        (float[3]){0.12f+playerOther.displayTemperature, 0.12f+playerOther.displayTemperature, 0.12f+playerOther.displayTemperature},
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
    constexpr int margin = ScreenWidth / 8;
    int x = floorf(playerOther.screenPos.v[0]);
    int y = floorf(playerOther.screenPos.v[1]);
    float theta = 0.f;

    if (x < margin) {
        x = margin;
        theta = T3D_PI / 2;
    }

    if (x > (ScreenWidth-margin) ) {
        x = (ScreenWidth-margin);
        theta = 3* T3D_PI / 2;
    }

    if (y < margin) {
        y = margin;
        theta = 2 * T3D_PI;
    }

    if (y > (ScreenHeight-margin) ) {
        y = (ScreenHeight-margin);
        theta = T3D_PI;
    }

    rdpq_sync_pipe();
    rdpq_sync_tile();
    if (theta != 0.f) {
        rdpq_set_mode_standard();

        const color_t colors[] = {
            PLAYERCOLOR_1,
            PLAYERCOLOR_2,
            PLAYERCOLOR_3,
            PLAYERCOLOR_4,
        };

        rdpq_mode_zbuf(false, false);
        rdpq_mode_alphacompare(1);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
        rdpq_set_fog_color(RGBA32(0, 0, 0, 200));
        rdpq_mode_combiner(RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, TEX0)));

        rdpq_set_prim_color(colors[id]);

        rdpq_blitparms_t params {
            .width = 32,
            .height = 32,
            .cx = 16,
            .cy = 16,
            .theta = theta,
        };
        rdpq_sprite_blit(arrowSprite, x, y, &params);
    } else {
        constexpr int textHalfWidth = 10;
        constexpr int textHalfHeight = 6;
        int x = floorf(playerOther.screenPos.v[0]) - textHalfWidth;
        int y = floorf(playerOther.screenPos.v[1]) - textHalfHeight;
        rdpq_textparms_t fontParams {
            .style_id = (int16_t)id,
            .width = 20,
            .align = ALIGN_CENTER,
            .disable_aa_fix = true
        };
        rdpq_text_printf(
            &fontParams,
            SmallFont,
            x,
            y,
            "P%lu",
            id + 1
        );
    }

    // if (playerGameplay.temperature > 0.5f) {
    //     constexpr int barHalfWidth = 8;
    //     constexpr int barHeight = 3;
    //     constexpr int barYOffset = 10;

    //     int finalWidth = 2 * barHalfWidth * std::min({playerGameplay.temperature, 1.f});

    //     rdpq_sync_tile();
    //     rdpq_sync_pipe();
    //     rdpq_set_mode_fill(RGBA32(255, 0, 0, 255));

    //     rdpq_fill_rectangle(x-barHalfWidth, y-barYOffset, x-barHalfWidth+finalWidth, y-barYOffset+barHeight);
    // }
}