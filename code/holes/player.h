#ifndef PLAYER_H
#define PLAYER_H

extern player_data players[MAXPLAYERS];

extern T3DModel *model;

void player_init(player_data *player, color_t color, T3DVec3 position, float rotation);
void player_do_damage(player_data *player);
bool player_has_control(game_data *game, player_data *player);
void player_fixedloop(game_data *game, player_data *player, object_type *objects, float deltaTime, joypad_port_t port, bool is_human);
void player_loop(game_data *game, player_data *player, float deltaTime, joypad_port_t port, bool is_human);
void player_draw(player_data *player);
void player_cleanup(player_data *player);

void player_init(player_data *player, color_t color, T3DVec3 position, float rotation)
{
  player->modelMatFP = malloc_uncached(sizeof(T3DMat4FP));

  player->moveDir = (T3DVec3){{0, 0, 0}};
  player->playerPos = position;
  player->scale = (T3DVec3){{0.125f, 0.125f, 0.125f}};
  player->score = 0;
  memset(&player->btn, 0, sizeof(control_data));

  rspq_block_begin();
  t3d_matrix_set(player->modelMatFP, true);
  rdpq_set_prim_color(color);
  t3d_model_draw(model);
  player->dplHole = rspq_block_end();

  player->rotY = rotation;
  player->currSpeed = 0.0f;
  player->isAlive = true;
  player->ai_targetPlayer = rand() % MAXPLAYERS;
  player->ai_targetObject = rand() % NUM_OBJECTS;
  player->ai_reactionspeed = (2 - core_get_aidifficulty()) * 5 + rand() % ((3 - core_get_aidifficulty()) * 3);
}

float ai_target_priority_object(player_data *player, object_data *object)
{
  float priority = 0.0f;

  if (!object->visible)
    return 0.0f;

  if (player->score >= 20)
    priority -= 0.3f;

  float distance = sqrtf(vec2_dist_squared(&object->position, &player->playerPos));

  if (distance > 0 && object->visible)
    priority += 0.5f / distance;

  return priority;
}

float ai_target_priority_player(player_data *player, player_data *other)
{
  float priority = 0.0f;

  if (!other->isAlive)
    return 0.0f;

  if (player->scale.x > 0.4f)
    priority += 0.8f;

  if (other->scale.x < player->scale.x)
  {
    priority += 0.8f;
  }
  else if (other->scale.x > player->scale.x)
  {
    priority -= 0.3f;
  }

  priority *= (1.0f + 0.1f * core_get_aidifficulty());

  return priority;
}

void player_do_damage(player_data *player)
{
  if (!player->isAlive)
  {
    // Prevent edge cases
    return;
  }

  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    player_data *other_player = &players[i];
    if (other_player == player || !other_player->isAlive || other_player->scale.x > player->scale.x)
      continue;

    if (check_collision(&other_player->playerPos, 0, &player->playerPos, HITBOX_RADIUS * player->scale.x))
    {
      if (other_player->score < player->score)
      {
        other_player->isAlive = false;
        player->score += 5.0f;
      }
    }
  }
}

bool player_has_control(game_data *game, player_data *player)
{
  return player->isAlive && game->countDownTimer < 0.0f;
}

