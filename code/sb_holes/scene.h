#ifndef SCENE_H
#define SCENE_H

extern scene_data scenes[NUM_SCENES];

void intro_fixedLoop(game_data *game, float deltaTime);
void intro_loop(game_data *game, float deltaTime);
void gameplay_fixedLoop(game_data *game, float deltaTime);
void gameplay_loop(game_data *game, float deltaTime);
void pause_fixedLoop(game_data *game, float deltaTime);
void pause_loop(game_data *game, float deltaTime);
void ending_fixedLoop(game_data *game, float deltaTime);
void ending_loop(game_data *game, float deltaTime);

void intro_fixedLoop(game_data *game, float deltaTime)
{
    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        for (int j = 0; j < NUM_OBJ_TYPES; j++)
        {
            player_fixedloop(game, &players[i], &objects[j], deltaTime, core_get_playercontroller(i), i < game->playerCount);
        }
    }
    sound_setChannels();
}
void intro_loop(game_data *game, float deltaTime)
{

    viewport_set(viewport, game->playerCount, cam);

    for (size_t i = 0; i < game->playerCount; i++)
    {

        for (int j = 0; j < NUM_OBJ_TYPES; j++)
        {
            for (size_t p = 0; p < MAXPLAYERS; p++)
            {
                object_updateBatch(&objects[j], &viewport[i], &players[p]);
            }
        }

        cam[i].position = (T3DVec3){{players[i].playerPos.x, players[i].playerPos.y + 250.0f, players[i].playerPos.z + 100.0f}};
        cam[i].target = players[i].playerPos;
    }

    for (size_t p = 0; p < MAXPLAYERS; p++)
    {
        player_loop(game, &players[p], deltaTime, core_get_playercontroller(p), p < game->playerCount);
    }

    render_scene(game, &scenes[game->scene]);
}

void gameplay_fixedLoop(game_data *game, float deltaTime)
{

    ////////// PLAYERS
    bool controlbefore = player_has_control(game, &players[0]);
    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        for (int j = 0; j < NUM_OBJ_TYPES; j++)
        {
            player_fixedloop(game, &players[i], &objects[j], deltaTime, core_get_playercontroller(i), i < game->playerCount);
        }
    }
    //////////

    sound_setChannels();
    if (game->introTimer > 0)
    {
        game->introTimer -= deltaTime;
    }
    if (game->scene == GAMEPLAY && game->countDownTimer > -GO_DELAY)
    {
        float prevCountDown = game->countDownTimer;
        game->countDownTimer -= deltaTime;
        if ((int)prevCountDown != (int)game->countDownTimer && game->countDownTimer >= 0)
            sound_wavPlay(SFX_COUNTDOWN, false);
    }
    if (!controlbefore && player_has_control(game, &players[0]))
        sound_wavPlay(SFX_START, false);

    if (!game->isEnding)
    {
        // Determine if a player has won
        uint32_t alivePlayers = 0;
        uint32_t humanPlayers = 0;
        int8_t highestScore = -1;
        static PlyNum highestScorePlayer = 0;
        PlyNum lastPlayer = 0;
        for (size_t i = 0; i < MAXPLAYERS; i++)
        {
            // Track players who are still alive
            if (players[i].isAlive)
            {
                if (i < game->playerCount)
                    humanPlayers++;
                alivePlayers++;
                lastPlayer = i;
            }

            // Track the player with the highest score
            if (players[i].score > highestScore)
            {
                highestScore = players[i].score;
                highestScorePlayer = i;
            }
        }

        if (alivePlayers == 1)
        {
            game->isEnding = true;
            game->winner = lastPlayer;
            if (game->playerCount != 4)
                sound_xmStop();
            sound_wavPlay(SFX_STOP, false);
            game->scene = ENDING;
        }

        if (humanPlayers <= 0)
        {
            game->isEnding = true;
            game->winner = highestScorePlayer;
            if (game->playerCount != 4)
                sound_xmStop();
            sound_wavPlay(SFX_STOP, false);
            game->scene = ENDING;
        }

    }
    else
    {
        float prevEndTime = game->endTimer;
        game->endTimer += deltaTime;
        if ((int)prevEndTime != (int)game->endTimer && (int)game->endTimer == WIN_SHOW_DELAY)
            sound_wavPlay(SFX_WINNER, false);
        if (game->endTimer > WIN_DELAY)
        {
            core_set_winner(game->winner);
            minigame_end();
        }
    }
}

