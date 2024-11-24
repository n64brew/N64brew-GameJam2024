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
#include "./math/mathf.h"

#define HIGH_RES    1

surface_t *depthBuffer;
T3DViewport viewport;

T3DVec3 camPos = {{SCALE_FIXED_POINT(3.0f), SCALE_FIXED_POINT(5.0f), SCALE_FIXED_POINT(4.0f)}};
T3DVec3 camTarget = {{0.0f, 0.0f, 0.0f}};

rdpq_font_t* font;
#define FONT_TEXT 1

#define START_DELAY 0.5f
// #define START_DELAY 4.5f
#define FINISH_DELAY 3.0f
#define END_SCREEN_DELAY    4.0f
#define DESTROY_TITLE_TIME  2.0f

struct Rampage gRampage;

const MinigameDef minigame_def = {
    .gamename = "Rampage",
    .developername = "Ultrarare",
    .description = "Destroy the most buildings to win",
    .instructions = "Press B to attack."
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
    display_init(HIGH_RES ? RESOLUTION_640x240 : RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
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

    wav64_play(&rampage_assets_get()->startJingle, 2);
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
        gRampage.winner_count = 0;
        gRampage.winner_mask = 0;
    }
    
    struct Vector2 lookRotation = gRight2;
    vector2ComplexFromAngle(0.64f, &lookRotation);

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        if (gRampage.winner_mask & (1 << i)) {
            core_set_winner(i);
            rampage_player_set_did_win(&gRampage.players[i], true);
        } else {
            rampage_player_set_did_win(&gRampage.players[i], false);
        }

        gRampage.players[i].dynamic_object.rotation = lookRotation;        
    }
}

