#include "map.hpp"

MapRenderer::MapRenderer() :
    surface {FMT_CI8, MapWidth, MapWidth},
    renderModeBlock {nullptr, rspq_block_free},
    paintBlock {nullptr, rspq_block_free},
    // drawBlock {nullptr, rspq_block_free},
    footstep {sprite_load("rom:/paintball/step.ia4.sprite"), sprite_free},
    splashSprites {
        {sprite_load("rom:/paintball/splash1.ia4.sprite"), sprite_free},
        {sprite_load("rom:/paintball/splash2.ia4.sprite"), sprite_free},
        {sprite_load("rom:/paintball/splash3.ia4.sprite"), sprite_free},
        {sprite_load("rom:/paintball/splash4.ia4.sprite"), sprite_free}
    },
    tlut {
        (uint16_t*)malloc_uncached(sizeof(uint16_t[256])),
        free_uncached
    }
{
    debugf("Map renderer initialized\n");
    assertf(surface.get(), "surface is null");

    mapSize = 1.f;

    rdpq_attach(surface.get(), nullptr);
        rdpq_set_scissor(0, 0, MapWidth, MapWidth);
        rdpq_clear(RGBA32(0, 0, 0, 0));
    rdpq_detach();

    // Initialize TLUT
    uint16_t *p_tlut = tlut.get();
    for (int i = 0; i < 5; i++) {
        if (i == 1) p_tlut[i] = color_to_packed16(PLAYERCOLOR_1);
        else if (i == 2) p_tlut[i] = color_to_packed16(PLAYERCOLOR_2);
        else if (i == 3) p_tlut[i] = color_to_packed16(PLAYERCOLOR_3);
        else if (i == 4) p_tlut[i] = color_to_packed16(PLAYERCOLOR_4);
        else p_tlut[i] = color_to_packed16(RGBA32(239, 239, 239, 255));
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
                .rgbaA = 0xFFFFFF'FF,
                .rgbaB = 0xFFFFFF'FF,
                .stA = {(int16_t)(pixelX << 5), (int16_t)(pixelY << 5)},
                .stB = {(int16_t)((pixelX + TileSize) << 5), (int16_t)((pixelY) << 5)},
            };
            vertices[idx * 2 + 1] = (T3DVertPacked){
                .posA = {x, 0, (int16_t)(y + SegmentSize)},
                .normA = norm,
                .posB = {(int16_t)(x + SegmentSize), 0, (int16_t)(y + SegmentSize)},
                .normB = norm,
                .rgbaA = 0xFFFFFF'FF,
                .rgbaB = 0xFFFFFF'FF,
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

    rspq_block_begin();
        t3d_frame_start();
        rdpq_sync_pipe();
        rdpq_mode_tlut(TLUT_RGBA16);
        rdpq_tex_upload_tlut(tlut.get(), 0, 5);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
        t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_TEXTURED | T3D_FLAG_DEPTH | T3D_FLAG_SHADED));

        rdpq_mode_filter(FILTER_POINT);
        rdpq_mode_persp(true);

        rdpq_mode_zbuf(false, false);

        rdpq_change_other_modes_raw(SOM_COVERAGE_DEST_MASK, SOM_COVERAGE_DEST_ZAP);
    renderModeBlock = U::RSPQBlock(rspq_block_end(), rspq_block_free);

    rspq_block_begin();
        rdpq_set_mode_standard();
        rdpq_mode_antialias(AA_NONE);
        rdpq_mode_alphacompare(1);
        rdpq_mode_combiner(RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, TEX0)));
        rdpq_mode_blender(RDPQ_BLENDER((MEMORY_RGB, 0, IN_RGB, 1)));
        rdpq_mode_filter(FILTER_BILINEAR);
    paintBlock = U::RSPQBlock(rspq_block_end(), rspq_block_free);

    // This breaks ares if used
    // rspq_block_begin();
    //     t3d_tri_draw(0, 1, 2);
    //     t3d_tri_draw(2, 1, 3);

    //     rdpq_sync_tile();
    //     rdpq_sync_load();
    //     rdpq_sync_pipe();
    // drawBlock = U::RSPQBlock(rspq_block_end(), rspq_block_free);
}

MapRenderer::~MapRenderer() {
    debugf("Map renderer de-initialized\n");

    free_uncached(vertices);
}

