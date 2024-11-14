#include "map.hpp"

MapRenderer::MapRenderer() :
    surface {FMT_CI8, MapWidth, MapWidth},
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
    assertf(surface.get(), "surface is null");
    assertf(matFP.get(), "Map matrix is null");

    t3d_mat4fp_from_srt_euler(
        matFP.get(),
        T3DVec3 {1.f, 1.f, 1.f},
        T3DVec3 {0 , 0 , 0},
        T3DVec3 {-SegmentSize * MapWidth/TileSize / 2, 0, -SegmentSize * MapWidth/TileSize / 2}
    );

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
    vertices = (T3DVertPacked*)malloc_uncached(sizeof(T3DVertPacked) * (MapWidth/TileSize) * (MapWidth/TileSize) * 2);

    auto normalDir = T3DVec3 {{ 0, 1, 0 }};
    uint16_t norm = t3d_vert_pack_normal(&normalDir); // normals are packed in a 5.6.5 format

    for (int iy = 0; iy < MapWidth/TileSize; iy++) {
        for (int ix = 0; ix < MapWidth/TileSize; ix++) {
            int idx = iy * (MapWidth/TileSize) + ix;

            int pixelX = ix * TileSize;
            int pixelY = iy * TileSize;

            int16_t x = ix * SegmentSize;
            int16_t y = iy * SegmentSize;

            debugf("idx: %d, 0: %d:%d 1: %d:%d\n", idx, x, y, (int16_t)(x + SegmentSize), y);
            debugf("        2: %d:%d 3: %d:%d\n", x, (int16_t)(y + SegmentSize), (int16_t)(x + SegmentSize), (int16_t)(y + SegmentSize));

            // TODO: convert to tri strip
            vertices[idx * 2] = (T3DVertPacked){
                .posA = {x, 0, y},
                .normA = norm,
                .posB = {(int16_t)(x + SegmentSize), 0, y},
                .normB = norm,
                .rgbaA = 0xFF0000'FF,
                .rgbaB = 0x00FF00'FF,
                .stA = {(int16_t)(pixelX << 5), (int16_t)(pixelY << 5)},
                .stB = {(int16_t)((pixelX + TileSize) << 5), (int16_t)((pixelY) << 5)},
            };
            vertices[idx * 2 + 1] = (T3DVertPacked){
                .posA = {x, 0, (int16_t)(y + SegmentSize)},
                .normA = norm,
                .posB = {(int16_t)(x + SegmentSize), 0, (int16_t)(y + SegmentSize)},
                .normB = norm,
                .rgbaA = 0x0000FF'FF,
                .rgbaB = 0xFF00FF'FF,
                .stA = {(int16_t)((pixelX) << 5), (int16_t)((pixelY + TileSize) << 5)},
                .stB = {(int16_t)((pixelX + TileSize) << 5), (int16_t)((pixelY + TileSize) << 5)},
            };
        }
    }

    for (int i = 0; i < MapWidth/TileSize * MapWidth/TileSize; i++) {
        int x = i % (MapWidth/TileSize) * TileSize;
        int y = i / (MapWidth/TileSize) * TileSize;
        splash(x + 16, y + 16, PLAYER_1);
    }


    rspq_block_begin();
        rdpq_set_mode_standard();

        rdpq_sync_pipe();
        rdpq_mode_tlut(TLUT_RGBA16);
        rdpq_tex_upload_tlut(tlut.get(), 0, 5);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_TEXTURED | T3D_FLAG_DEPTH));

        rdpq_mode_filter(FILTER_POINT);
        rdpq_mode_persp(true);

        t3d_matrix_push(matFP.get());
        for (int iy = 0; iy < MapWidth/TileSize; iy++) {
            for (int ix = 0; ix < MapWidth/TileSize; ix++ ) {
                int idx = iy * (MapWidth/TileSize) + ix;

                int pixelX = ix * TileSize;
                int pixelY = iy * TileSize;
                rdpq_sync_tile();
                rdpq_sync_load();
                rdpq_sync_pipe();

                rdpq_tex_upload_sub(TILE0, surface.get(), NULL, pixelX, pixelY, pixelX+TileSize, pixelY+TileSize);

                // TODO: this is not efficient, load more vertices
                t3d_vert_load(&vertices[idx * 2], 0, 4);

                t3d_tri_draw(0, 1, 2);
                t3d_tri_draw(2, 1, 3);
            }
        }
        t3d_matrix_pop(1);
    block = U::RSPQBlock(rspq_block_end(), rspq_block_free);

}

MapRenderer::~MapRenderer() {
    free_uncached(vertices);
}

void MapRenderer::render() {
    rspq_block_run(block.get());
}

void MapRenderer::splash(int x, int y, PlyNum player) {
    surface_t s = sprite_get_pixels(sprite.get());

    // TODO: move this to its own block & batch all the blits
    rdpq_attach(surface.get(), nullptr);
        rdpq_set_mode_standard();
        rdpq_mode_antialias(AA_NONE);
        // TODO: make sure this doesn't exceed the bounds
        rdpq_set_scissor(x - 16, y - 16, x + 16, y + 16);
        rdpq_mode_alphacompare(1);
        rdpq_mode_combiner(RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, TEX0)));
        rdpq_mode_blender(RDPQ_BLENDER((MEMORY_RGB, 0, IN_RGB, 1)));
        rdpq_mode_filter(FILTER_BILINEAR);
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
        // rdpq_sync_pipe();
        // rdpq_sync_tile();
        // rdpq_sync_load();
    rdpq_detach();
}