#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include <stdio.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#include "./rampage.h"
#include "./collision/collision_scene.h"
#include "./health.h"
#include "./assets.h"
#include "./rampage.h"

surface_t *depthBuffer;
T3DViewport viewport;

T3DVec3 camPos = {{SCALE_FIXED_POINT(3.0f), SCALE_FIXED_POINT(5.0f), SCALE_FIXED_POINT(4.0f)}};
T3DVec3 camTarget = {{0.0f, 0.0f, 0.0f}};

struct Rampage gRampage;

const MinigameDef minigame_def = {
    .gamename = "Rampage",
    .developername = "Ultrarare",
    .description = "Destroy the most buildings to win",
    .instructions = "Press A to win."
};

static struct mesh_triangle_indices global_mesh_collider_triangles[] = {
    {.indices = {0, 1, 2}},
    {.indices = {0, 2, 3}},
};

static struct Vector3 global_mesh_collider_vertices[] = {
    {SCALE_FIXED_POINT(-20.0f), 0.0f, SCALE_FIXED_POINT(-20.0f)},
    {SCALE_FIXED_POINT(20.0f), 0.0f, SCALE_FIXED_POINT(-20.0f)},
    {SCALE_FIXED_POINT(20.0f), 0.0f, SCALE_FIXED_POINT(20.0f)},
    {SCALE_FIXED_POINT(-20.0f), 0.0f, SCALE_FIXED_POINT(20.0f)},  
};

static struct mesh_index_block global_mesh_collider_blocks[] = {
    {.first_index = 0, .last_index = 2},
};

static uint16_t global_mesh_collider_indices[] = {
    0, 1,
};

static struct mesh_collider global_mesh_collider = {
    .triangle_count = 2,
    .triangles = global_mesh_collider_triangles,
    .vertices = global_mesh_collider_vertices,

    .index = {
        .block_count = {1, 1, 1},
        .min = {SCALE_FIXED_POINT(-20.0f), 0.0f, SCALE_FIXED_POINT(-20.0f)},
        .stride_inv = {1.0f / SCALE_FIXED_POINT(40.0f), 1.0f, 1.0f / SCALE_FIXED_POINT(40.0f)},
        .blocks = global_mesh_collider_blocks,
        .index_indices = global_mesh_collider_indices,
    }
};

void minigame_init() {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    t3d_init((T3DInitParams){});

    collision_scene_init();
    health_init();
    collision_scene_use_static_collision(&global_mesh_collider);

    depthBuffer = display_get_zbuf();
    viewport = t3d_viewport_create();

    rampage_init(&gRampage);
}

void minigame_fixedloop(float deltatime) {
    collision_scene_collide(deltatime);

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_update(&gRampage.players[i], deltatime);
    }

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            rampage_building_update(&gRampage.buildings[y][x], deltatime);
        }
    }

    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_update(&gRampage.tanks[i], deltatime);
    }
}

#define ORTHO_SCALE     7.0f

void minigame_loop(float deltatime) {   
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    T3DVec3 lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    t3d_viewport_set_ortho(
        &viewport, 
        SCALE_FIXED_POINT(-ORTHO_SCALE * 1.5f), SCALE_FIXED_POINT(ORTHO_SCALE * 1.5f),
        SCALE_FIXED_POINT(-ORTHO_SCALE), SCALE_FIXED_POINT(ORTHO_SCALE),
        SCALE_FIXED_POINT(-1.0f), SCALE_FIXED_POINT(20.0f)
    );
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    rdpq_attach(display_get(), depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_render(&gRampage.players[i]);
    }

    rspq_block_run(rampage_assets_get()->buildingSplit.material);
    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            rampage_building_render(&gRampage.buildings[y][x]);
        }
    }

    rspq_block_run(rampage_assets_get()->tankSplit.material);
    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_render(&gRampage.tanks[i]);
    }

    t3d_model_draw(rampage_assets_get()->ground);

    rdpq_detach_show();
}

void minigame_cleanup() {
    rampage_destroy(&gRampage);
    t3d_destroy();
    collision_scene_destroy();
    health_destroy();
}

static struct Vector3 gStartingPositions[] = {
    {SCALE_FIXED_POINT(-8.0f), 0.0f, SCALE_FIXED_POINT(-8.0f)},
    {SCALE_FIXED_POINT(8.0f), 0.0f, SCALE_FIXED_POINT(-8.0f)},
    {SCALE_FIXED_POINT(-8.0f), 0.0f, SCALE_FIXED_POINT(8.0f)},
    {SCALE_FIXED_POINT(8.0f), 0.0f, SCALE_FIXED_POINT(8.0f)},
};

static struct Vector3 gStartingTankPositions[] = {
    {SCALE_FIXED_POINT(-1.5f), 0.0f, SCALE_FIXED_POINT(-3.0f)},
    {SCALE_FIXED_POINT(1.5f), 0.0f, SCALE_FIXED_POINT(-3.0f)},
    {SCALE_FIXED_POINT(-1.5f), 0.0f, SCALE_FIXED_POINT(3.0f)},
    {SCALE_FIXED_POINT(1.5f), 0.0f, SCALE_FIXED_POINT(3.0f)},
};

enum PlayerType rampage_player_type(int index) {
    if (index < core_get_playercount()) {
        return (enum PlayerType)index;
    } else {
        return (enum PlayerType)(PLAYER_TYPE_EASY + core_get_aidifficulty());
    }
}

void rampage_init(struct Rampage* rampage) {
    rampage_assets_init();

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_init(&rampage->players[i], &gStartingPositions[i], i, rampage_player_type(i));
    }

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            T3DVec3 position = {{
                (x - (BUILDING_COUNT_X - 1) * 0.5f) * BUILDING_SPACING,
                0.0f,
                (y - (BUILDING_COUNT_Y - 1) * 0.5f) * BUILDING_SPACING,
            }};
            rampage_building_init(&rampage->buildings[y][x], &position);
        }
    }

    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_init(&gRampage.tanks[i], &gStartingTankPositions[i]);
    }
}

void rampage_destroy(struct Rampage* rampage) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_destroy(&rampage->players[i]);
    }

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            rampage_building_destroy(&rampage->buildings[y][x]);
        }
    }

    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_destroy(&rampage->tanks[i]);
    }

    rampage_assets_destroy();
}