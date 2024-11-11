#include "map.hpp"

MapRenderer::MapRenderer() :
    surface {FMT_I8, MapWidth, MapHeight},
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
        (float[3]){0.3f, 0.3f, 0.3f},
        (float[3]){0, 0, 0},
        (float[3]){0, 0, -10}
    );

    rspq_block_begin();
        // t3d_matrix_push(matFP.get());
        // t3d_matrix_pop(1);
    block = U::RSPQBlock(rspq_block_end(), rspq_block_free);

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

    splash(16, 16, PLAYER_1);
    splash(20, 20, PLAYER_2);
    splash(25, 25, PLAYER_3);
}

void MapRenderer::render() {
    // This is just a hack to convince the validator
    surface_t _surface = surface_make(surface.get()->buffer, FMT_CI8, MapWidth, MapHeight, TEX_FORMAT_PIX2BYTES(FMT_CI8, MapWidth));

    rdpq_blitparms_t params {
        .width = 64,
        .height = 64,
    };

    // TODO: use a block
    rdpq_set_mode_standard();
    rdpq_sync_tile();
    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(tlut.get(), 0, 5);
    rdpq_tex_blit(&_surface, 0, 0, &params);
}

void MapRenderer::splash(int x, int y, PlyNum player) {
    surface_t s = sprite_get_pixels(sprite.get());

    // TODO: move this to its own block & batch all the blits
    rdpq_attach(surface.get(), nullptr);
        rdpq_set_mode_standard();
        rdpq_clear(RGBA32(0, 0, 0, 0));
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

    // rdpq_detach_wait();
    // for (int y = 0; y < 32; y++) {
    //     for (int x = 0; x < 32; x++) {
    //         int i = x + y * surface.get()->width;
    //         debugf("%02x ", ((uint8_t*)surface.get()->buffer)[i]);
    //     }
    //     debugf("\n");
    // }
}