void player_fixedloop(game_data *game, player_data *player, object_type *objects, float deltaTime, joypad_port_t port, bool is_human)
{
  float speed = 0.0f;
  T3DVec3 newDir = {0};
  int deadzone = 3;

  if (player_has_control(game, player))
  {
    if (is_human)
    {
      joypad_inputs_t joypad = joypad_get_inputs(port);
      float moveX = 0;
      float moveY = 0;

      /** D Pad inputs
       * Why 4.0f? Since the control range is ~[-79,79],
       * the max absolute input value being used, `fabsf(joypad.stick_ * 0.05f)`,
       * would return ~4.0f
       */
      if (joypad.btn.d_up)
        moveY -= 4.0f;
      if (joypad.btn.d_down)
        moveY += 4.0f;
      if (joypad.btn.d_left)
        moveX -= 4.0f;
      if (joypad.btn.d_right)
        moveX += 4.0f;

      // Control Stick inputs
      if (fabsf(joypad.stick_x) >= deadzone || fabsf(joypad.stick_y) >= deadzone)
      {
        moveX += (float)joypad.stick_x * 0.05f;
        moveY -= (float)joypad.stick_y * 0.05f;
      }

      newDir.v[0] = moveX;
      newDir.v[2] = moveY;
      speed = sqrtf(t3d_vec3_len2(&newDir));
    }
    else
    {

      player_data *targetPlayer = &player[player->ai_targetPlayer];
      object_data *targetObject = &objects->objects[player->ai_targetObject];

      float playerPriority = ai_target_priority_player(player, targetPlayer);
      float objectPriority = ai_target_priority_object(player, targetObject);
      if (objectPriority > playerPriority)
      {

        if (targetObject->visible)
        {
          float dist, norm;
          newDir.v[0] = (targetObject->position.v[0] - player->playerPos.v[0]);
          newDir.v[2] = (targetObject->position.v[2] - player->playerPos.v[2]);
          dist = sqrtf(newDir.v[0] * newDir.v[0] + newDir.v[2] * newDir.v[2]);
          if (dist == 0)
            dist = 1;
          norm = 1 / dist;
          newDir.v[0] *= norm + (0.1f * core_get_aidifficulty());
          newDir.v[2] *= norm + (0.1f * core_get_aidifficulty());
          speed = 20;
        }
        else
        {
          player->ai_targetObject++;
        }
      }
      else if (objectPriority < playerPriority)
      {
        if (targetPlayer->isAlive && player->plynum != targetPlayer->plynum)
        {
          float dist, norm;
          newDir.v[0] = (targetPlayer->playerPos.v[0] - player->playerPos.v[0]);
          newDir.v[2] = (targetPlayer->playerPos.v[2] - player->playerPos.v[2]);
          dist = sqrtf(newDir.v[0] * newDir.v[0] + newDir.v[2] * newDir.v[2]);
          if (dist == 0)
            dist = 1;
          norm = 1 / dist;
          newDir.v[0] *= norm + (0.05f * core_get_aidifficulty());
          newDir.v[2] *= norm + (0.05f * core_get_aidifficulty());
          speed = 15;
        }
        else
        {
          player->ai_targetPlayer = 0; // (Attempt) to aquire a new target this frame
        }
      }
      else
      {
        player->ai_targetObject = rand() % NUM_OBJECTS;
        player->ai_targetPlayer = rand() % MAXPLAYERS;
      }
    }
  }

  // Player movement
  bool boost = game->playerCount > 1 ? true : false;
  if (speed > 0.15f)
  {
    newDir.v[0] /= speed;
    newDir.v[2] /= speed;
    player->moveDir = newDir;

    float newAngle = atan2f(player->moveDir.v[0], player->moveDir.v[2]);
    player->rotY = t3d_lerp_angle(player->rotY, newAngle, 0.5f);
    if (boost)
    {
      player->currSpeed = t3d_lerp(player->currSpeed, speed * 0.2f, 0.2f);
    }
    else
    {
      player->currSpeed = t3d_lerp(player->currSpeed, speed * 0.1f, 0.2f);
    }
  }
  else
  {
    player->currSpeed *= 0.6f;
  }

  // ...and limit position inside the box
  const float BOX_SIZE = 140.0f;
  if (player->playerPos.v[0] < -BOX_SIZE)
    player->playerPos.v[0] = -BOX_SIZE;
  if (player->playerPos.v[0] > BOX_SIZE)
    player->playerPos.v[0] = BOX_SIZE;
  if (player->playerPos.v[2] < -BOX_SIZE)
    player->playerPos.v[2] = -BOX_SIZE;
  if (player->playerPos.v[2] > BOX_SIZE)
    player->playerPos.v[2] = BOX_SIZE;

  player_do_damage(player);

  // Scaling based on score
  if (player->score >= 2 && player->score < 6)
  {
    if (player->scale.v[0] < 0.3f)
      player->scale.v[0] = t3d_lerp(player->scale.v[0], 0.25f, deltaTime);
    if (player->scale.v[2] < 0.3f)
      player->scale.v[2] = t3d_lerp(player->scale.v[2], 0.25f, deltaTime);
  }
  else if (player->score >= 6 && player->score < 10)
  {
    if (player->scale.v[0] < 0.6f)
      player->scale.v[0] = t3d_lerp(player->scale.v[0], 0.5f, deltaTime);
    if (player->scale.v[2] < 0.6f)
      player->scale.v[2] = t3d_lerp(player->scale.v[2], 0.5f, deltaTime);
  }
  else if (player->score >= 10)
  {
    if (player->scale.v[0] < 0.9f)
      player->scale.v[0] = t3d_lerp(player->scale.v[0], 0.8f, deltaTime);
    if (player->scale.v[2] < 0.9f)
      player->scale.v[2] = t3d_lerp(player->scale.v[2], 0.8f, deltaTime);
  }
}

void player_loop(game_data *game, player_data *player, float deltaTime, joypad_port_t port, bool is_human)
{
  if (is_human)
  {
    player->btn.pressed = joypad_get_buttons_pressed(port);
    player->btn.held = joypad_get_buttons_held(port);
    player->btn.released = joypad_get_buttons_released(port);
  }

  // move player...
  player->playerPos.v[0] += player->moveDir.v[0] * player->currSpeed;
  player->playerPos.v[2] += player->moveDir.v[2] * player->currSpeed;

  // Update player matrix
  if (player->isAlive)
  {
    t3d_mat4fp_from_srt_euler(player->modelMatFP,
                              player->scale.v,
                              (float[3]){0.0f, -player->rotY, 0},
                              player->playerPos.v);
  }
  else
  {
    player->currSpeed = 0;
    player->moveDir = (T3DVec3){{0}};
  }

  if (game->syncPoint)
    rspq_syncpoint_wait(game->syncPoint); // wait for the RSP to process the previous frame
}

void player_draw(player_data *player)
{
  if (player->isAlive)
    rspq_block_run(player->dplHole);
}

void player_cleanup(player_data *player)
{
  rspq_block_free(player->dplHole);
  free_uncached(player->modelMatFP);
}

#endif // PLAYER_H