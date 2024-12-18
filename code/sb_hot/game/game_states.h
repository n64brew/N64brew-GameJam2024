#ifndef GAME_STATES_H
#define GAME_STATES_H

// Comment out to disable RSPQ Profiling
// #define PROFILING

#ifdef PROFILING
#include "rspq_profile.h"
static rspq_profile_data_t profile_data;
#endif

// function prototypes

void gameState_setIntro(Game *game, Player *player, Scenery *scenery);
void gameState_setMainMenu(Game *game, Player *player, Actor *actor, Scenery *scenery);
void gameState_setCS(Game *game, Player *player, Actor *actor, Scenery *scenery);

void gameState_setGameplay(Game *game, Player *player, AI *ai, Actor *actor, Scenery *scenery, ActorCollider *actor_collider, ActorContactData *actor_contact);
void gameState_setPause(Game *game, Player *player, Actor *actor, Scenery *scenery);

void gameState_setGameOver();

void game_play(Game *game, Player *player, AI *ai, Actor *actor, Scenery *scenery, ActorCollider *actor_collider, ActorContactData *actor_contact);

// new camera code ////

void camera_getMinigamePosition(Camera *camera, Actor *actor, Player *player, Vector3 camera_distance)
{
	Vector3 camera_target;

	uint8_t average_count = 0;

	vector3_init(&camera->position);
	vector3_init(&camera_target);

	for (uint8_t i = 0; i < ACTOR_COUNT; i++)
	{
		if (!player[i].died && player[i].isHuman)
		{

			vector3_add(&camera_target, &actor[player[i].actor_id].body.position);
			average_count++;
		}
	}

	if (average_count > 0)
		vector3_divideByNumber(&camera_target, average_count);
	camera_target.z = 200;

	if (camera_target.x != camera->target.x || camera_target.y != camera->target.y)
		camera->target = vector3_lerp(&camera->target, &camera_target, 0.2f);

	camera->position = camera->target;

	vector3_add(&camera->position, &camera_distance);
}

//////////////////////

void gameState_setIntro(Game *game, Player *player, Scenery *scenery)
{

	for (size_t j = 0; j < PLATFORM_COUNT; j++)
	{

		platform_loop(&hexagons[j], NULL, 0);
	}

	move_lava(scenery);

	// ======== Draw ======== //

	screen_clearDisplay(&game->screen);
	screen_clearT3dViewport(&game->screen);
	screen_applyColor_Depth(&game->screen, ui_color(DARK_RED), true);

	light_set(&game->scene.light);

	t3d_matrix_push_pos(1);

	room_draw(scenery);

	light_setAmbient(&game->scene.light, 0xBF);
	platform_drawBatch();
	light_resetAmbient(&game->scene.light);

	t3d_matrix_pop(1);

	game->syncPoint = rspq_syncpoint_new();

	// TPX
	ptx_draw(&game->screen.gameplay_viewport, &lavaBubbles, 1, 1);

	ui_intro(&player[0].control);

	if (player[0].control.held.r)
	{
		ui_fps(game->timing.frame_rate, 20.0f, 20.0f);
		if (core_get_playercount() == 1)
			ui_input_display(&player[0].control);
	}

	rdpq_detach_show();
	sound_update();
}

