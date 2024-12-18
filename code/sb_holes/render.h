#ifndef RENDER_H
#define RENDER_H

void render_scene(game_data *game, scene_data *scene);

void render_scene(game_data *game, scene_data *scene)
{
    // Attach display and color clear
    rdpq_attach(display_get(), depthBuffer);
    rdpq_clear(RGBA32(0, 0, 0, 255));

    // For each human player
    for (size_t i = 0; i < game->playerCount; i++)
    {

        // If not alive, do nothing, only for 4 player though
        if (!players[i].isAlive && game->playerCount == 4)
            continue;

        // Reset render mode and attach player's viewport
        t3d_frame_start();
        t3d_viewport_attach(&viewport[i]);

        // Set lights
        t3d_light_set_ambient(scene->colorAmbient);
        t3d_light_set_directional(0, scene->colorDir, &scene->lightDirVec);
        t3d_light_set_count(1);

        /** Render Layout
         * - Map
         * - Players
         * - Objects
         * - UI
         */

        t3d_matrix_push_pos(1);

        // Disable Z buffer compare for first layer
        rdpq_mode_zbuf(false, true);
        rspq_block_run(map.block);

        // reset Z buffer testing for second layer
        rdpq_sync_tile();
        rdpq_sync_pipe();
        rdpq_mode_zbuf(false, false);
        for (size_t p = 0; p < MAXPLAYERS; p++)
        {
            player_draw(&players[p]);
        }

        // Disable Z buffer write for last layer
        rdpq_sync_tile();
        rdpq_sync_pipe();
        rdpq_mode_zbuf(true, false);
        for (int j = 0; j < NUM_OBJ_TYPES; j++)
        {
            object_cull(&objects[j], &viewport[i], game->playerCount);
        }

        object_drawBatch(&objects[OBJ_HYDRANT]);
        for (size_t o = 0; o < NUM_OBJECTS; o++)
        {
            if (spray[o] && !objects[OBJ_HYDRANT].objects[o].hide)
                hydrant_water_spray(objects[OBJ_HYDRANT].objects[o].position, &viewport[i]);
        }

        // Reset render mode for 3D
        t3d_frame_start();

        object_drawBatch(&objects[OBJ_BUILDING]);
        object_drawBatch(&objects[OBJ_CAR]);

        t3d_matrix_pop(1);
    }

    game->syncPoint = rspq_syncpoint_new();

    // ======== Draw (2D) ======== //
    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise

    viewport_drawScissor();

    if (game->scene == INTRO)
    {

        players[0].btn.pressed = joypad_get_buttons_pressed(PLAYER_1);
        players[0].btn.held = joypad_get_buttons_held(PLAYER_1);
        players[0].btn.released = joypad_get_buttons_released(PLAYER_1);
        ui_intro(&players[0].btn);
        if (players[0].btn.pressed.start)
        {
            if (game->introTimer <= 0)
            {
                game->scene = GAMEPLAY;
            }
            else
            {
                game->introTimer = -1;
            }
        }
    }
    else if (game->scene == GAMEPLAY)
    {
        bool fps = false;
        if (players[0].btn.held.r)
            fps = true;

        ui_print(game, fps);

        if (game->playerCount == 4)
        {
            for (size_t out = 0; out < MAXPLAYERS; out++)
            {
                if (!players[out].isAlive)
                    ui_playerOut(&players[out]);
            }
        }
        if (players[0].btn.pressed.start && game->countDownTimer < COUNTDOWN_DELAY)
            game->scene = PAUSE;
    }
    else if (game->scene == PAUSE)
    {
        players[0].btn.pressed = joypad_get_buttons_pressed(PLAYER_1);
        players[0].btn.held = joypad_get_buttons_held(PLAYER_1);
        players[0].btn.released = joypad_get_buttons_released(PLAYER_1);
        ui_pause(&players[0].btn);
        if (players[0].btn.pressed.start)
            game->scene = GAMEPLAY;

        // Hold Z to quit on the Pause screen
        static int resetTimer = 0;
        if (game->scene == PAUSE && players[0].btn.held.z)
        {
            resetTimer++;
            if (resetTimer == 5)
            {
                sound_wavPlay(SFX_STOP, false);
            }
            else if (resetTimer >= 30)
            {
                game->isEnding = true;
                minigame_end();
            }
        }
        if (game->scene == PAUSE && players[0].btn.released.z)
            resetTimer = 0;
    }
    else // ENDING
    {
        ui_print(game, false);
    }

    rdpq_detach_show();
}

#endif // RENDER_H