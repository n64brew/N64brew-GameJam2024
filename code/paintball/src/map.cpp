#include "map.hpp"

MapRenderer::MapRenderer() :
    surface {FMT_CI8, MapWidth, MapHeight},
    matFP {
        (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)),
        free_uncached
    },
    block {nullptr, rspq_block_free},
    sprite {sprite_load("rom:/paintball/splash.ia4.sprite"), sprite_free},
    tlut {
        (uint16_t*)malloc_uncached(sizeof(uint16_t[256])),
        free_uncached
    }
{
    assertf(matFP.get(), "Map matrix is null");

    t3d_mat4fp_from_srt_euler(
        matFP.get(),
        (float[3]){1.f, 1.f, 1.f},
        (float[3]){0, 0, 0},
        (float[3]){0, 0, 0}
    );

    // rspq_block_begin();

    // block = U::RSPQBlock(rspq_block_end(), rspq_block_free);

    rdpq_attach(surface.get(), nullptr);
        rdpq_set_scissor(0, 0, 320, 240);
        rdpq_clear(RGBA32(0, 0, 0, 0));
    rdpq_detach();

    // Initialize TLUT
    uint16_t *p_tlut = tlut.get();
    for (int i = 0; i < 4; i++) {
        if (i == 1) p_tlut[i] = color_to_packed16(PLAYERCOLOR_1);
        else if (i == 2) p_tlut[i] = color_to_packed16(PLAYERCOLOR_2);
        else if (i == 3) p_tlut[i] = color_to_packed16(PLAYERCOLOR_3);
        else if (i == 4) p_tlut[i] = color_to_packed16(PLAYERCOLOR_4);
        else p_tlut[i] = color_to_packed16(RGBA32(255, 255, 255, 255));
    }

    // TODO: use managed memory
    vertices = (T3DVertPacked*)malloc_uncached(sizeof(T3DVertPacked) * 2);

    auto normalDir = T3DVec3 {{ 0, 1, 0}};
    uint16_t norm = t3d_vert_pack_normal(&normalDir); // normals are packed in a 5.6.5 format
    vertices[0] = (T3DVertPacked){
        .posA = {0, 0, 0},
        .normA = norm,
        .posB = { 100, 0, 0},
        .normB = norm,
        .rgbaA = 0xFF0000'FF,
        .rgbaB = 0x00FF00'FF,
        .stA = {0, 0},
        .stB = {64 << 5, 0},
    };
    vertices[1] = (T3DVertPacked){
        .posA = { 0, 0, 100},
        .normA = norm,
        .posB = {100, 0, 100},
        .normB = norm,
        .rgbaA = 0x0000FF'FF,
        .rgbaB = 0xFF00FF'FF,
        .stA = {0, 64 << 5},
        .stB = {64 << 5, 64 << 5},
    };

    splash(16, 16, PLAYER_1);
    splash(20, 20, PLAYER_2);
    splash(25, 25, PLAYER_3);
}

void MapRenderer::render() {
    rdpq_blitparms_t params {
        .width = 64,
        .height = 64,
    };

    assertf(surface.get(), "surface is null");
    assertf(matFP.get(), "Map matrix is null");

    rdpq_set_mode_standard();
    rdpq_sync_tile();

    rdpq_tex_upload_sub(TILE0, surface.get(), NULL, 0, 0, 32, 32);
    rdpq_tex_upload_sub(TILE1, surface.get(), NULL, 32, 0, 32, 32);

    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(tlut.get(), 0, 5);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX);
    t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_TEXTURED | T3D_FLAG_DEPTH));


    t3d_matrix_push(matFP.get());
        t3d_vert_load(vertices, 0, 4); // load 4 vertices...
    t3d_matrix_pop(1);
    t3d_tri_draw(0, 1, 2);
    t3d_tri_draw(2, 1, 3);
}

void MapRenderer::splash(int x, int y, PlyNum player) {
    surface_t s = sprite_get_pixels(sprite.get());

    // TODO: move this to its own block & batch all the blits
    rdpq_attach(surface.get(), nullptr);
        rdpq_set_mode_standard();
        rdpq_mode_antialias(AA_NONE);
        rdpq_mode_filter(FILTER_POINT);
        rdpq_set_scissor(0, 0, 32, 32);
        rdpq_mode_alphacompare(1);
        rdpq_mode_combiner(RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, TEX0)));
        rdpq_mode_blender(RDPQ_BLENDER((MEMORY_RGB, 0, IN_RGB, 1)));
        rdpq_blitparms_t params {
            .width = 32,
            .height = 32,
            .cx = 16,
            .cy = 16,
            // TODO: use the firing direction + some randomness
            .theta = 2 * T3D_PI * static_cast<float>(rand()) / RAND_MAX,
        };
        // Set all channels to the same value b/c for an I8 target, RDP will
        // interleave R&G channels
        rdpq_set_prim_color(
            RGBA32(
                (uint8_t)(player + 1),
                (uint8_t)(player + 1),
                (uint8_t)(player + 1),
                255
            )
        );
        rdpq_tex_blit(&s, x, y, &params);
    rdpq_detach();
}