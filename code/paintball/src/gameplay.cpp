#include "./gameplay.hpp"

GameplayController::GameplayController() :
    timer({ nullptr, delete_timer }),
    model({
        t3d_model_load("rom:/paintball/Bat.t3dm"),
        t3d_model_free
    }),
    gameFinished(false)
    {
        assertf(model.get(), "Player model is null");

        playerOtherData.reserve(PlayerCount);

        playerOtherData.emplace_back(PlayerOtherData {model.get()});
        playerOtherData.emplace_back(PlayerOtherData {model.get()});
        playerOtherData.emplace_back(PlayerOtherData {model.get()});
        playerOtherData.emplace_back(PlayerOtherData {model.get()});

        playerGameplayData.reserve(PlayerCount);
        playerGameplayData.emplace_back(PlayerGameplayData {{-100,0.15f,0}, PLAYER_1, {MaxHealth, 0, 0, 0}});
        playerGameplayData.emplace_back(PlayerGameplayData {{0,0.15f,-100}, PLAYER_2, {0, MaxHealth, 0, 0}});
        playerGameplayData.emplace_back(PlayerGameplayData {{100,0.15f,0}, PLAYER_3, {0, 0, MaxHealth, 0}});
        playerGameplayData.emplace_back(PlayerGameplayData {{0,0.15f,100}, PLAYER_4, {0, 0, 0, MaxHealth}});
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

        if (strength < 10.0f) {
            T3DVec3 force = otherData.velocity;
            t3d_vec3_norm(force);
            t3d_vec3_scale(force, force, -ForceLimit);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, invMass);
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

        } else  {
            otherData.direction = t3d_lerp_angle(otherData.direction, -atan2f(direction.v[0], direction.v[2]), 0.5f);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, invMass);
            t3d_vec3_add(newAccel, otherData.accel, newAccel);

            T3DVec3 velocityTarget = {0};
            t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
            t3d_vec3_add(velocityTarget, otherData.velocity, velocityTarget);

            if (t3d_vec3_len(velocityTarget) > SpeedLimit) {
                T3DVec3 accelDiff = {0};
                t3d_vec3_diff(accelDiff, velocityTarget, otherData.velocity);
                t3d_vec3_scale(accelDiff, accelDiff, 1.0f/deltaTime);
                t3d_vec3_add(newAccel, otherData.accel, accelDiff);

                t3d_vec3_norm(velocityTarget);
                t3d_vec3_scale(velocityTarget, velocityTarget, SpeedLimit);
            }
            otherData.accel = newAccel;
            otherData.velocity = velocityTarget;
        }

        T3DVec3 posDiff = {0};
        t3d_vec3_scale(posDiff, otherData.velocity, deltaTime);

        t3d_vec3_add(gameplay.pos, gameplay.pos, posDiff);

        otherData.accel = {0};
    }
}

void GameplayController::handleActions(PlayerGameplayData &player, uint32_t id) {
    if (id < core_get_playercount()) {
        joypad_buttons_t pressed = joypad_get_buttons_pressed(core_get_playercontroller((PlyNum)id));
        // Fire button
        if (pressed.c_up || pressed.d_up) {
            bulletController.fireBullet(player.pos, (T3DVec3){0, 0, -BulletVelocity}, player.team);
        } else if (pressed.c_down || pressed.d_down) {
            bulletController.fireBullet(player.pos, (T3DVec3){0, 0, BulletVelocity}, player.team);
        } else if (pressed.c_left || pressed.d_left) {
            bulletController.fireBullet(player.pos, (T3DVec3){-BulletVelocity, 0, 0}, player.team);
        } else if (pressed.c_right || pressed.d_right) {
            bulletController.fireBullet(player.pos, (T3DVec3){BulletVelocity, 0, 0}, player.team);
        }

        if (pressed.start) minigame_end();
    }
}

void GameplayController::renderPlayer(PlayerGameplayData &playerGameplay, PlayerOtherData &playerOther, uint32_t id, T3DViewport &viewport)
{
    double interpolate = core_get_subtick();
    T3DVec3 currentPos {0};
    t3d_vec3_lerp(currentPos, playerGameplay.prevPos, playerGameplay.pos, interpolate);

    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    assertf(playerOther.matFP.get(), "Player %lu matrix is null", id);
    assertf(playerOther.block.get(), "Player %lu block is null", id);

    t3d_mat4fp_from_srt_euler(
        playerOther.matFP.get(),
        (float[3]){0.2f, 0.2f, 0.2f},
        (float[3]){0.0f, playerOther.direction, 0},
        currentPos.v
    );

    rdpq_set_prim_color(colors[playerGameplay.team]);

    rspq_block_run(playerOther.block.get());

    T3DVec3 billboardPos = (T3DVec3){{
        currentPos.v[0],
        currentPos.v[1] + 15,
        currentPos.v[2]
    }};

    T3DVec3 billboardScreenPos;
    t3d_viewport_calc_viewspace_pos(viewport, billboardScreenPos, billboardPos);

    int x = floorf(billboardScreenPos.v[0]);
    int y = floorf(billboardScreenPos.v[1]);

    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise

    rdpq_textparms_t fontParams { .style_id = playerGameplay.team };
    rdpq_text_printf(
        &fontParams,
        MainFont,
        x-5,
        y-16,
        "P%lu (%u, %u, %u, %u)",
        id + 1,
        playerGameplay.health[0],
        playerGameplay.health[1],
        playerGameplay.health[2],
        playerGameplay.health[3]
    );
}

void GameplayController::render(float deltaTime, T3DViewport &viewport)
{
    int i = 0;
    for (auto& playerOther : playerOtherData)
    {
        auto& playerGameplay = playerGameplayData[i];

        handleActions(playerGameplay, i);
        renderPlayer(playerGameplay, playerOther, i, viewport);

        i++;
    }
    bulletController.render(deltaTime);
}

void GameplayController::fixedUpdate(float deltaTime)
{
    uint32_t i = 0;
    for (auto& player : playerOtherData)
    {
        auto& gameplayData = playerGameplayData[i];
        simulatePhysics(gameplayData, player, i, deltaTime);
        i++;
    }

    std::array<bool, PlayerCount>playerHitStatus = bulletController.fixedUpdate(deltaTime, playerGameplayData);

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
    if (isFinished && !gameFinished) {
        i = 0;
        for (auto& died : playerHitStatus) {
            if (i < core_get_playercount() && !died) {
                core_set_winner((PlyNum)i);
            }
            i++;
        }
        gameFinished = true;
        timer = {
            new_timer_context(TICKS_FROM_MS(3000), TF_ONE_SHOT, [](int ovfl, void* self) -> void { ((GameplayController*)self)->gameOver(); }, this),
            delete_timer
        };
    }
}

void GameplayController::gameOver() {
    minigame_end();
}