void gameState_setMainMenu(Game *game, Player *player, Actor *actor, Scenery *scenery)
{
	move_lava(scenery);

	// ======== Draw ======== //

	screen_clearDisplay(&game->screen);
	screen_clearT3dViewport(&game->screen);
	screen_applyColor_Depth(&game->screen, ui_color(DARK_RED), true);

	light_set(&game->scene.light);
	// Instead drawing a dark transparent texture over the scene, just change the light direction
	game->scene.light.direction1 = (T3DVec3){{-1, -1, -1}};

	t3d_matrix_push_pos(1);

	room_draw(scenery);

	light_setAmbient(&game->scene.light, 0xFF);
	platform_drawBatch();
	light_resetAmbient(&game->scene.light);

	t3d_matrix_pop(1);

	game->syncPoint = rspq_syncpoint_new();

	// TPX
	ptx_draw(&game->screen.gameplay_viewport, &lavaBubbles, 1, 1);

	if (core_get_playercount() == 4)
	{
		if (player[0].control.pressed.b)
		{
			if (game->diff <= 1)
			{
				game->diff++;
			}
			else
			{
				game->diff = 0;
			}
		}
	}
	ui_main_menu(&player[0].control, game->diff);
	if (player[0].control.held.r)
	{
		ui_fps(game->timing.frame_rate, 20.0f, 20.0f);
		if (core_get_playercount() == 1)
			ui_input_display(&player[0].control);
	}

	rdpq_detach_show();
	sound_update();

#ifdef PROFILING
	rspq_profile_next_frame();
	if (game->timing.frame_counter > 29)
	{
		rspq_profile_dump();
		rspq_profile_reset();
		game->timing.frame_counter = 0;
	}
	rspq_profile_get_data(&profile_data);
#endif // PROFILING
}

void gameState_setCS(Game *game, Player *player, Actor *actor, Scenery *scenery)
{

	static uint8_t activePlayer = 0;
	const uint8_t totalPlayers = core_get_playercount();
	static uint8_t selectedCharacter[ACTOR_COUNT] = {0};
	static bool actorSelected[ACTOR_COUNT] = {false};

	if (activePlayer >= 4)
	{
		game->state = GAMEPLAY;
		return;
	}

	controllerData_8way(&player[activePlayer].control);

	if (player[activePlayer].control.pressed.d_right)
	{
		uint8_t initialSelection = selectedCharacter[activePlayer];
		do
		{
			selectedCharacter[activePlayer] = (selectedCharacter[activePlayer] + 1) % ACTOR_COUNT;
		} while (actorSelected[selectedCharacter[activePlayer]] && selectedCharacter[activePlayer] != initialSelection);
	}

	if (player[activePlayer].control.pressed.d_left)
	{
		uint8_t initialSelection = selectedCharacter[activePlayer];
		do
		{
			selectedCharacter[activePlayer] = (selectedCharacter[activePlayer] - 1 + ACTOR_COUNT) % ACTOR_COUNT;
		} while (actorSelected[selectedCharacter[activePlayer]] && selectedCharacter[activePlayer] != initialSelection);
	}

	if (player[activePlayer].control.pressed.a)
	{
		// @TODO: More of a bandaid than a fix
		// Bugfix: Ensure selected actor is next available one
		if (!actorSelected[selectedCharacter[activePlayer]])
		{
			uint8_t selectedActorId = selectedCharacter[activePlayer];
			player[activePlayer].actor_id = selectedActorId;
			player[activePlayer].isHuman = true;
			actorSelected[selectedActorId] = true;

			// Visual feedback for selecting actor
			actor[selectedActorId].body.rotation.z = actor[selectedActorId].body.rotation.z + 180.0f;

			activePlayer++;
		}
		else
		{
			// Audio feedback for selecting unavailable actor
			sound_wavPlay(SFX_JUMP, false);
		}

		// Automatically assign actors to AI players
		if (activePlayer >= totalPlayers)
		{
			for (uint8_t i = totalPlayers; i < 4; i++) // AI players start after human players
			{
				for (uint8_t j = 0; j < ACTOR_COUNT; j++)
				{
					if (!actorSelected[j]) // Assign the first unselected actor
					{
						player[i].actor_id = j;
						player[i].isHuman = false;
						actorSelected[j] = true;
						break;
					}
				}
			}
			activePlayer = 4; // Lock selection
		}

		if (activePlayer >= 4)
		{
			game->state = GAMEPLAY;
			return;
		}
	}

	for (size_t i = 0; i < ACTOR_COUNT; i++)
	{
		actor_update(&actor[i], NULL, &game->timing, game->scene.camera.angle_around_barycenter, game->scene.camera.offset_angle, &game->syncPoint);

		// Reset non-selected actors
		if (!actorSelected[i])
		{
			actor[i].body.position = actor[i].home;
			actor[i].body.rotation.z = 0;
		}

		actor_updateMat(&actor[i]);
	}

	// Sync RSPQ once, and then update each actors' skeleton
	if (game->syncPoint)
		rspq_syncpoint_wait(game->syncPoint);
	for (size_t i = 0; i < ACTOR_COUNT; i++)
	{
		t3d_skeleton_update(&actor[i].armature.main);
	}

	move_lava(scenery);

	// ======== Draw ======== //

	screen_clearDisplay(&game->screen);
	screen_clearT3dViewport(&game->screen);
	screen_applyColor_Depth(&game->screen, ui_color(DARK_RED), true);

	light_set(&game->scene.light);

	// Change light direction to illuminate players
	game->scene.light.direction1 = (T3DVec3){{0, -1, 0}};
	game->scene.light.directional_color1[0] = 200;
	game->scene.light.directional_color1[1] = 200;
	game->scene.light.directional_color1[2] = 200;

	t3d_matrix_push_pos(1);

	room_draw(scenery);

	light_setAmbient(&game->scene.light, 0xBF);
	platform_drawBatch();
	light_resetAmbient(&game->scene.light);

	t3d_frame_start();
	actor_draw(actor);

	t3d_matrix_pop(1);

	game->syncPoint = rspq_syncpoint_new();

	// TPX
	lavaBubbles.count = 512;
	ptx_draw(&game->screen.gameplay_viewport, &lavaBubbles, 1, 1);

	if (activePlayer < MAXPLAYERS)
	{
		player[activePlayer].position.x = (actor[selectedCharacter[activePlayer]].body.position.x * 3.6f) - 30.0f;
		player[activePlayer].position.z = 125.0f;
		ui_print_playerNum(&player[activePlayer], &game->screen);
	}

	ui_character_select(&player[activePlayer].control, selectedCharacter[activePlayer]);

	if (player[0].control.held.r)
	{
		ui_fps(game->timing.frame_rate, 20.0f, 20.0f);
		if (core_get_playercount() == 1)
			ui_input_display(&player[0].control);
	}

	rdpq_detach_show();
	sound_update();
}

