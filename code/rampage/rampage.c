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

rdpq_font_t* font;
#define FONT_TEXT 1

#define START_DELAY 4.0f
#define FINISH_DELAY 3.0f
#define END_SCREEN_DELAY    4.0f

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

    font = rdpq_font_load("rom:/rampage/QuirkyRobot.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){
        .color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF), .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });
    rdpq_font_style(font, 1, &(rdpq_fontstyle_t){
        .color = RGBA32(0xFF, 0x00, 0x00, 0xFF), .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });
    rdpq_font_style(font, 2, &(rdpq_fontstyle_t){
        .color = RGBA32(0x00, 0xFF, 0x00, 0xFF), .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });
    rdpq_font_style(font, 3, &(rdpq_fontstyle_t){
        .color = RGBA32(0x00, 0x00, 0xFF, 0xFF), .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });
    rdpq_font_style(font, 4, &(rdpq_fontstyle_t){
        .color = RGBA32(0xFF, 0xFF, 0x00, 0xFF), .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });

    rampage_init(&gRampage);
}

void minigame_set_active(bool is_active) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        gRampage.players[i].is_active = is_active;
    }

    for (int i = 0; i < TANK_COUNT; i += 1) {
        gRampage.tanks[i].is_active = is_active;
    }
}

bool minigame_is_done() {
    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            if (!gRampage.buildings[y][x].is_collapsing) {
                return false;
            }
        }
    }

    return true;
}

void minigame_find_winners() {
    gRampage.winner_count = 0;
    gRampage.winner_mask = 0;

    int winner_score = 0;

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        int current_score = gRampage.players[i].score;

        if (current_score > winner_score) {
            winner_score = current_score;
            gRampage.winner_count = 1;
            gRampage.winner_mask = 1 << i;
        } else if (current_score == winner_score) {
            gRampage.winner_count += 1;
            gRampage.winner_mask |= 1 << i;
        }
    }

    if (gRampage.winner_count == 4) {
        return;
    }

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        if (gRampage.winner_mask & (1 << 1)) {
            core_set_winner(i);
        }
    }
}

void minigame_fixedloop(float deltatime) {
    if (gRampage.state == RAMPAGE_STATE_START) {
        gRampage.delay_timer -= deltatime;

        if (gRampage.delay_timer < 0.0f) {
            gRampage.delay_timer = 0.0f;
            gRampage.state = RAMPAGE_STATE_PLAYING;
            minigame_set_active(true);
        }
    } else if (gRampage.state == RAMPAGE_STATE_PLAYING) {
        if (minigame_is_done()) {
            gRampage.state = RAMPAGE_STATE_FINISHED;
            gRampage.delay_timer = FINISH_DELAY;
            minigame_set_active(false);
        }
    } else if (gRampage.state == RAMPAGE_STATE_FINISHED) {
        gRampage.delay_timer -= deltatime;

        if (gRampage.delay_timer < 0.0f) {
            gRampage.state = RAMPAGE_STATE_END_SCREEN;
            gRampage.delay_timer = END_SCREEN_DELAY;
            minigame_find_winners();
        }
    } else if (gRampage.state == RAMPAGE_STATE_END_SCREEN) {
        gRampage.delay_timer -= deltatime;

        if (gRampage.delay_timer < 0.0f) {
            minigame_end();
        }
    }

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

#define ORTHO_SCALE     5.0f

uint8_t colorWhite[4] = {0xFF, 0xFF, 0xFF, 0xFF};

uint8_t pointLightColors[][4] = {
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0x80, 0x30, 0x20, 0xFF},
    {0x80, 0x30, 0x20, 0xFF},
    {0x80, 0x30, 0x20, 0xFF},
    {0x80, 0x30, 0x20, 0xFF},
};

T3DVec3 pointLightPositions[] = {
    {{0.0f, SCALE_FIXED_POINT(4.0f), 1.0f}},
    {{SCALE_FIXED_POINT(7.0f), SCALE_FIXED_POINT(4.0f), SCALE_FIXED_POINT(5.5f)}},
    {{SCALE_FIXED_POINT(-6.0f), SCALE_FIXED_POINT(4.0f), SCALE_FIXED_POINT(5.5f)}},
    {{SCALE_FIXED_POINT(-6.0f), SCALE_FIXED_POINT(4.0f), SCALE_FIXED_POINT(-1.5f)}},
    {{SCALE_FIXED_POINT(7.5f), SCALE_FIXED_POINT(4.0f), SCALE_FIXED_POINT(-6.0f)}},
};

