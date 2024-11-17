#include "./gameplay.hpp"

GameplayController::GameplayController(std::shared_ptr<MapRenderer> map) :
    bulletController(map),
    model({
        t3d_model_load("rom:/paintball/char.t3dm"),
        t3d_model_free
    }),
    shadowModel({
        t3d_model_load("rom:/paintball/shadow.t3dm"),
        t3d_model_free
    })
    {
        assertf(model.get(), "Player model is null");

        playerOtherData.reserve(PlayerCount);

        playerOtherData.emplace_back(PlayerOtherData {model.get(), shadowModel.get()});
        playerOtherData.emplace_back(PlayerOtherData {model.get(), shadowModel.get()});
        playerOtherData.emplace_back(PlayerOtherData {model.get(), shadowModel.get()});
        playerOtherData.emplace_back(PlayerOtherData {model.get(), shadowModel.get()});

        playerGameplayData.reserve(PlayerCount);
        playerGameplayData.emplace_back(PlayerGameplayData {{-100,0,0}, PLAYER_1, {MaxHealth, 0, 0, 0}});
        playerGameplayData.emplace_back(PlayerGameplayData {{0,0,-100}, PLAYER_2, {0, MaxHealth, 0, 0}});
        playerGameplayData.emplace_back(PlayerGameplayData {{100,0,0}, PLAYER_3, {0, 0, MaxHealth, 0}});
        playerGameplayData.emplace_back(PlayerGameplayData {{0,0,100}, PLAYER_4, {0, 0, 0, MaxHealth}});
    }

void GameplayController::simulatePhysics(PlayerGameplayData &gameplay, PlayerOtherData &otherData, uint32_t id, float deltaTime)
{
    gameplay.prevPos = gameplay.pos;

    if (id < MAXPLAYERS && id < core_get_playercount()) {
        joypad_inputs_t joypad = joypad_get_inputs(core_get_playercontroller((PlyNum)id));
        T3DVec3 direction = {0};
        direction.v[0] = (float)joypad.stick_x;
        direction.v[2] = -(float)joypad.stick_y;

        float strength = sqrtf(t3d_vec3_len2(direction));
        if (strength > ForceLimit) {
            strength = ForceLimit;
        }

        t3d_vec3_norm(direction);

        T3DVec3 force = {0};
        t3d_vec3_scale(force, direction, strength);

        // TODO: move into static functions?
        if (strength < 10.0f) {
            // Physics
            T3DVec3 force = otherData.velocity;
            t3d_vec3_norm(force);
            t3d_vec3_scale(force, force, -ForceLimit);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, PlayerInvMass);
            t3d_vec3_add(newAccel, otherData.accel, newAccel);

            T3DVec3 velocityTarget = {0};
            t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
            t3d_vec3_add(velocityTarget, otherData.velocity, velocityTarget);

            if (t3d_vec3_dot(velocityTarget, otherData.velocity) < 0) {
                otherData.velocity = {0};
                otherData.accel = {0};
            } else {
                otherData.accel = newAccel;
                otherData.velocity = velocityTarget;
            }

            // Animation
            t3d_anim_set_playing(otherData.animWalk.get(), false);
        } else  {
            // Physics
            otherData.direction = t3d_lerp_angle(otherData.direction, -atan2f(direction.v[0], direction.v[2]), 0.5f);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, PlayerInvMass);
            t3d_vec3_add(newAccel, otherData.accel, newAccel);

            T3DVec3 velocityTarget = {0};
            t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
            t3d_vec3_add(velocityTarget, otherData.velocity, velocityTarget);

            float speedLimit = strength * SpeedLimit / ForceLimit;
            if (t3d_vec3_len(velocityTarget) > speedLimit) {
                T3DVec3 accelDiff = {0};
                t3d_vec3_diff(accelDiff, velocityTarget, otherData.velocity);
                t3d_vec3_scale(accelDiff, accelDiff, 1.0f/deltaTime);
                t3d_vec3_add(newAccel, otherData.accel, accelDiff);

                t3d_vec3_norm(velocityTarget);
                t3d_vec3_scale(velocityTarget, velocityTarget, speedLimit);
            }
            otherData.accel = newAccel;
            otherData.velocity = velocityTarget;

            // Animation
            t3d_anim_set_playing(otherData.animWalk.get(), true);
            t3d_anim_set_speed(otherData.animWalk.get(), 2.f * t3d_vec3_len(velocityTarget) / SpeedLimit);
        }

        T3DVec3 posDiff = {0};
        t3d_vec3_scale(posDiff, otherData.velocity, deltaTime);

        t3d_vec3_add(gameplay.pos, gameplay.pos, posDiff);

        otherData.accel = {0};
    } else {
        // AI
        t3d_anim_set_playing(otherData.animWalk.get(), false);
    }
}

void GameplayController::handleActions(PlayerGameplayData &player, uint32_t id, GameState &state) {
    if (id < core_get_playercount()) {
        joypad_buttons_t pressed = joypad_get_buttons_pressed(core_get_playercontroller((PlyNum)id));

        if (pressed.start) minigame_end();

        if (state.isCountdown) return;

        auto position = T3DVec3{player.pos.v[0], BulletHeight, player.pos.v[2]};

        // Fire button
        if (pressed.c_up || pressed.d_up) {
            t3d_vec3_add(position, position, (T3DVec3){0, 0, -BulletOffset});
            bulletController.fireBullet(position, (T3DVec3){0, 0, -BulletVelocity}, (PlyNum)id, player.team);
            // Max firing rate is 1 bullet per 2 frames & can't fire more than one bullet
            return;
        } else if (pressed.c_down || pressed.d_down) {
            t3d_vec3_add(position, position, (T3DVec3){0, 0, BulletOffset});
            bulletController.fireBullet(position, (T3DVec3){0, 0, BulletVelocity}, (PlyNum)id, player.team);
            return;
        } else if (pressed.c_left || pressed.d_left) {
            t3d_vec3_add(position, position, (T3DVec3){-BulletOffset, 0, 0});
            bulletController.fireBullet(position, (T3DVec3){-BulletVelocity, 0, 0}, (PlyNum)id, player.team);
            return;
        } else if (pressed.c_right || pressed.d_right) {
            t3d_vec3_add(position, position, (T3DVec3){BulletOffset, 0, 0});
            bulletController.fireBullet(position, (T3DVec3){BulletVelocity, 0, 0}, (PlyNum)id, player.team);
            return;
        }
    }
}

