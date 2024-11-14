#include "game.hpp"

Game::Game() :
    viewport(t3d_viewport_create()),
    font("rom:/squarewave.font64", MainFont),
    mapRenderer(std::make_shared<MapRenderer>()),
    gameplayController(mapRenderer)
{
    rdpq_fontstyle_t p1Style = { .color = PLAYERCOLOR_1 };
    rdpq_fontstyle_t p2Style = { .color = PLAYERCOLOR_2 };
    rdpq_fontstyle_t p3Style = { .color = PLAYERCOLOR_3 };
    rdpq_fontstyle_t p4Style = { .color = PLAYERCOLOR_4 };

    auto fnt = font.font.get();
    assertf(fnt, "Font is null");

    // TODO: move to player.cpp
    rdpq_font_style(fnt, 0, &p1Style);
    rdpq_font_style(fnt, 1, &p2Style);
    rdpq_font_style(fnt, 2, &p3Style);
    rdpq_font_style(fnt, 3, &p4Style);

    debugf("Paintball minigame initialized\n");
}

Game::~Game() {
    debugf("Paintball minigame cleaned up\n");
}

void Game::render(float deltaTime) {
    assertf(mapRenderer.get(), "Map renderer is null");

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xFF, 0xFF, 0xFF};

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

    rdpq_set_scissor(0, 0, 320, 240);
    t3d_screen_clear_color(RGBA32(255, 255, 255, 255));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    mapRenderer->render();

    gameplayController.render(deltaTime, viewport);
    gameplayController.render2ndPass();

    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);

    debugf("FPS: %.2f, heap Mem: %d KiB\n", display_get_fps(), heap_stats.used/1024);

    rdpq_detach_show();
}

void Game::fixedUpdate(float deltaTime) {
    gameplayController.fixedUpdate(deltaTime);
}