void MapRenderer::render(float deltaTime, const T3DFrustum &frustum) {
    for (auto splash = newSplashes.begin(); splash < newSplashes.end(); ++splash) {
        __splash(*splash);
    }
    newSplashes.clear();

    for (auto step = newFootsteps.begin(); step < newFootsteps.end(); ++step) {
        __step(*step);
    }
    newFootsteps.clear();

    rspq_block_run(renderModeBlock.get());

    for (int iy = 0; iy < MapWidth/TileSize; iy++) {
        for (int ix = 0; ix < MapWidth/TileSize; ix++ ) {
            int idx = iy * (MapWidth/TileSize) + ix;

            int halfSegmentCount = (MapWidth/TileSize)/2;
            int effectiveCount = (int)(halfSegmentCount * mapSize);
            if (effectiveCount < MinSegmentCount/2) effectiveCount = MinSegmentCount/2;

            // This assumes zero height
            bool visible = t3d_frustum_vs_aabb_s16(&frustum, vertices[idx * 2].posA, vertices[idx * 2+1].posB);
            if (!visible) continue;

            if (std::abs(ix - halfSegmentCount + 0.5f) > effectiveCount ||
                std::abs(iy - halfSegmentCount + 0.5f) > effectiveCount) {
                vertices[idx * 2].rgbaA = 0xAAAAAA'FF;
                vertices[idx * 2].rgbaB = 0xAAAAAA'FF;
                vertices[idx * 2+1].rgbaA = 0xAAAAAA'FF;
                vertices[idx * 2+1].rgbaB = 0xAAAAAA'FF;
            } else {
                vertices[idx * 2].rgbaA = 0xFFFFFF'FF;
                vertices[idx * 2].rgbaB = 0xFFFFFF'FF;
                vertices[idx * 2+1].rgbaA = 0xFFFFFF'FF;
                vertices[idx * 2+1].rgbaB = 0xFFFFFF'FF;
            }

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

void MapRenderer::__splash(Splash &splash) {
    int safeMargin = 52;
    if (splash.x > MapWidth - safeMargin) return;
    if (splash.x < safeMargin) return;
    if (splash.y > MapWidth - safeMargin) return;
    if (splash.y < safeMargin) return;

    int id = randomRange(0, SplashVariations - 1);
    surface_t s = sprite_get_pixels(splashSprites[id].get());

    rdpq_attach(surface.get(), nullptr);
        rspq_block_run(paintBlock.get());

        rdpq_set_scissor(splash.x - safeMargin, splash.y - safeMargin, splash.x + safeMargin, splash.y + safeMargin);
        rdpq_blitparms_t params {
            .width = 32,
            .height = 32,
            .flip_x = (bool)randomRange(0, 1),
            .flip_y = true,
            .cx = 16,
            .cy = 16,
            .scale_x = 0.8f + static_cast<float>(rand()) / RAND_MAX,
            .scale_y = 0.8f + static_cast<float>(rand()) / RAND_MAX,
            .theta = splash.direction,
        };
        // Set all channels to the same value b/c for an I8 target, RDP will
        // interleave R&G channels
        rdpq_set_prim_color(
            RGBA32(
                (uint8_t)(splash.team + 1),
                (uint8_t)(splash.team + 1),
                (uint8_t)(splash.team + 1),
                255
            )
        );
        rdpq_tex_blit(&s, splash.x, splash.y, &params);
    rdpq_detach();
}

void MapRenderer::__step(Splash &step) {
    if (step.x > MapWidth - 16) return;
    if (step.x < 16) return;
    if (step.y > MapWidth - 16) return;
    if (step.y < 16) return;

    surface_t s = sprite_get_pixels(footstep.get());

    rdpq_attach(surface.get(), nullptr);
        rspq_block_run(paintBlock.get());

        rdpq_set_scissor(step.x - 16, step.y - 16, step.x + 16, step.y + 16);
        rdpq_blitparms_t params {
            .width = 8,
            .height = 8,
            .flip_x = !step.isFirst,
            .flip_y = true,
            .cx = 0,
            .cy = 4,
            .theta = step.direction,

        };
        // Set all channels to the same value b/c for an I8 target, RDP will
        // interleave R&G channels
        rdpq_set_prim_color(
            RGBA32(
                (uint8_t)(step.team + 1),
                (uint8_t)(step.team + 1),
                (uint8_t)(step.team + 1),
                255
            )
        );
        rdpq_tex_blit(&s, step.x, step.y, &params);
    rdpq_detach();
}

void MapRenderer::splash(float x, float y, PlyNum team, float direction) {
    float distancePerSegment = SegmentSize * (MapWidth/TileSize);
    float finalX = (x/distancePerSegment) * MapWidth + MapWidth/2;
    float finalY = (y/distancePerSegment) * MapWidth + MapWidth/2;
    newSplashes.add(Splash {
        finalX,
        finalY,
        team,
        direction + T3D_DEG_TO_RAD(45 * static_cast<float>(rand()) / RAND_MAX),
        false
    });
}

void MapRenderer::step(float x, float y, PlyNum team, float direction, bool firstStep) {
    float distancePerSegment = SegmentSize * (MapWidth/TileSize);
    float finalX = (x/distancePerSegment) * MapWidth + MapWidth/2.f;
    float finalY = (y/distancePerSegment) * MapWidth + MapWidth/2.f;
    newFootsteps.add(Splash {
        finalX,
        finalY,
        team,
        direction,
        firstStep
    });
}

float MapRenderer::getHalfSize() {
    float size = mapSize * SegmentSize * MapWidth/TileSize;
    if (size < MinSegmentCount * SegmentSize) {
        size = MinSegmentCount * SegmentSize;
    }
    return (size / 2.f) - 10.f;
}

void MapRenderer::setSize(float size) {
    assertf(size <= 1.f && size >= 0.f, "Incorrect size");
    mapSize = size;
}