void gameState_setGameplay(Game *game, Player *player, AI *ai, Actor *actor, Scenery *scenery, ActorCollider *actor_collider, ActorContactData *actor_contact)
{

	if (!game->actorSet)
	{
		for (size_t i = 0; i < ACTOR_COUNT; i++)
		{
			actor[i].body.position = hexagons[9].position;				  // Center Platform
			actor[i].body.position.z = actor[i].body.position.z + 150.0f; // Adjust height to prevent spawning inside platform
			actor[i].home = actor[i].body.position;
		}
		game->actorSet ^= 1;
	}

	// ======== Countdown ======== //
	if (game->countdownTimer > 0)
	{
		if (game->countdownTimer % 44 == 0)
			sound_wavPlay(SFX_COUNTDOWN, false);

		if (game->countdownTimer == 3)
			sound_wavPlay(SFX_START, false);

		move_lava(scenery);

		// ======== Draw ======== //
		screen_clearDisplay(&game->screen);
		screen_clearT3dViewport(&game->screen);
		screen_applyColor_Depth(&game->screen, ui_color(DARK_RED), true);

		light_set(&game->scene.light);

		// Lerp light direction back to default during countdown
		T3DVec3 dirLightPos;
		t3d_vec3_lerp(&dirLightPos, &(T3DVec3){{1, 1, 1}}, &(T3DVec3){{0, -1, 0}}, (float)(game->countdownTimer) * 0.006f);
		game->scene.light.direction1 = dirLightPos;
		game->scene.light.directional_color1[0] = 200;
		game->scene.light.directional_color1[1] = 100;
		game->scene.light.directional_color1[2] = 50;

		t3d_matrix_push_pos(1);

		room_draw(scenery);

		light_setAmbient(&game->scene.light, 0xBF);
		platform_drawBatch();
		light_resetAmbient(&game->scene.light);

		t3d_matrix_pop(1);

		game->syncPoint = rspq_syncpoint_new();

		// TPX
		ptx_draw(&game->screen.gameplay_viewport, &lavaBubbles, 1, 1);

		// Convert frames to seconds based on refresh rate
		uint8_t secondsLeft = (game->countdownTimer / display_get_refresh_rate()) + 1;
		ui_countdown(secondsLeft);

		game->countdownTimer--;

		rdpq_detach_show();
		sound_update();
		return; // Exit early until countdown finishes
	}

	// ======== Gameplay ======== //

	// AI
#ifndef AI_BATTLE
	for (size_t i = 0; i < AI_COUNT; i++)
	{
		if (player[i + PLAYER_COUNT].died)
			continue;
		if (game->winnerSet)
			continue;
		ai_generateControlData(&ai[i], &player[i + PLAYER_COUNT].control, &actor[player[i + PLAYER_COUNT].actor_id], hexagons, game->scene.camera.offset_angle);
	}
#else
	for (size_t i = 0; i < AI_COUNT; i++)
	{
		if (player[i].died)
			continue;
		if (game->winnerSet)
			continue;
		ai_generateControlData(&ai[i], &player[i].control, &actor[player[i].actor_id], hexagons, game->scene.camera.offset_angle);
	}
#endif

	// Platforms
	for (size_t j = 0; j < PLATFORM_COUNT; j++)
	{
		if (!game->winnerSet)
			platform_loop(&hexagons[j], actor, game->diff);
	}

	move_lava(scenery);

	// Actors
	uint8_t loserCount = 0;
	uint8_t aliveCount = 0;
	uint8_t lastAlivePlayer = 0;

	for (size_t i = 0; i < ACTOR_COUNT; i++)
	{
		// Use player[i].actor_id to identify the assigned actor
		uint8_t actorIndex = player[i].actor_id;
		Actor *currentActor = &actor[actorIndex];
		// Sync player's position with the actor
		player[i].position = currentActor->body.position;
		if (currentActor->state != DEATH)
		{
			if (!game->winnerSet)
			{
				aliveCount++;
				lastAlivePlayer = i; // Track the last alive player
				// Update the assigned actor using its actor ID
				actor_update(currentActor, &player[i].control, &game->timing, game->scene.camera.angle_around_barycenter, game->scene.camera.offset_angle, &game->syncPoint);
				// Update collision data for the assigned actor
				actorCollision_collidePlatforms(currentActor, &actor_contact[actorIndex], &actor_collider[actorIndex], hexagons);

				// Update matrix
				actor_updateMat(currentActor);
			}
		}
		else
		{

			// Bugfix: Center dead actor's position to not break camera
			currentActor->body.position = (Vector3){0, 0, 250};
			static int8_t timer[MAXPLAYERS] = {0};
			if (player[i].isHuman)
			{
				if (!player[i].control.has_rumbled)
				{
					controllerData_rumbleFrames(&player[i].control, i, 5);
					if (++timer[i] > 25)
						player[i].control.has_rumbled = true;
				}
				else
				{
					controllerData_rumbleStop(&player[i].control, i);
				}
			}
			player[i].died = true;
			loserCount++;
		}
	}

	// Check if we have a winner (only one alive player left)
	if (aliveCount == 1 && !game->winnerSet)
	{
		core_set_winner(lastAlivePlayer); // Set the winner to the last remaining player
		game->winnerID = lastAlivePlayer;
		game->winnerSet = true;
	}

	// Sync RSPQ once, and then update each actors' skeleton
	if (game->syncPoint)
		rspq_syncpoint_wait(game->syncPoint);
	for (size_t i = 0; i < ACTOR_COUNT; i++)
	{
		t3d_skeleton_update(&actor[i].armature.main);
	}

	// ======== Draw ======== //

	screen_clearDisplay(&game->screen);
	screen_clearT3dViewport(&game->screen);
	screen_applyColor(&game->screen, ui_color(DARK_RED), true);

	light_set(&game->scene.light);

	// Reset light direction to default in case players have paused
	game->scene.light.direction1 = (T3DVec3){{1, 1, 1}};

	t3d_matrix_push_pos(1);

	room_draw(scenery);

	light_setAmbient(&game->scene.light, 0xBF);
	platform_drawBatch();
	light_resetAmbient(&game->scene.light);

	// Don't bother drawing shadows for the AI
	for (size_t s = 0; s < PLAYER_COUNT; s++)
	{
		if (actor[player[s].actor_id].state == FALLING || actor[player[s].actor_id].state == JUMP)
			player_drawShadow(actor[player[s].actor_id].body.position, &game->screen.gameplay_viewport);
	}

	t3d_frame_start(); // reset after drawing shadows

	actor_draw(actor);

	t3d_matrix_pop(1);

	game->syncPoint = rspq_syncpoint_new();

	// TPX
	if (!game->winnerSet)
		ptx_draw(&game->screen.gameplay_viewport, &lavaBubbles, 1, 1);

	for (size_t i = 0; i < ACTOR_COUNT; i++)
	{
		if (!game->winnerSet)
			ui_print_playerNum(&player[i], &game->screen);
	}

	if (loserCount == 3)
	{
		if (game->winnerSet)
		{
			game->winTimer++;
			if (game->winTimer == 3)
			{
				// XM takes up a lot of the buffer, have to free everything
				xm64player_stop(&xmPlayer);
				sound_wavClose(SFX_LAVA);
				sound_wavClose(SFX_JUMP);
				sound_wavClose(SFX_STONES);
				wait_ticks(4);
				sound_wavPlay(SFX_START, false); // Stop nor Winner will work, buffer too small
			}
			if (game->winTimer < 120)
				ui_print_winner(game->winnerID + 1);
			if (game->winTimer >= 118)
				game->state = GAME_OVER;
		}
	}
	else if (loserCount > 3)
	{
		game->winTimer++;
		if (game->winTimer == 3)
		{
			sound_wavClose(SFX_JUMP);
			sound_wavClose(SFX_STONES);
			wait_ticks(4);
			sound_wavPlay(SFX_START, false);
		}
		if (game->winTimer < 120)
			ui_print_winner(5);
		if (game->winTimer >= 118)
			game->state = GAME_OVER;
	}

	if (player[0].control.held.r)
	{
		ui_fps(game->timing.frame_rate, 20.0f, 20.0f);
		if (core_get_playercount() == 1)
			ui_input_display(&player[0].control);
	}

	rdpq_detach_show();
	sound_update();

#ifdef PROFILING
	rspq_profile_next_frame();
	if (game->timing.frame_counter > 29)
	{
		rspq_profile_dump();
		rspq_profile_reset();
		game->timing.frame_counter = 0;
	}
	rspq_profile_get_data(&profile_data);
#endif // PROFILING
}

