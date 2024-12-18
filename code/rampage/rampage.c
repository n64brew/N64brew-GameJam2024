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
#include <math.h>

#include "./rampage.h"
#include "./collision/collision_scene.h"
#include "./health.h"
#include "./assets.h"
#include "./rampage.h"
#include "./math/mathf.h"
#include "./frame_malloc.h"
#include "./spark_effect.h"

bool useHighRes = false;

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

int screenWidth;
int screenHeight;

surface_t *depthBuffer;
T3DViewport viewport;
surface_t background_surface;
bool has_background;

T3DVec3 camPos = {{SCALE_FIXED_POINT(3.0f), SCALE_FIXED_POINT(4.5f), SCALE_FIXED_POINT(4.0f)}};
T3DVec3 camTarget = {{0.0f, 0.0f, 0.0f}};

#define START_DELAY 4.5f
#define FINISH_DELAY 3.0f
#define END_SCREEN_DELAY    4.0f
#define DESTROY_TITLE_TIME  2.0f

struct Rampage gRampage;

const MinigameDef minigame_def = {
    .gamename = "Destroy!",
    .developername = "Ultrarare",
    .description = "Destroy buildings for points. Most points wins.",
    .instructions = "Press B to attack."
};

struct frame_malloc frame_mallocs[2];
int next_frame_malloc;

static float accum_time;
static float last_frame_time;

#define FIXED_DT    (1.0f / 30.0f)

#define PROJECTION_RATIO    2.0f
#define NEAR_PLANE          SCALE_FIXED_POINT(-1.0f)
#define FAR_PLANE           SCALE_FIXED_POINT(20.0f)
#define ORTHO_SCALE         2.6f

/**
 * Modify an ortho matrix to apply some perspective
 */
void minigame_add_some_perspective(T3DMat4* mat, float near, float far) {
    float scale = (PROJECTION_RATIO - 1.0f/PROJECTION_RATIO) / (near - far);
    float offset = PROJECTION_RATIO - scale * near;

    mat->m[2][3] = scale;
    mat->m[3][3] = offset;
}

void minigame_init_viewport() {
    t3d_viewport_set_ortho(
        &viewport, 
        SCALE_FIXED_POINT(-ORTHO_SCALE * 1.5f), SCALE_FIXED_POINT(ORTHO_SCALE * 1.5f),
        SCALE_FIXED_POINT(-ORTHO_SCALE), SCALE_FIXED_POINT(ORTHO_SCALE),
        NEAR_PLANE, FAR_PLANE
    );
    minigame_add_some_perspective(&viewport.matProj, NEAR_PLANE, FAR_PLANE);

    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
}

void minigame_init() {
    randomSeed((int)get_ticks_us());
    useHighRes = is_memory_expanded();
    screenWidth = 640;
    screenHeight = 480;
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);

    redraw_manager_init(screenWidth, screenHeight);
    t3d_init((T3DInitParams){});

    collision_scene_init();
    health_init();
    
    depthBuffer = display_get_zbuf();
    viewport = t3d_viewport_create();

    rampage_init(&gRampage);

    wav64_play(&rampage_assets_get()->startJingle, 2);

    background_surface = surface_alloc(FMT_RGBA16, screenWidth, screenHeight);

    last_frame_time = get_ticks_ms() * (1.0f / 1000.0f);
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

void minigame_fixedloop(float delattime) {

}

void rampage_fixedloop(float deltatime) {
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
            mixer_ch_set_vol_pan(1, 0.85f, 0.5f);
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

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        props_check_collision(&gRampage.props, &gRampage.players[i].dynamic_object);
    }

    for (int i = 0; i < TANK_COUNT; i += 1) {
        props_check_collision(&gRampage.props, &gRampage.tanks[i].dynamic_object);
    }

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_update(&gRampage.players[i], deltatime);
    }

    spark_effects_update(deltatime);

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            rampage_building_update(&gRampage.buildings[y][x], deltatime);
        }
    }

    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_update(&gRampage.tanks[i], deltatime);
    }
}

uint8_t colorWhite[4] = {0xFF, 0xFF, 0xFF, 0xFF};

uint8_t pointLightColors[][4] = {
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xE0, 0x30, 0x20, 0xFF},
    {0xE0, 0x30, 0x20, 0xFF},
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

#define NUMBER_WIDTH    34
#define NUMBER_HEIGHT   48
#define MARGIN          40

