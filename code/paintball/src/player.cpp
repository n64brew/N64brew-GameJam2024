#include "./player.hpp"

Player::Player(T3DModel *model, color_t color, T3DVec3 pos) :
    pos {pos},
    accel({0}),
    velocity({0}),
    direction(0),
    color(color),
    block({nullptr, rspq_block_free}),
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}),
    skel({new T3DSkeleton(t3d_skeleton_create(model)), t3d_skeleton_destroy}) {
        t3d_skeleton_update(skel.get());

        rspq_block_begin();
            t3d_matrix_push(matFP.get());
            // TODO: this will change, this is not a good place
            rdpq_set_prim_color(color);
            t3d_model_draw_skinned(model, skel.get());
            t3d_matrix_pop(1);
        block = U::RSPQBlock(rspq_block_end(), rspq_block_free);
    }

PlayerController::PlayerController() :
    model({
        t3d_model_load("rom:/paintball/snake.t3dm"),
        t3d_model_free
    })
    {
        players.reserve(PlayerCount);
        players.emplace_back(Player {model.get(), PLAYERCOLOR_1, {-100,0.15f,0}});
        players.emplace_back(Player {model.get(), PLAYERCOLOR_2, {0,0.15f,-100}});
        players.emplace_back(Player {model.get(), PLAYERCOLOR_3, {100,0.15f,0}});
        players.emplace_back(Player {model.get(), PLAYERCOLOR_4, {0,0.15f,100}});
    }

void PlayerController::processInputs(Player &player, uint32_t id, float deltaTime)
{
    if (id < MAXPLAYERS && id < core_get_playercount()) {
        joypad_buttons_t pressed = joypad_get_buttons_pressed(core_get_playercontroller((PlyNum)id));
        // Fire button
        if (pressed.z) {
            T3DVec3 dir = {500, 0, 0};
            damageController.fireBullet(player.pos, dir, player.color);
        }

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
            T3DVec3 force = player.velocity;
            t3d_vec3_norm(force);
            t3d_vec3_scale(force, force, -ForceLimit);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, invMass);
            t3d_vec3_add(newAccel, player.accel, newAccel);

            T3DVec3 velocityTarget = {0};
            t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
            t3d_vec3_add(velocityTarget, player.velocity, velocityTarget);

            if (t3d_vec3_dot(velocityTarget, player.velocity) < 0) {
                player.velocity = {0};
                player.accel = {0};
            } else {
                player.accel = newAccel;
                player.velocity = velocityTarget;
            }

        } else  {
            player.direction = t3d_lerp_angle(player.direction, -atan2f(direction.v[0], direction.v[2]), 0.5f);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, invMass);
            t3d_vec3_add(newAccel, player.accel, newAccel);

            T3DVec3 velocityTarget = {0};
            t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
            t3d_vec3_add(velocityTarget, player.velocity, velocityTarget);

            if (t3d_vec3_len(velocityTarget) > SpeedLimit) {
                T3DVec3 accelDiff = {0};
                t3d_vec3_diff(accelDiff, velocityTarget, player.velocity);
                t3d_vec3_scale(accelDiff, accelDiff, 1.0f/deltaTime);
                t3d_vec3_add(newAccel, player.accel, accelDiff);

                t3d_vec3_norm(velocityTarget);
                t3d_vec3_scale(velocityTarget, velocityTarget, SpeedLimit);
            }
            player.accel = newAccel;
            player.velocity = velocityTarget;
        }

        T3DVec3 posDiff = {0};
        t3d_vec3_scale(posDiff, player.velocity, deltaTime);
        t3d_vec3_add(player.pos, player.pos, posDiff);

        player.accel = {0};
    }
}

void PlayerController::update(float deltaTime, T3DViewport &viewport)
{
    int16_t i = 0;
    for (auto& player : players)
    {
        t3d_mat4fp_from_srt_euler(
            player.matFP.get(),
            (float[3]){0.125f, 0.125f, 0.125f},
            (float[3]){0.0f, player.direction, 0},
            player.pos.v
        );
        auto bl = player.block.get();
        assertf(bl != nullptr, "Block is null");
        rspq_block_run(player.block.get());

        T3DVec3 billboardPos = (T3DVec3){{
            player.pos.v[0],
            player.pos.v[1] + 15,
            player.pos.v[2]
        }};

        T3DVec3 billboardScreenPos;
        t3d_viewport_calc_viewspace_pos(viewport, billboardScreenPos, billboardPos);

        int x = floorf(billboardScreenPos.v[0]);
        int y = floorf(billboardScreenPos.v[1]);

        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise

        rdpq_textparms_t fontParams { .style_id = i };
        rdpq_text_printf(&fontParams, MainFont, x-5, y-16, "P%d", i+1);
        i++;
    }
    damageController.update(deltaTime);
}

void PlayerController::fixed_update(float deltaTime)
{
    uint32_t i = 0;
    for (auto& player : players)
    {
        processInputs(player, i, deltaTime);
        i++;
    }
    damageController.fixed_update(deltaTime);
}