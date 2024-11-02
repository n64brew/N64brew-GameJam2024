#include "./player.hpp"

PlayerOtherData::PlayerOtherData(T3DModel *model, const color_t &color) :
    accel({0}),
    velocity({0}),
    direction(0),
    block({nullptr, rspq_block_free}),
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}),
    skel({new T3DSkeleton(t3d_skeleton_create(model)), t3d_skeleton_destroy}) {
        assertf(skel.get(), "Player skel is null");
        assertf(matFP.get(), "Player matrix is null");

        t3d_skeleton_update(skel.get());

        rspq_block_begin();
            t3d_matrix_push(matFP.get());
            // TODO: this will change, this is not a good place
            rdpq_set_prim_color(color);
            t3d_model_draw_skinned(model, skel.get());
            t3d_matrix_pop(1);
        block = U::RSPQBlock(rspq_block_end(), rspq_block_free);
    }

GameplayController::GameplayController() :
    model({
        t3d_model_load("rom:/paintball/snake.t3dm"),
        t3d_model_free
    })
    {
        assertf(model.get(), "Player model is null");

        playerOtherData.reserve(PlayerCount);

        playerOtherData.emplace_back(PlayerOtherData {model.get(), PLAYERCOLOR_1});
        playerOtherData.emplace_back(PlayerOtherData {model.get(), PLAYERCOLOR_2});
        playerOtherData.emplace_back(PlayerOtherData {model.get(), PLAYERCOLOR_3});
        playerOtherData.emplace_back(PlayerOtherData {model.get(), PLAYERCOLOR_4});

        playerGameplayData.reserve(PlayerCount);
        playerGameplayData.emplace_back(PlayerGameplayData {{-100,0.15f,0}, PLAYERCOLOR_1});
        playerGameplayData.emplace_back(PlayerGameplayData {{0,0.15f,-100}, PLAYERCOLOR_2});
        playerGameplayData.emplace_back(PlayerGameplayData {{100,0.15f,0}, PLAYERCOLOR_3});
        playerGameplayData.emplace_back(PlayerGameplayData {{0,0.15f,100}, PLAYERCOLOR_4});
    }

void GameplayController::simulatePhysics(PlayerGameplayData &gameplay, PlayerOtherData &otherData, uint32_t id, float deltaTime)
{
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
            damageController.fireBullet(player.pos, (T3DVec3){0, 0, -BulletVelocity}, player.color);
        } else if (pressed.c_down || pressed.d_down) {
            damageController.fireBullet(player.pos, (T3DVec3){0, 0, BulletVelocity}, player.color);
        } else if (pressed.c_left || pressed.d_left) {
            damageController.fireBullet(player.pos, (T3DVec3){-BulletVelocity, 0, 0}, player.color);
        } else if (pressed.c_right || pressed.d_right) {
            damageController.fireBullet(player.pos, (T3DVec3){BulletVelocity, 0, 0}, player.color);
        }
    }
}

void GameplayController::renderPlayer(PlayerGameplayData &playerGameplay, PlayerOtherData &playerOther, uint32_t id, T3DViewport &viewport)
{
    assertf(playerOther.matFP.get(), "Player %lu matrix is null", id);
    assertf(playerOther.block.get(), "Player %lu block is null", id);

    t3d_mat4fp_from_srt_euler(
        playerOther.matFP.get(),
        (float[3]){0.125f, 0.125f, 0.125f},
        (float[3]){0.0f, playerOther.direction, 0},
        playerGameplay.pos.v
    );
    rspq_block_run(playerOther.block.get());

    T3DVec3 billboardPos = (T3DVec3){{
        playerGameplay.pos.v[0],
        playerGameplay.pos.v[1] + 15,
        playerGameplay.pos.v[2]
    }};

    T3DVec3 billboardScreenPos;
    t3d_viewport_calc_viewspace_pos(viewport, billboardScreenPos, billboardPos);

    int x = floorf(billboardScreenPos.v[0]);
    int y = floorf(billboardScreenPos.v[1]);

    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise

    rdpq_textparms_t fontParams { .style_id = static_cast<int16_t>(id) };
    rdpq_text_printf(&fontParams, MainFont, x-5, y-16, "P%lu", id+1);
}

void GameplayController::update(float deltaTime, T3DViewport &viewport)
{
    int i = 0;
    for (auto& playerOther : playerOtherData)
    {
        auto& playerGameplay = playerGameplayData[i];

        handleActions(playerGameplay, i);
        renderPlayer(playerGameplay, playerOther, i, viewport);

        i++;
    }
    damageController.update(deltaTime);
}

void GameplayController::fixed_update(float deltaTime)
{
    uint32_t i = 0;
    for (auto& player : playerOtherData)
    {
        auto& gameplayData = playerGameplayData[i];
        
        simulatePhysics(gameplayData, player, i, deltaTime);
        i++;
    }
    damageController.fixed_update(deltaTime);
}