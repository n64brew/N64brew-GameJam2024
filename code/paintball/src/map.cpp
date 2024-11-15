#include "map.hpp"

MapRenderer::MapRenderer() :
    surface {FMT_CI8, MapWidth, MapWidth},
    block {nullptr, rspq_block_free},
    sprite {sprite_load("rom:/paintball/splash.ia4.sprite"), sprite_free},
    tlut {
        (uint16_t*)malloc_uncached(sizeof(uint16_t[256])),
        free_uncached
    },
    newSplashCount(0)
{
    debugf("Map renderer initialized\n");
    assertf(surface.get(), "surface is null");

    rdpq_attach(surface.get(), nullptr);
        rdpq_set_scissor(0, 0, MapWidth, MapWidth);
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

            int16_t x = ix * SegmentSize-SegmentSize * MapWidth/TileSize / 2;
            int16_t y = iy * SegmentSize-SegmentSize * MapWidth/TileSize / 2;

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

    // for (int i = 0; i < MapWidth/TileSize * MapWidth/TileSize; i++) {
    //     int x = i % (MapWidth/TileSize) * TileSize;
    //     int y = i / (MapWidth/TileSize) * TileSize;
    //     __splash(x + 16, y + 16, PLAYER_1);
    // }
}

MapRenderer::~MapRenderer() {
    debugf("Map renderer de-initialized\n");

    free_uncached(vertices);
}

void MapRenderer::render(const T3DViewport &viewport) {
    for (std::size_t i = 0; i < newSplashCount; i++) {
        float distancePerSegment = SegmentSize * (MapWidth/TileSize);
        int finalX = (newSplashes[i].x/distancePerSegment) * MapWidth + MapWidth/2;
        int finalY = (newSplashes[i].y/distancePerSegment) * MapWidth + MapWidth/2;
        __splash(finalX, finalY, newSplashes[i].team);
    }
    newSplashCount = 0;

    t3d_frame_start();

    // Moving the following to a block is crashing ares for some reason
    rdpq_sync_pipe();
    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(tlut.get(), 0, 5);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX);
    t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_TEXTURED | T3D_FLAG_DEPTH));

    rdpq_mode_filter(FILTER_POINT);
    rdpq_mode_persp(true);

    for (int iy = 0; iy < MapWidth/TileSize; iy++) {
        for (int ix = 0; ix < MapWidth/TileSize; ix++ ) {
            int idx = iy * (MapWidth/TileSize) + ix;

            // This assumes zero height
            bool visible = t3d_frustum_vs_aabb_s16(&viewport.viewFrustum, vertices[idx * 2].posA, vertices[idx * 2+1].posB);
            if (!visible) continue;

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
}

void MapRenderer::__splash(int x, int y, PlyNum player) {
    if (x > MapWidth - 16) return;
    if (x < 16) return;
    if (y > MapWidth - 16) return;
    if (y < 16) return;

    surface_t s = sprite_get_pixels(sprite.get());

    // TODO: move this to its own block & batch all the blits
    rdpq_attach(surface.get(), nullptr);
        rdpq_set_mode_standard();
        rdpq_mode_antialias(AA_NONE);
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
    rdpq_detach();
}

void MapRenderer::splash(float x, float y, PlyNum team) {
    if (newSplashCount >= newSplashes.size()) return;

    newSplashes[newSplashCount].x = x;
    newSplashes[newSplashCount].y = y;
    newSplashes[newSplashCount].team = team;
    newSplashCount++;
}