struct Vector2 scorePosition[] = {
    {MARGIN, MARGIN},
    {SCREEN_WIDTH - (MARGIN + NUMBER_WIDTH * 2), MARGIN},
    {SCREEN_WIDTH - (MARGIN + NUMBER_WIDTH * 2), SCREEN_HEIGHT - (MARGIN + NUMBER_HEIGHT)},
    {MARGIN, SCREEN_HEIGHT - (MARGIN + NUMBER_HEIGHT)},
};

int get_winner_index(int index) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        if (gRampage.winner_mask & (1 << i)) {
            if (index == 0) {
                return i;
            }

            --index;
        }
    }

    return 0;
}

uint8_t clear_shade = 128;

#define DESTROY_SIZE_X  (348 / 2 + 2)
#define DESTROY_SIZE_Y  (66 / 2 + 2)

void minigame_redraw_rects() {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_redraw_rect(&viewport, &gRampage.players[i]);

        if (gRampage.players[i].score_dirty == 0) {
            continue;
        }

        struct RedrawRect rect;
        rect.min[0] = scorePosition[i].x;
        rect.min[1] = scorePosition[i].y;
        rect.max[0] = scorePosition[i].x + 70;
        rect.max[1] = scorePosition[i].y + 48;

        redraw_update_dirty(gRampage.score_redraw[i], &rect);
        gRampage.players[i].score_dirty -= 1;
    }

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            rampage_building_redraw_rect(&viewport, &gRampage.buildings[y][x]);
        }
    }

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_tank_redraw_rect(&viewport, &gRampage.tanks[i]);
    }

    if (gRampage.state == RAMPAGE_STATE_START) {
        struct RedrawRect rect;
        rect.min[0] = (screenWidth >> 1) - 30;
        rect.min[1] = (screenHeight >> 1) - 32;
        rect.max[0] = (screenWidth >> 1) + 30;
        rect.max[1] = (screenHeight >> 1) + 32;
        redraw_update_dirty(gRampage.center_text_redraw, &rect);
    } else if (gRampage.state == RAMPAGE_STATE_PLAYING && gRampage.delay_timer > 0.0f) {
        struct RedrawRect rect;
        rect.min[0] = (screenWidth >> 1) - DESTROY_SIZE_X;
        rect.min[1] = (screenHeight >> 1) - DESTROY_SIZE_Y;
        rect.max[0] = (screenWidth >> 1) + DESTROY_SIZE_X;
        rect.max[1] = (screenHeight >> 1) + DESTROY_SIZE_Y;
        redraw_update_dirty(gRampage.center_text_redraw, &rect);
    } else if (gRampage.state == RAMPAGE_STATE_FINISHED) {
        struct RedrawRect rect;
        rect.min[0] = (screenWidth >> 1) - DESTROY_SIZE_X;
        rect.min[1] = (screenHeight >> 1) - DESTROY_SIZE_Y;
        rect.max[0] = (screenWidth >> 1) + DESTROY_SIZE_X;
        rect.max[1] = (screenHeight >> 1) + DESTROY_SIZE_Y;
        redraw_update_dirty(gRampage.center_text_redraw, &rect);
    }

    struct RedrawRect rects[MAX_REDRAW_ENTITIES];

    int rect_count = redraw_retrieve_dirty_rects(rects);
    clear_shade += 1;

    rdpq_set_prim_color((color_t){255 - clear_shade, clear_shade, 0, 255});

    for (int i = 0; i < rect_count; i += 1) {
        struct RedrawRect* rect = &rects[i];
        
        rdpq_set_scissor(rect->min[0], rect->min[1], rect->max[0], rect->max[1]);
        rspq_block_run(rampage_assets_get()->ground_cover->userBlock);
    }
    rdpq_set_scissor(0, 0, screenWidth, screenHeight);

    rdpq_sync_pipe();
    rdpq_sync_tile();
    rdpq_set_mode_copy(false);

    for (int i = 0; i < rect_count; i += 1) {
        struct RedrawRect* rect = &rects[i];

        rdpq_tex_blit(
            &background_surface, 
            rect->min[0], rect->min[1], 
            &(rdpq_blitparms_t){
                .s0 = rect->min[0],
                .t0 = rect->min[1],
                .width = rect->max[0] - rect->min[0],
                .height = rect->max[1] - rect->min[1],
            }
        );
    }

    rdpq_sync_pipe();
    rdpq_sync_tile();
    t3d_frame_start();
}