void minigame_fixedloop(float deltatime) {
    if (gRampage.state == RAMPAGE_STATE_START) {
        float before = gRampage.delay_timer;

        gRampage.delay_timer -= deltatime;

        if (floorf(before) != floorf(gRampage.delay_timer) && gRampage.delay_timer <= 3.0f) {
            wav64_play(&rampage_assets_get()->countdownSound, 2);
        }

        if (gRampage.delay_timer < 0.0f) {
            gRampage.delay_timer = DESTROY_TITLE_TIME;
            gRampage.state = RAMPAGE_STATE_PLAYING;
            minigame_set_active(true);
            wav64_set_loop(&rampage_assets_get()->music, true);
            wav64_play(&rampage_assets_get()->music, 1);
        }
    } else if (gRampage.state == RAMPAGE_STATE_PLAYING) {
        gRampage.delay_timer -= deltatime;

        if (minigame_is_done()) {
            gRampage.state = RAMPAGE_STATE_FINISHED;
            gRampage.delay_timer = FINISH_DELAY;
            minigame_set_active(false);
            mixer_ch_stop(1);
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

    props_check_collision(&gRampage.props, &gRampage.players[0].dynamic_object);

    // for (int i = 0; i < PLAYER_COUNT; i += 1) {
    //     props_check_collision(&gRampage.props, &gRampage.players[i].dynamic_object);
    // }

    // for (int i = 0; i < TANK_COUNT; i += 1) {
    //     props_check_collision(&gRampage.props, &gRampage.tanks[i].dynamic_object);
    // }

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

#define ORTHO_SCALE     6.0f

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

#define SCREEN_WIDTH    (HIGH_RES ? 640 : 320)
// #define SCREEN_HEIGHT   (HIGH_RES ? 480 : 240)
#define SCREEN_HEIGHT   240

struct Vector2 scorePosition[] = {
    {30.0f, 52.0f},
    {SCREEN_WIDTH - 90, 52.0f},
    {SCREEN_WIDTH - 90, SCREEN_HEIGHT - 20},
    {30.0f, SCREEN_HEIGHT - 20},
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

    t3d_light_set_ambient(colorWhite);
    t3d_light_set_count(0);

    rspq_block_run(rampage_assets_get()->ground->userBlock);

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_count(sizeof(pointLightPositions) / sizeof(*pointLightPositions));

    for (int i = 0; i < sizeof(pointLightPositions) / sizeof(*pointLightPositions); i += 1) {
        t3d_light_set_point(i, pointLightColors[i], &pointLightPositions[i], pointLightDistance[i], false);
    }

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_render(&gRampage.players[i]);
    }

    for (int i = 0; i < BUILDING_HEIGHT_STEPS; i += 1) {
        rspq_block_run(rampage_assets_get()->buildingSplit[i].material);
        for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
            for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
                rampage_building_render(&gRampage.buildings[y][x], i);
            }
        }
    }

    rspq_block_run(rampage_assets_get()->tankSplit.material);
    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_render(&gRampage.tanks[i]);
    }
    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_render_bullets(&gRampage.tanks[i]);
    }

    props_render(&gRampage.props);

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rdpq_sync_pipe();
        rdpq_sync_tile();

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

    rdpq_sync_pipe();
    rdpq_sync_tile(); 

    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,TEX0), (0,0,0,TEX0)));
    
    if (gRampage.state == RAMPAGE_STATE_START) {
        int countdown_number = (int)ceilf(gRampage.delay_timer);
        if (countdown_number <= 3) {
            rdpq_sprite_blit(
                rampage_assets_get()->countdown_numbers[countdown_number],
                SCREEN_WIDTH / 2 - 9,
                SCREEN_HEIGHT / 2 - 16,
                NULL
            );
        }
    } else if (gRampage.state == RAMPAGE_STATE_PLAYING && gRampage.delay_timer > 0.0f) {
        rdpq_sprite_blit(
            rampage_assets_get()->destroy_image,
            (SCREEN_WIDTH - 137) / 2,
            SCREEN_HEIGHT / 2 - 16,
            NULL
        );
    } else if (gRampage.state == RAMPAGE_STATE_FINISHED) {
        rdpq_sprite_blit(
            rampage_assets_get()->finish_image,
            (SCREEN_WIDTH - 118) / 2,
            SCREEN_HEIGHT / 2 - 16,
            NULL
        );
    } else if (gRampage.state == RAMPAGE_STATE_END_SCREEN) {
        switch (gRampage.winner_count) {
            case 1:
                rdpq_text_printf(
                    &(rdpq_textparms_t){
                        .width = SCREEN_WIDTH, 
                        .align = ALIGN_CENTER,
                        .style_id = get_winner_index(0),
                    }, FONT_TEXT, 
                    0.0f, SCREEN_HEIGHT / 2, 
                    "Player %d wins!",
                    get_winner_index(0)
                );
                break;
            case 2:
                rdpq_text_printf(
                    &(rdpq_textparms_t){
                        .width = SCREEN_WIDTH, 
                        .align = ALIGN_CENTER,
                        .style_id = 0,
                    }, FONT_TEXT, 
                    0.0f, SCREEN_HEIGHT / 2, 
                    "Players %d and %d wins!",
                    get_winner_index(0),
                    get_winner_index(1)
                );
                break;
            case 3:
                rdpq_text_printf(
                    &(rdpq_textparms_t){
                        .width = SCREEN_WIDTH, 
                        .align = ALIGN_CENTER,
                        .style_id = 0,
                    }, FONT_TEXT, 
                    0.0f, SCREEN_HEIGHT / 2, 
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
                        .width = SCREEN_WIDTH, 
                        .align = ALIGN_CENTER,
                        .style_id = 0,
                    }, FONT_TEXT, 
                    0.0f, SCREEN_HEIGHT / 2, 
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
            rampage_building_init(&rampage->buildings[y][x], &position, randomInRange(0, 4));
        }
    }

    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_init(&gRampage.tanks[i], &gStartingTankPositions[i]);
    }

    rampage->state = RAMPAGE_STATE_START;
    rampage->delay_timer = START_DELAY;

    props_init(&rampage->props, "rom:/rampage/ground.layout");
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
    props_destroy(&rampage->props);
}