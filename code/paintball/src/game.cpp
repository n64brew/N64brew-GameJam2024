#include "game.hpp"

Game::Game() :
    viewport(t3d_viewport_create()),
    font("rom:/squarewave.font64", MainFont),
    timer({
        new_timer_context(TICKS_FROM_MS(10000), TF_ONE_SHOT, [](int ovfl, void* self) -> void { ((Game*)self)->timer_callback(); }, this),
        delete_timer
    }),

    // Map
    mapMatFP({
        (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)),
        free_uncached
    }),
    dplMap({nullptr, rspq_block_free}),
    modelMap({
        t3d_model_load("rom:/paintball/map.t3dm"),
        t3d_model_free
    })
{
    rdpq_fontstyle_t p1Style = { .color = PLAYERCOLOR_1 };
    rdpq_fontstyle_t p2Style = { .color = PLAYERCOLOR_2 };
    rdpq_fontstyle_t p3Style = { .color = PLAYERCOLOR_3 };
    rdpq_fontstyle_t p4Style = { .color = PLAYERCOLOR_4 };

    auto fnt = font.font.get();
    assertf(fnt, "Font is null");

    rdpq_font_style(fnt, 0, &p1Style);
    rdpq_font_style(fnt, 1, &p2Style);
    rdpq_font_style(fnt, 2, &p3Style);
    rdpq_font_style(fnt, 3, &p4Style);

    setupMap(mapMatFP);
    debugf("Paintball minigame initialized\n");
}

void Game::timer_callback() {
    debugf("Timer callback called\n");
    minigame_end();
}

void Game::setupMap(U::T3DMat4FP &mapMatFP) {
    assertf(mapMatFP.get(), "Map matrix is null");
    assertf(modelMap.get(), "Map model is null");

    t3d_mat4fp_from_srt_euler(
        mapMatFP.get(),
        (float[3]){0.3f, 0.3f, 0.3f},
        (float[3]){0, 0, 0},
        (float[3]){0, 0, -10}
    );
    rspq_block_begin();
        t3d_matrix_push(mapMatFP.get());
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(modelMap.get());
        t3d_matrix_pop(1);
    dplMap = U::RSPQBlock(rspq_block_end(), rspq_block_free);
}

void Game::renderMap() {
    assertf(dplMap.get(), "Map dl is null");
    rspq_block_run(dplMap.get());
}

Game::~Game() {
    debugf("Paintball minigame cleaned up\n");
}

void Game::update(float deltaTime) {
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

    T3DVec3 camPos = (T3DVec3){{0, 125.0f, 100.0f}};
    T3DVec3 camTarget = (T3DVec3){{0, 0, 40}};
    T3DVec3 up = (T3DVec3){{0,1,0}};

    T3DVec3 lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 160.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &up);

    rdpq_attach(display_get(), display.depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    renderMap();

    playerController.update(deltaTime, viewport);

    rdpq_detach_show();
}

void Game::fixed_update(float deltaTime) {
    playerController.fixed_update(deltaTime);
}

Game* Game_new()
{
    return new Game();
}

void Game_update(Game* self, float delta)
{
    self->update(delta);
}

void Game_fixed_update(Game* self, float delta)
{
    self->fixed_update(delta);
}

void Game_destroy(Game* self)
{
    delete self;
}