void gameplay_loop(game_data *game, float deltaTime)
{

    viewport_set(viewport, game->playerCount, cam);

    if (game->scene == GAMEPLAY)
    {

        for (size_t i = 0; i < game->playerCount; i++)
        {

            for (int j = 0; j < NUM_OBJ_TYPES; j++)
            {
                for (size_t p = 0; p < MAXPLAYERS; p++)
                {
                    object_updateBatch(&objects[j], &viewport[i], &players[p]);
                }
            }

            cam[i].position = (T3DVec3){{players[i].playerPos.x, players[i].playerPos.y + 250.0f, players[i].playerPos.z + 100.0f}};
            cam[i].target = players[i].playerPos;
        }

        for (size_t p = 0; p < MAXPLAYERS; p++)
        {
            player_loop(game, &players[p], deltaTime, core_get_playercontroller(p), p < game->playerCount);
        }
    }

    render_scene(game, &scenes[game->scene]);
}

void pause_fixedLoop(game_data *game, float deltaTime)
{
    sound_setChannels();
}
void pause_loop(game_data *game, float deltaTime)
{
    render_scene(game, &scenes[game->scene]);
}

void scene_init(game_data *game, scene_data *scene)
{
    const uint32_t colors[4] = {
        0xAA0000FF,
        0x00AA00FF,
        0x0000AAFF,
        0xAAAA00FF,
    };
    for (size_t s = 0; s < NUM_SCENES; s++)
    {
        scene[s].ID = s;

        switch (s)
        {
        case INTRO:
            uint8_t tempAmb1[4] = {54, 40, 47, 0xFF};
            uint8_t tempDir1[4] = {0, 0, 0, 0xFF};
            for (int i = 0; i < 4; i++)
            {
                scene[s].colorAmbient[i] = tempAmb1[i];
                scene[s].colorDir[i] = tempDir1[i];
            }
            scene[s].lightDirVec = (T3DVec3){{0.0f, 1.0f, 0.0f}};
            scene[s].loop = intro_loop;
            scene[s].fixedLoop = intro_fixedLoop;
            break;
        case GAMEPLAY:
            uint8_t tempAmb2[4] = {54, 40, 47, 0xFF};
            uint8_t tempDir2[4] = {0xFF, 0xAA, 0xAA, 0xFF};
            for (int i = 0; i < 4; i++)
            {
                scene[s].colorAmbient[i] = tempAmb2[i];
                scene[s].colorDir[i] = tempDir2[i];
            }
            scene[s].lightDirVec = (T3DVec3){{0.0f, 1.0f, 1.0f}};
            scene[s].loop = gameplay_loop;
            scene[s].fixedLoop = gameplay_fixedLoop;
            break;
        case PAUSE:
            uint8_t tempAmb3[4] = {54, 40, 47, 0xFF};
            uint8_t tempDir3[4] = {0xAA, 0, 0, 0xFF};
            for (int i = 0; i < 4; i++)
            {
                scene[s].colorAmbient[i] = tempAmb3[i];
                scene[s].colorDir[i] = tempDir3[i];
            }
            scene[s].lightDirVec = (T3DVec3){{0.0f, -1.0f, 0.0f}};
            scene[s].loop = pause_loop;
            scene[s].fixedLoop = pause_fixedLoop;
            break;
        case ENDING:
            uint8_t tempAmb4[4] = {54, 40, 47, 0xFF};
            uint32_t winnerColor = colors[game->winner];
            uint8_t tempDir4[4] = {
                (winnerColor >> 24) & 0xFF,
                (winnerColor >> 16) & 0xFF,
                (winnerColor >> 8) & 0xFF,
                winnerColor & 0xFF};
            for (int i = 0; i < 4; i++)
            {
                scene[s].colorAmbient[i] = tempAmb4[i];
                scene[s].colorDir[i] = tempDir4[i];
            }
            scene[s].lightDirVec = (T3DVec3){{1.0f, 1.0f, 0.0f}};
            scene[s].loop = gameplay_loop;
            scene[s].fixedLoop = gameplay_fixedLoop;
            break;
        }

        t3d_vec3_norm(&scene[s].lightDirVec);
    }
}

#endif // SCENE_H