void GameplayController::renderPlayer(PlayerGameplayData &playerGameplay, PlayerOtherData &playerOther, uint32_t id, T3DViewport &viewport, float deltaTime)
{
    double interpolate = core_get_subtick();
    T3DVec3 currentPos {0};
    t3d_vec3_lerp(currentPos, playerGameplay.prevPos, playerGameplay.pos, interpolate);
    t3d_vec3_add(currentPos, currentPos, (T3DVec3){0, 0, 0});

    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    assertf(playerOther.matFP.get(), "Player %lu matrix is null", id);
    assertf(playerOther.block.get(), "Player %lu block is null", id);

    t3d_anim_update(playerOther.animWalk.get(), deltaTime);
    t3d_skeleton_update(playerOther.skel.get());

    t3d_mat4fp_from_srt_euler(
        playerOther.matFP.get(),
        (float[3]){0.12f, 0.12f, 0.12f},
        (float[3]){0.0f, playerOther.direction, 0},
        currentPos.v
    );

    rdpq_set_prim_color(colors[playerGameplay.team]);
    rspq_block_run(playerOther.block.get());

    T3DVec3 billboardPos = (T3DVec3){{
        currentPos.v[0],
        currentPos.v[1] + 40,
        currentPos.v[2]
    }};

    t3d_viewport_calc_viewspace_pos(viewport,  playerOther.screenPos, billboardPos);
}

void GameplayController::renderPlayerUI(PlayerGameplayData &playerGameplay, PlayerOtherData &playerOther, uint32_t id)
{
    constexpr int margin = ScreenWidth / 16;
    constexpr int textHalfWidth = 10;
    constexpr int textHalfHeight = 6;
    int x = floorf(playerOther.screenPos.v[0]) - textHalfWidth;
    int y = floorf(playerOther.screenPos.v[1]) - textHalfHeight;

    if (x < margin) x = margin;
    if (x > (ScreenWidth-margin-2*textHalfWidth) ) x = (ScreenWidth-margin-2*textHalfWidth);

    if (y < (margin + textHalfHeight)) y = (margin + textHalfHeight);
    if (y > (ScreenHeight-margin-2*textHalfHeight) ) y = (ScreenHeight-margin-2*textHalfHeight);

    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise

    rdpq_textparms_t fontParams {
        .style_id = playerGameplay.team,
        .width = 20,
        .align = ALIGN_CENTER, .disable_aa_fix = false };
    rdpq_text_printf(
        &fontParams,
        MainFont,
        x,
        y,
        "P%lu", //  (%u, %u, %u, %u)
        id + 1//,
        // playerGameplay.health[0],
        // playerGameplay.health[1],
        // playerGameplay.health[2],
        // playerGameplay.health[3]
    );
}

void GameplayController::render(float deltaTime, T3DViewport &viewport, GameState &state)
{
    state.avPos = {0};
    int i = 0;
    for (auto& playerOther : playerOtherData)
    {
        auto& playerGameplay = playerGameplayData[i];

        handleActions(playerGameplay, i, state);
        renderPlayer(playerGameplay, playerOther, i, viewport, deltaTime);

        t3d_vec3_add(state.avPos, state.avPos, playerGameplay.pos);

        i++;
    }
    t3d_vec3_scale(state.avPos, state.avPos, 0.25);
    bulletController.render(deltaTime);
}

void GameplayController::renderUI()
{
    int i = 0;
    for (auto& playerOther : playerOtherData)
    {
        auto& playerGameplay = playerGameplayData[i];
        renderPlayerUI(playerGameplay, playerOther, i);
        i++;
    }
}

void GameplayController::fixedUpdate(float deltaTime, GameState &state)
{
    uint32_t i = 0;
    for (auto& player : playerOtherData)
    {
        auto& gameplayData = playerGameplayData[i];
        simulatePhysics(gameplayData, player, i, deltaTime);
        i++;
    }

    std::array<bool, PlayerCount>playerHitStatus = bulletController.fixedUpdate(deltaTime, playerGameplayData);

    // TODO: do this check less frequently
    checkGameFinished(state, playerHitStatus);
}

void GameplayController::checkGameFinished(GameState &state, std::array<bool, PlayerCount> &playerHitStatus)
{
    PlyNum team = playerGameplayData[0].team;
    bool isFinished = true;
    for (auto it = playerGameplayData.begin() + 1; it != playerGameplayData.end(); ++it) {
        // All players must be in the same team to finish the game
        if (it->team != team) {
            isFinished = false;
        }
        team = it->team;
    }

    // If you died last, you lose!
    if (isFinished && !state.gameFinished) {
        uint32_t i = 0;
        for (auto& died : playerHitStatus) {
            if (i < core_get_playercount() && !died) {
                // TODO: probably better to do this in Game
                core_set_winner((PlyNum)i);
            }
            i++;
        }
        state.gameFinished = true;
    }
}