void minigame_loop(float deltatime) {
    float current_time = get_ticks_ms() * (1.0f / 1000.0f);
    deltatime = current_time - last_frame_time;

    if (deltatime > 0.25f) {
        deltatime = 0.25f;
    }

    accum_time += deltatime;

    while (accum_time > FIXED_DT) {
        rampage_fixedloop(FIXED_DT);
        accum_time -= FIXED_DT;
    }

    last_frame_time = current_time;

    surface_t* display = display_try_get();

    if (!display) {
        return;
    }

    uint8_t colorAmbient[4] = {0x30, 0x30, 0x30, 0xFF};

    struct frame_malloc* fm = &frame_mallocs[next_frame_malloc];

    frame_malloc_init(fm);
    next_frame_malloc ^= 1;

    minigame_init_viewport();

    rdpq_attach(display, depthBuffer);

    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    if (!has_background) {
        rdpq_set_color_image_raw(
            0, 
            PhysicalAddr(background_surface.buffer), 
            FMT_RGBA16, 
            background_surface.width, 
            background_surface.height, 
            background_surface.stride
        );

        rdpq_clear((color_t){0, 0, 0, 0xFF});
        t3d_screen_clear_depth();
        t3d_light_set_ambient(colorWhite);
        t3d_light_set_count(0);
        rspq_block_run(rampage_assets_get()->ground->userBlock);
        has_background = true;

        rdpq_set_color_image_raw(
            0, 
            PhysicalAddr(display->buffer), 
            FMT_RGBA16, 
            display->width, 
            display->height, 
            display->stride
        );
    }

    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorWhite);
    t3d_light_set_count(0);

    minigame_redraw_rects();

    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_render_bullets(&gRampage.tanks[i]);
    }

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_count(sizeof(pointLightPositions) / sizeof(*pointLightPositions));

    for (int i = 0; i < sizeof(pointLightPositions) / sizeof(*pointLightPositions); i += 1) {
        t3d_light_set_point(i, pointLightColors[i], &pointLightPositions[i], pointLightDistance[i], false);
    }

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_render(&gRampage.players[i]);
    }

    spark_effects_render(fm);

    for (int i = 0; i < BUILDING_HEIGHT_STEPS; i += 1) {
        rspq_block_run(rampage_assets_get()->buildingSplit[i].material);
        for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
            for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
                rampage_building_render(&gRampage.buildings[y][x], i);
            }
        }
    }

    rspq_block_run(rampage_assets_get()->billboardsSplit[0].material);
    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            rampage_building_render_billboards(&gRampage.buildings[y][x]);
        }
    }

    rspq_block_run(rampage_assets_get()->tankSplit.material);
    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_render(&gRampage.tanks[i]);
    }

    props_render(&gRampage.props);

    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,TEX0), (0,0,0,TEX0)));
    rdpq_mode_filter(FILTER_BILINEAR);
    
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        int tens = gRampage.players[i].score / 10;
        int ones = gRampage.players[i].score % 10;

        rdpq_sprite_blit(
            rampage_assets_get()->score_digits[i],
            scorePosition[i].x - 7,
            scorePosition[i].y,
            &(rdpq_blitparms_t) {
                .s0 = tens * 48,
                .width = 48,
            }
        );

        rdpq_sprite_blit(
            rampage_assets_get()->score_digits[i],
            scorePosition[i].x + 27.0f,
            scorePosition[i].y,
            &(rdpq_blitparms_t) {
                .s0 = ones * 48,
                .width = 48,
            }
        );
    }
    if (gRampage.state == RAMPAGE_STATE_START) {
        int countdown_number = (int)ceilf(gRampage.delay_timer);
        if (countdown_number <= 3) {
            rdpq_sprite_blit(
                rampage_assets_get()->countdown_numbers[countdown_number],
                (screenWidth >> 1) - 24,
                (screenHeight >> 1) - 30,
                NULL
            );
        }
    } else if (gRampage.state == RAMPAGE_STATE_PLAYING && gRampage.delay_timer > 0.0f) {
        rdpq_sprite_blit(
            rampage_assets_get()->destroy_image,
            (screenWidth - 348) / 2 + randomInRange(-2, 3),
            (screenHeight - 64) / 2 + randomInRange(-2, 3),
            &(rdpq_blitparms_t) {
                    .scale_y = useHighRes ? 1.0f : 2.0f,
                    .scale_x = useHighRes ? 1.0f : 2.0f,
            }
        );
    } else if (gRampage.state == RAMPAGE_STATE_FINISHED) {
        rdpq_sprite_blit(
            rampage_assets_get()->finish_image,
            (screenWidth - 237) / 2,
            (screenHeight - 62) / 2,
            &(rdpq_blitparms_t) {
                .scale_y = useHighRes ? 1.0f : 2.0f,
                .scale_x = useHighRes ? 1.0f : 2.0f,
            }
        );
    } else if (gRampage.state == RAMPAGE_STATE_END_SCREEN) {
        for (int i = 0; i < gRampage.winner_count; i += 1) {
            int x = (screenWidth - 288) / 2;
            int y = (screenHeight >> 1) - gRampage.winner_count * 32 +
                i * 64;

            rdpq_sprite_blit(
                rampage_assets_get()->winner_screen[get_winner_index(i)],
                x,
                y,
                &(rdpq_blitparms_t) {
                    .scale_y = useHighRes ? 1.0f : 2.0f,
                    .scale_x = useHighRes ? 1.0f : 2.0f,
                }
            );
        }

        if (gRampage.winner_count == 0) {
            rdpq_sprite_blit(
                rampage_assets_get()->tie_image,
                (screenWidth - 124) / 2,
                (screenHeight - 64) / 2,
                &(rdpq_blitparms_t) {
                }
            );
        }
    }

    rdpq_detach_show();
}