void gameState_setPause(Game *game, Player *player, Actor *actor, Scenery *scenery)
{

	move_lava(scenery);

	// ======== Draw ======== //

	screen_clearDisplay(&game->screen);
	screen_clearT3dViewport(&game->screen);
	screen_applyColor(&game->screen, ui_color(BLACK), true);

	light_set(&game->scene.light);

	// Instead drawing a dark transparent texture over the scene, just change the light direction
	game->scene.light.direction1 = (T3DVec3){{-1, -1, -1}};

	t3d_matrix_push_pos(1);

	room_draw(scenery);

	light_setAmbient(&game->scene.light, 0xFF);
	platform_drawBatch();
	light_resetAmbient(&game->scene.light);

	actor_draw(actor);

	t3d_matrix_pop(1);

	game->syncPoint = rspq_syncpoint_new();

	// TPX
	ptx_draw(&game->screen.gameplay_viewport, &lavaBubbles, 1, 1);

	if (player[0].control.held.r)
	{
		ui_fps(game->timing.frame_rate, 20.0f, 20.0f);
		if (core_get_playercount() == 1)
			ui_input_display(&player[0].control);
	}
	else
	{
		ui_pause(&player[0].control);
	}

	rdpq_detach_show();
	sound_update();

#ifdef PROFILING
	rspq_profile_next_frame();
	if (game->timing.frame_counter > 29)
	{
		rspq_profile_dump();
		rspq_profile_reset();
		game->timing.frame_counter = 0;
	}
	rspq_profile_get_data(&profile_data);
#endif // PROFILING
}

