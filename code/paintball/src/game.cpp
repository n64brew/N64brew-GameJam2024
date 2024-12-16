#include "game.hpp"

Game::Game() :
    viewport(t3d_viewport_create()),
    font("rom:/paintball/FingerPaint-Regular.font64", SmallFont),
    timer({nullptr, delete_timer}),
    mapRenderer(std::make_shared<MapRenderer>()),
    uiRenderer(std::make_shared<UIRenderer>()),
    gameplayController(mapRenderer, uiRenderer),
    state({
        .state = STATE_INTRO,
        .timeInState = 0.0f,
        .currentRound = 0,
        .scores = {0},
    }),
    sfxStart("rom:/core/Start.wav64"),
    sfxFinish("rom:/core/Winner.wav64"),
    sfxLastOne("rom:/core/Stop.wav64")
{
    rdpq_fontstyle_t p1Style = { .color = PLAYERCOLOR_1 };
    rdpq_fontstyle_t p2Style = { .color = PLAYERCOLOR_2 };
    rdpq_fontstyle_t p3Style = { .color = PLAYERCOLOR_3 };
    rdpq_fontstyle_t p4Style = { .color = PLAYERCOLOR_4 };
    rdpq_fontstyle_t style = { .color = RGBA32(255, 255, 180, 255) };

    auto fnt = font.font.get();
    assertf(fnt, "Font is null");

    // TODO: move to player.cpp?
    rdpq_font_style(fnt, 0, &p1Style);
    rdpq_font_style(fnt, 1, &p2Style);
    rdpq_font_style(fnt, 2, &p3Style);
    rdpq_font_style(fnt, 3, &p4Style);
    rdpq_font_style(fnt, 4, &style);

    camTarget = T3DVec3 {{0, 0, 40}};
    camPos = T3DVec3 {{0, 125.0f, 100.0f}};

    debugf("Paintball minigame initialized\n");
}

Game::~Game() {
    debugf("Paintball minigame cleaned up\n");
}

void Game::render(float deltaTime) {
    if (state.state == STATE_PAUSED || state.state == STATE_INTRO) {
        deltaTime = 0.0f;
    }
    assertf(mapRenderer.get(), "Map renderer is null");

    mixer_ch_set_vol(GeneralPurposeAudioChannel, 0.5f, 0.5f);
    
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0x55, 0x55, 0x55, 0xFF};
    uint8_t colorPoint[4]   = {0xFF, 0xFF, 0xFF, 0xFF};

    // Update camera
    T3DVec3 tmpPos = {0};
    t3d_vec3_add(tmpPos, state.avPos, T3DVec3{0, 0, 40});
    t3d_vec3_lerp(camTarget, camTarget, tmpPos, 0.12);

    t3d_vec3_add(tmpPos, state.avPos, T3DVec3{0, 125.0f, 100.0f});
    t3d_vec3_lerp(camPos, camPos, tmpPos, 0.1);

    T3DVec3 up = (T3DVec3){{0,1,0}};
    T3DVec3 lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 200.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &up);

    rdpq_attach(display_get(), display.depthBuffer);
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    rdpq_set_scissor(0, 0, ScreenWidth, ScreenHeight);
    // t3d_screen_clear_color(RGBA32(0xAA, 0xAA, 0xAA, 255));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_point(1, colorPoint, T3DVec3 {0, 50.f + (1.f - (state.gameTime / MapShrinkTime)) * 100.f,0}, 0.5f, false );
    t3d_light_set_count(2);

    // 3D
    mapRenderer->render(deltaTime, viewport.viewFrustum);

    if (state.state != STATE_PAUSED && state.state != STATE_INTRO) {
        gameplayController.render(deltaTime, viewport, state);

        // 2D
        gameplayController.renderUI();
    }
    uiRenderer->render(state, viewport, deltaTime);

    rdpq_detach_show();

    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);

    // debugf("msPF: %.2f, heap Mem: %d B\n", 1000/display_get_fps(), heap_stats.used);
}