float pointLightDistance[] = {
    1.0f,
    0.6f,
    0.6f,
    0.3f,
    0.2f,
};

struct Vector2 scorePosition[] = {
    {30.0f, 36.0f},
    {230.0f, 36.0f},
    {230.0f, 220.0f},
    {30.0f, 220.0f},
};

int get_winner_index(int index) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        if (gRampage.winner_mask & (1 << i)) {
            if (index == 0) {
                return i + 1;
            }

            --index;
        }
    }

    return 0;
}

void minigame_loop(float deltatime) {   
    uint8_t colorAmbient[4] = {0x60, 0x60, 0x60, 0xFF};

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
    t3d_screen_clear_color(RGBA32(0, 0, 0, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_count(sizeof(pointLightPositions) / sizeof(*pointLightPositions));

    for (int i = 0; i < sizeof(pointLightPositions) / sizeof(*pointLightPositions); i += 1) {
        t3d_light_set_point(i, pointLightColors[i], &pointLightPositions[i], pointLightDistance[i], false);
    }

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

    t3d_light_set_ambient(colorWhite);
    t3d_light_set_count(0);

    t3d_model_draw(rampage_assets_get()->ground);

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rdpq_text_printf(
            &(rdpq_textparms_t){
                .width = 60, 
                .align = scorePosition[i].x < 160.0f ? ALIGN_LEFT : ALIGN_RIGHT,
                .style_id = i + 1,
            }, FONT_TEXT, 
            scorePosition[i].x, scorePosition[i].y, 
            "%d",
            gRampage.players[i].score
        );
    }

    if (gRampage.state == RAMPAGE_STATE_START) {
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,TEX0), (0,0,0,TEX0)));
        rdpq_sprite_blit(
            rampage_assets_get()->countdown_numbers[(int)ceilf(gRampage.delay_timer)],
            160 - 9,
            120 - 16,
            NULL
        );
    }

    if (gRampage.state == RAMPAGE_STATE_END_SCREEN) {
        switch (gRampage.winner_count) {
            case 1:
                rdpq_text_printf(
                    &(rdpq_textparms_t){
                        .width = 320.0f, 
                        .align = ALIGN_CENTER,
                        .style_id = get_winner_index(0),
                    }, FONT_TEXT, 
                    0.0f, 120.0f, 
                    "Player %d wins!",
                    get_winner_index(0)
                );
                break;
            case 2:
                rdpq_text_printf(
                    &(rdpq_textparms_t){
                        .width = 320.0f, 
                        .align = ALIGN_CENTER,
                        .style_id = 0,
                    }, FONT_TEXT, 
                    0.0f, 120.0f, 
                    "Players %d and %d wins!",
                    get_winner_index(0),
                    get_winner_index(1)
                );
                break;
            case 3:
                rdpq_text_printf(
                    &(rdpq_textparms_t){
                        .width = 320.0f, 
                        .align = ALIGN_CENTER,
                        .style_id = 0,
                    }, FONT_TEXT, 
                    0.0f, 120.0f, 
                    "Player %d, %d and %d wins!",
                    get_winner_index(0),
                    get_winner_index(1),
                    get_winner_index(2)
                );
                break;
            case 0:
            case 4:
                rdpq_text_printf(
                    &(rdpq_textparms_t){
                        .width = 320.0f, 
                        .align = ALIGN_CENTER,
                        .style_id = 0,
                    }, FONT_TEXT, 
                    0.0f, 120.0f, 
                    "Draw"
                );
                break;
        }
    }

    rdpq_detach_show();
}

void minigame_cleanup() {
    rampage_destroy(&gRampage);
    t3d_destroy();
    collision_scene_destroy();
    health_destroy();
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
}

static struct Vector3 gStartingPositions[] = {
    {SCALE_FIXED_POINT(-8.0f), 0.0f, SCALE_FIXED_POINT(-1.5f)},
    {SCALE_FIXED_POINT(3.0f), 0.0f, SCALE_FIXED_POINT(-6.5f)},
    {SCALE_FIXED_POINT(8.0f), 0.0f, SCALE_FIXED_POINT(1.5f)},
    {SCALE_FIXED_POINT(-3.0f), 0.0f, SCALE_FIXED_POINT(6.5f)},
};

static struct Vector2 gStartingRotations[] = {
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {0.0f, -1.0f},
    {-1.0f, 0.0f},
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
        rampage_player_init(&rampage->players[i], &gStartingPositions[i], &gStartingRotations[i], i, rampage_player_type(i));
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

    rampage->state = RAMPAGE_STATE_START;
    rampage->delay_timer = START_DELAY;
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