void gameState_setGameOver()
{
	minigame_end();
}

void game_play(Game *game, Player *player, AI *ai, Actor *actor, Scenery *scenery, ActorCollider *actor_collider, ActorContactData *actor_contact)
{
	for (;;)
	{

		// Tme
		time_setData(&game->timing);

		// Controls
		game_setControlData(game, player);
		player_setControlData(player);

		// Hold Z to quit on the Pause screen
		static int resetTimer = 0;
		if (game->state == PAUSE && player[0].control.held.z)
		{
			resetTimer++;
			if (resetTimer == 30)
			{
				sound_wavPlay(SFX_JUMP, false);
			}
			else if (resetTimer > 40)
			{
				game->state = GAME_OVER;
			}
		}

		if (game->state == PAUSE && player[0].control.released.z)
			resetTimer = 0;

		//// CAMERA /////
		if (player[0].control.pressed.l)
			game->scene.camera.cam_mode ^= 1;

		Vector3 introStartPos = (Vector3){0, 3000, 100};
		Vector3 centerHex = hexagons[10].home;
		Vector3 csPos = (Vector3){0, -1000, 525};
		Vector3 gamePlayPos = (Vector3){0, -600, 1000};

		game->scene.camera.camTime += game->timing.fixed_time_s;

		if (game->state == INTRO)
		{
			float t = game->scene.camera.camTime / game->scene.camera.lerpTime;
			t = clamp(t, 0.0f, 1.0f);
			Vector3 camPos = vector3_lerp(&introStartPos, &centerHex, t);
			camera_getMinigamePosition(&game->scene.camera, actor, player, camPos);
		}
		else
		{

			if (game->scene.camera.cam_mode == 0)
			{
				camera_getOrbitalPosition(&game->scene.camera, csPos, game->timing.fixed_time_s);
			}
			else
			{
				float t = game->scene.camera.camTime / game->scene.camera.lerpTime;
				t = clamp(t, 0.0f, 1.0f);
				Vector3 camPos = vector3_lerp(&csPos, &gamePlayPos, t);
				if (game->state == PAUSE)
				{
					cameraControl_freeCam(&game->scene.camera, &player[0].control, game->timing.fixed_time_s);
				}
				else
				{
					camera_getMinigamePosition(&game->scene.camera, actor, player, camPos);
				}
			}
		}
		camera_set(&game->scene.camera, &game->screen);
		//// CAMERA /////

		// Sound: reverb
		mixer_ch_set_vol_pan(MUSIC_CHANNEL, sound_reverb(0.3f, 0.7f), 0.5f);
		for (int i = 0; i < SFX_WINNER; i++)
		{
			if (i < SFX_COUNTDOWN)
			{
				mixer_ch_set_vol_pan(SFX_CHANNEL - i, sound_reverb(0.9f, 0.6f), 0.5f);
			}
			else if (i != SFX_LAVA)
			{
				mixer_ch_set_vol_pan(SFX_CHANNEL - i, sound_reverb(0.5f, 0.8f), 0.5f);
			}
		}

		switch (game->state)
		{
		case INTRO:
		{
			gameState_setIntro(game, player, scenery);
			break;
		}
		case MAIN_MENU:
		{
			gameState_setMainMenu(game, player, actor, scenery);
			break;
		}
		case CHARACTER_SELECT:
		{
			gameState_setCS(game, player, actor, scenery);
			break;
		}
		case GAMEPLAY:
		{
			game->scene.camera.cam_mode = 1;
			for (uint8_t p = 0; p < game->humanCount; p++)
			{
				if (player[p].isHuman && player[p].died && !player[p].deathCounted)
				{
					game->deadPool++;
					player[p].deathCounted = true;
				}
			}
			if (game->deadPool == game->humanCount)
			{
				display_set_fps_limit(0);
			}
			else
			{
				display_set_fps_limit((display_get_refresh_rate() / 4) * 2);
			}
			gameState_setGameplay(game, player, ai, actor, scenery, actor_collider, actor_contact);
			break;
		}
		case PAUSE:
		{
			gameState_setPause(game, player, actor, scenery);
			break;
		}
		case GAME_OVER:
		{
			gameState_setGameOver();
			return;
		}
		}
	}
}

#endif