void minigame_cleanup() {
    rampage_destroy(&gRampage);
    t3d_destroy();
    collision_scene_destroy();
    health_destroy();
    display_close();
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

bool rampage_add_billboard(struct Rampage* rampage, int billboard) {
    int x = randomInRange(0, BUILDING_COUNT_X);
    int y = randomInRange(0, BUILDING_COUNT_Y);

    return rampage_building_add_billboard(&rampage->buildings[y][x], billboard);
}

int ramapge_random_shuffle(const void*, const void*) {
    return randomInRange(-10, 10);
}

void rampage_add_all_billboards(struct Rampage* rampage) {
    int billboard_count = randomInRange(7, 10);

    uint8_t billboard_deck[BILLBOARD_COUNT];

    for (int i = 0; i < BILLBOARD_COUNT; i += 1) {
        billboard_deck[i] = i;
    }

    int deck_index = BILLBOARD_COUNT;

    while (billboard_count) {
        if (deck_index == BILLBOARD_COUNT) {
            qsort(billboard_deck, BILLBOARD_COUNT, 1, ramapge_random_shuffle);
            deck_index = 0;
        }

        if (rampage_add_billboard(rampage, billboard_deck[deck_index])) {
            billboard_count -= 1;
            deck_index += 1;
        }

    }
}

static uint8_t min_building_height[BUILDING_COUNT_Y][BUILDING_COUNT_X] = {
    {1, 1, 2, 1, 1},
    {1, 2, 3, 2, 1},
    {1, 2, 3, 2, 1},
    {1, 1, 2, 1, 1},
};

static uint8_t max_building_height[BUILDING_COUNT_Y][BUILDING_COUNT_X] = {
    {1, 2, 2, 2, 1},
    {2, 2, 3, 3, 2},
    {2, 3, 3, 2, 2},
    {1, 2, 2, 2, 1},
};

void rampage_init(struct Rampage* rampage) {
    rampage_assets_init(useHighRes);

    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        rampage_player_init(&rampage->players[i], &gStartingPositions[i], &gStartingRotations[i], i, rampage_player_type(i));
        rampage->score_redraw[i] = redraw_aquire_handle();
    }

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            T3DVec3 position = {{
                (x - (BUILDING_COUNT_X - 1) * 0.5f) * BUILDING_SPACING,
                0.0f,
                (y - (BUILDING_COUNT_Y - 1) * 0.5f) * BUILDING_SPACING,
            }};
            rampage_building_init(
                &rampage->buildings[y][x], 
                &position, 
                randomInRange(0, 4), 
                randomInRange(min_building_height[y][x], max_building_height[y][x] + 1)
            );
        }
    }

    rampage_add_all_billboards(&gRampage);

    for (int i = 0; i < TANK_COUNT; i += 1) {
        rampage_tank_init(&gRampage.tanks[i], &gStartingTankPositions[i]);
    }

    rampage->state = RAMPAGE_STATE_START;
    rampage->delay_timer = START_DELAY;
    rampage->center_text_redraw = redraw_aquire_handle();

    props_init(&rampage->props, "rom:/rampage/ground.layout");
    spark_effects_init();
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
    spark_effects_destroy();
    surface_free(&background_surface);
}