void Game::fixedUpdate(float deltaTime) {
    if (state.state == STATE_PAUSED || state.state == STATE_INTRO) {
        return;
    }

    state.timeInState += deltaTime;

    if (state.state == STATE_GAME) {
        state.gameTime += deltaTime;
        if (state.gameTime > MapShrinkTime) {
            state.gameTime = MapShrinkTime;
        }
        mapRenderer->setSize(1.f - (state.gameTime / MapShrinkTime));
    }

    gameplayController.fixedUpdate(deltaTime, state);

    processState();
}

void Game::addScores(const std::vector<Player> &playerData) {
    for (int i = 0; i < PlayerCount; i++) {
        for (int j = 0; j < PlayerCount; j++) {
            state.scores[i] += (playerData[j].capturer == i) ? 1 : 0;
        }
    }
}

void Game::processState() {
    if (state.state == STATE_FINISHED) {
        if (timer.get() == nullptr) {
            timer = {
                new_timer_context(TICKS_FROM_MS(5000), TF_ONE_SHOT, [](int ovfl, void* self) -> void { ((Game*)self)->gameOver(); }, this),
                delete_timer
            };
        }
        return;
    }

    if (state.currentRound == RoundCount) {
        int maxScore = 0;
        PlyNum winner = PLAYER_1;
        // Calculate max score fol all teams
        for (int i = 0; i < MAXPLAYERS; i++) {
            if (state.scores[i] > maxScore) {
                maxScore = state.scores[i];
                winner = (PlyNum)i;
            }
        }

        state.winner = winner;
        core_set_winner(winner);

        state.state = STATE_FINISHED;
        wav64_play(sfxFinish.get(), GeneralPurposeAudioChannel);
        return;
    }

    if (state.state == STATE_COUNTDOWN && state.timeInState > 3.0f) {
        state.timeInState = 0.0f;
        state.gameTime = 0.0f;
        state.state = STATE_GAME;
        wav64_play(sfxStart.get(), GeneralPurposeAudioChannel);
        return;
    }

    if (state.state == STATE_WAIT_FOR_NEW_ROUND && state.timeInState > 5.0f) {
        gameplayController.newRound();
        mapRenderer->setSize(1.f);
        state.timeInState = 0.0f;
        state.state = STATE_COUNTDOWN;
        return;
    }

    auto&& playerData = gameplayController.getPlayerData();
    std::array<int, PlayerCount> counts = {0};

    int largestTeamCount = 0;
    PlyNum largestTeam = PLAYER_1;
    for (auto it = playerData.begin(); it != playerData.end(); ++it) {
        counts[it->team]++;
        if (counts[it->team] >= largestTeamCount) {
            largestTeamCount = counts[it->team];
            largestTeam = it->team;
            state.winner = largestTeam;
        }
    }

    if (
        (state.state == STATE_GAME || state.state == STATE_LAST_ONE_STANDING) &&
        largestTeamCount == PlayerCount
    ) {
        state.currentRound++;

        // One point to the team owner
        state.scores[largestTeam]++;
        state.winner = largestTeam;

        addScores(playerData);

        state.timeInState = 0.0f;
        state.state = STATE_WAIT_FOR_NEW_ROUND;
        return;
    }

    if (state.state == STATE_GAME && largestTeamCount == (PlayerCount - 1)) {
        state.timeInState = 0.0f;
        state.state = STATE_LAST_ONE_STANDING;
        wav64_play(sfxLastOne.get(), GeneralPurposeAudioChannel);
        return;
    }

    if (state.state == STATE_LAST_ONE_STANDING && largestTeamCount < 3) {
        state.timeInState = 0.6f;
        state.state = STATE_GAME;
        return;
    }

    if (state.state == STATE_LAST_ONE_STANDING && state.timeInState > LastOneStandingTime) {
        int i = 0;
        PlyNum lastPlayerTeam = PLAYER_1;
        for (auto it = playerData.begin(); it != playerData.end(); ++it) {
            if (it->team != largestTeam) {
                lastPlayerTeam = (PlyNum)i;
            }
            i++;
        }

        addScores(playerData);

        state.currentRound++;
        // Two points if can escape
        state.scores[lastPlayerTeam] += 2;
        state.winner = lastPlayerTeam;

        state.timeInState = 0.0f;
        state.state = STATE_WAIT_FOR_NEW_ROUND;
        return;
    }
}

void Game::gameOver() {
    minigame_end();
}

