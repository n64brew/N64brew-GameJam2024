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

#include "./mallard.h"
#include "./assets.h"

surface_t *depthBuffer;
T3DViewport viewport;

T3DVec3 camPos = {{0.0f, SCALE_FIXED_POINT(12.0f), SCALE_FIXED_POINT(4.0f)}};
T3DVec3 camTarget = {{0.0f, 0.0f, 0.0f}};

struct Mallard gMallard;

const MinigameDef minigame_def = {
    .gamename = "Mallard 64",
    .developername = "Josh Kautz",
    .description = "Migration is for cowards. Keep the temperature warm!",
    .instructions = "Press A to poop at snowmen!"
};

void minigame_init() {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    t3d_init((T3DInitParams){});

    depthBuffer = display_get_zbuf();
    viewport = t3d_viewport_create();

    mallard_init(&gMallard);
}

void minigame_fixedloop(float deltatime) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        mallard_player_update(&gMallard.players[i], deltatime);
    }
}

void minigame_loop(float deltatime) {   
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    T3DVec3 lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), SCALE_FIXED_POINT(6.0f), SCALE_FIXED_POINT(16.0f));
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
        mallard_player_render(&gMallard.players[i]);
    }

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            mallard_building_render(&gMallard.buildings[y][x]);
        }
    }

    rdpq_detach_show();
}

void minigame_cleanup() {
    mallard_destroy(&gMallard);
    t3d_destroy();
}

static T3DVec3 gStartingPositions[] = {
    {{SCALE_FIXED_POINT(-8.0f), 0.0f, SCALE_FIXED_POINT(-8.0f)}},
    {{SCALE_FIXED_POINT(8.0f), 0.0f, SCALE_FIXED_POINT(-8.0f)}},
    {{SCALE_FIXED_POINT(-8.0f), 0.0f, SCALE_FIXED_POINT(8.0f)}},
    {{SCALE_FIXED_POINT(8.0f), 0.0f, SCALE_FIXED_POINT(8.0f)}},
};

enum PlayerType mallard_player_type(int index) {
    if (index < core_get_playercount()) {
        return (enum PlayerType)index;
    } else {
        return (enum PlayerType)(PLAYER_TYPE_EASY + core_get_aidifficulty());
    }
}

#define BUILDING_SPACING    3.0f

void mallard_init(struct Mallard* mallard) {
    mallard_assets_init();

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        mallard_player_init(&mallard->players[i], &gStartingPositions[i], mallard_player_type(i));
    }

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            T3DVec3 position = {{
                SCALE_FIXED_POINT((x - (BUILDING_COUNT_X - 1) * 0.5f) * BUILDING_SPACING),
                0.0f,
                SCALE_FIXED_POINT((y - (BUILDING_COUNT_Y - 1) * 0.5f) * BUILDING_SPACING),
            }};
            mallard_building_init(&mallard->buildings[y][x], &position);
        }
    }
}

void mallard_destroy(struct Mallard* mallard) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        mallard_player_destroy(&mallard->players[i]);
    }

    mallard_assets_destroy();
}