#include "game.hpp"

Game::Game() :
    viewport(t3d_viewport_create()),
    font("rom:/squarewave.font64", MainFont),
    timer({nullptr, delete_timer}),
    mapRenderer(std::make_shared<MapRenderer>()),
    gameplayController(mapRenderer),
    state({
        .state = STATE_COUNTDOWN,
        .gameTime = 0.0f,
        .currentRound = 0,
        .scores = {0},
    })
{
    rdpq_fontstyle_t p1Style = { .color = PLAYERCOLOR_1 };
    rdpq_fontstyle_t p2Style = { .color = PLAYERCOLOR_2 };
    rdpq_fontstyle_t p3Style = { .color = PLAYERCOLOR_3 };
    rdpq_fontstyle_t p4Style = { .color = PLAYERCOLOR_4 };
    rdpq_fontstyle_t style = { .color = RGBA32(255, 255, 0, 255) };

    auto fnt = font.font.get();
    assertf(fnt, "Font is null");

    // TODO: move to player.cpp
    rdpq_font_style(fnt, 0, &p1Style);
    rdpq_font_style(fnt, 1, &p2Style);
    rdpq_font_style(fnt, 2, &p3Style);
    rdpq_font_style(fnt, 3, &p4Style);
    rdpq_font_style(fnt, 4, &style);

    camTarget = (T3DVec3){{0, 0, 40}};
    camPos = (T3DVec3){{0, 125.0f, 100.0f}};

    debugf("Paintball minigame initialized\n");
}

Game::~Game() {
    debugf("Paintball minigame cleaned up\n");
}

void Game::render(float deltaTime) {
    assertf(mapRenderer.get(), "Map renderer is null");

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xFF, 0xFF, 0xFF};

    // Update camera
    t3d_vec3_add(camTarget, state.avPos, T3DVec3{0, 0, 40});
    t3d_vec3_add(camPos, state.avPos, T3DVec3{0, 125.0f, 100.0f});

    T3DVec3 up = (T3DVec3){{0,1,0}};
    T3DVec3 lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 200.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &up);

    rdpq_attach(display_get(), display.depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    rdpq_set_scissor(0, 0, ScreenWidth, ScreenHeight);
    t3d_screen_clear_color(RGBA32(255, 255, 255, 255));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    // 3D
    mapRenderer->render(viewport);
    gameplayController.render(deltaTime, viewport, state);

    // 2D
    gameplayController.renderUI();
    uiRenderer.render(state);
    rdpq_detach_show();

    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);

    debugf("FPS: %.2f, heap Mem: %d KiB\n", display_get_fps(), heap_stats.used/1024);
}

void Game::fixedUpdate(float deltaTime) {
    state.gameTime += deltaTime;
    gameplayController.fixedUpdate(deltaTime, state);

    processState();

    // if (timer.get() == nullptr) {
    //     timer = {
    //         new_timer_context(TICKS_FROM_MS(3000), TF_ONE_SHOT, [](int ovfl, void* self) -> void { ((Game*)self)->gameOver(); }, this),
    //         delete_timer
    //     };
    // }
}

void Game::processState() {
    if (state.state == STATE_FINISHED) {
        state.state = STATE_FINISHED;
        return;
    }

    if (state.currentRound == RoundCount) {
        state.state = STATE_FINISHED;
        return;
    }

    if (state.state == STATE_COUNTDOWN && state.gameTime > 3.0f) {
        state.state = STATE_GAME;
        return;
    }

    if (state.state == STATE_WAIT_FOR_NEW_ROUND && state.gameTime > 3.0f) {
        gameplayController.newRound();
        state.state = STATE_COUNTDOWN;
        return;
    }

    auto&& playerGameplayData = gameplayController.getPlayerGameplayData();
    std::array<int, MAXPLAYERS> counts = {0};

    int largestTeamCount = 0;
    PlyNum largestTeam = PLAYER_1;
    for (auto it = playerGameplayData.begin() + 1; it != playerGameplayData.end(); ++it) {
        counts[it->team]++;
        if (counts[it->team] > largestTeamCount) {
            largestTeamCount = counts[it->team];
            largestTeam = it->team;
        }
    }

    if (largestTeamCount == 4) {
        state.currentRound++;
        state.scores[largestTeam]++;

        int i = 0;
        for (auto& player : playerGameplayData) {
            state.scores[i] += player.fragCount;
            i++;
        }

        state.gameTime = 0.0f;
        state.state = STATE_WAIT_FOR_NEW_ROUND;
        return;
    }

    if (largestTeamCount == 3) {
        state.gameTime = 0.0f;
        state.state = STATE_LAST_ONE_STANDING;
        return;
    }

    if (state.state == STATE_LAST_ONE_STANDING && state.gameTime > LastOneStandingTime) {
        int i = 0;
        PlyNum lastPlayerTeam = PLAYER_1;
        for (auto it = playerGameplayData.begin() + 1; it != playerGameplayData.end(); ++it) {
            if (it->team != largestTeam) {
                lastPlayerTeam = (PlyNum)i;
            }
            i++;
        }
        state.scores[lastPlayerTeam]++;
        state.gameTime = 0.0f;
        state.state = STATE_WAIT_FOR_NEW_ROUND;
        return;
    }
}

void Game::gameOver() {
    minigame_end();
}

