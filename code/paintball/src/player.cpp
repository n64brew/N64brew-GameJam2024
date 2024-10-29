#include "./player.hpp"

PlayerController::PlayerController() :
    model({
        t3d_model_load("rom:/paintball/snake.t3dm"),
        t3d_model_free
    }) {}

void PlayerController::processInputs(Player &player, uint32_t id, float deltaTime)
{
    if (id < 4 && id < core_get_playercount()) {
        T3DVec3 direction = {0};

        joypad_inputs_t joypad = joypad_get_inputs(core_get_playercontroller((PlyNum)id));
        direction.v[0] = (float)joypad.stick_x;
        direction.v[2] = -(float)joypad.stick_y;

        float strength = sqrtf(t3d_vec3_len2(direction));
        if (strength > forceLimit) {
            strength = forceLimit;
        }

        t3d_vec3_norm(direction);

        T3DVec3 force = {0};
        t3d_vec3_scale(force, direction, strength);

        if (strength < 10.0f) {
            T3DVec3 force = player.velocity;
            t3d_vec3_norm(force);
            t3d_vec3_scale(force, force, -forceLimit);

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

            if (t3d_vec3_len(velocityTarget) > speedLimit) {
                T3DVec3 accelDiff = {0};
                t3d_vec3_diff(accelDiff, velocityTarget, player.velocity);
                t3d_vec3_scale(accelDiff, accelDiff, 1.0f/deltaTime);
                t3d_vec3_add(newAccel, player.accel, accelDiff);

                t3d_vec3_norm(velocityTarget);
                t3d_vec3_scale(velocityTarget, velocityTarget, speedLimit);
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

void PlayerController::setup()
{
    T3DVec3 start_positions[] = {
        (T3DVec3){{-100,0.15f,0}},
        (T3DVec3){{0,0.15f,-100}},
        (T3DVec3){{100,0.15f,0}},
        (T3DVec3){{0,0.15f,100}},
    };
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };
    players.reserve(PLAYER_COUNT);

    for (int i = 0; i<PLAYER_COUNT; i++)
    {
        players.push_back({
            start_positions[i],
            (T3DVec3){{0,0,0}},
            (T3DVec3){{0,0,0}},
            0,
            std::unique_ptr<rspq_block_t, decltype(&rspq_block_free)>({nullptr, rspq_block_free}),
            std::unique_ptr<T3DMat4FP, decltype(&free_uncached)>({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}),
            std::unique_ptr<T3DSkeleton, decltype(&t3d_skeleton_destroy)>({new T3DSkeleton(t3d_skeleton_create(model.get())), t3d_skeleton_destroy})
        });

        t3d_skeleton_update(players[i].skel.get());

        rspq_block_begin();
            t3d_matrix_push(players[i].matFP.get());
            rdpq_set_prim_color(colors[i]);
            t3d_model_draw_skinned(model.get(), players[i].skel.get());
            t3d_matrix_pop(1);
        players[i].block = std::unique_ptr<rspq_block_t, decltype(&rspq_block_free)>(rspq_block_end(), rspq_block_free);
    }
}


void PlayerController::update(float deltaTime)
{
    for (auto& player : players)
    {
        t3d_mat4fp_from_srt_euler(
            player.matFP.get(),
            (float[3]){0.125f, 0.125f, 0.125f},
            (float[3]){0.0f, player.direction, 0},
            player.pos.v
        );
        rspq_block_run(player.block.get());
    }
}

void PlayerController::fixed_update(float deltaTime)
{
    uint32_t i = 0;
    for (auto& player : players)
    {
        processInputs(player, i, deltaTime);
        i++;
    }
}