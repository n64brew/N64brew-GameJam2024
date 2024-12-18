#ifndef OBJECTS_H
#define OBJECTS_H

#define GRID_SIZE 144 // BOX_SIZE plus 4 to be neatly divide by 9
#define CELL_SIZE 96
#define NUM_CELLS (GRID_SIZE * 2 / CELL_SIZE)
#define MAX_GRID_POINTS (NUM_CELLS * NUM_CELLS)

extern T3DVec3 gridPos[MAX_GRID_POINTS];
extern size_t gridPointCount;
extern T3DObject *buildings[2];
extern bool spray[NUM_OBJECTS];
extern bool stop[NUM_OBJECTS];

void generate_grid(void);
void object_init(object_data *object, uint8_t objectType, uint8_t ID, T3DVec3 position);
void object_initBatch(object_type *batch, uint8_t objectType);
void hydrant_water_spray(T3DVec3 position, T3DViewport *viewport);
void object_updateBatch(object_type *batch, T3DViewport *vp, player_data *player);
void object_cull(object_type *batch, T3DViewport *vp, int playercount);
void object_drawBatch(object_type *batch);
void object_destroyBatch(object_type *batch);

// Checks whether an object is close enough to the player's frustum to enable rendering
void object_cull(object_type *batch, T3DViewport *vp, int playercount)
{
  for (int o = 0; o < NUM_OBJECTS; o++)
  {
    // We want to find a sweet spot between performance and minimizing pop-in
    if (t3d_frustum_vs_sphere(&vp->viewFrustum, &gridPos[o], (60.0f * batch->collisionRadius) - (playercount * batch->collisionRadius)))
    {
      batch->objects[o].hide = false;
    }
    else
    {
      batch->objects[o].hide = true;
    }
  }
}

// Populates gridPos according to predefined settings
void generate_grid(void)
{
  gridPointCount = 0;

  // Calculate half the grid size
  int halfGridSize = GRID_SIZE;

  for (int i = 0; i < NUM_CELLS; i++)
  {
    for (int j = 0; j < NUM_CELLS; j++)
    {
      // Calculate cell center coordinates
      int x = -halfGridSize + j * CELL_SIZE + CELL_SIZE / 2;
      int z = -halfGridSize + i * CELL_SIZE + CELL_SIZE / 2;

      // Store the grid cell center position
      gridPos[gridPointCount++] = (T3DVec3){{(float)x, 10.0f, (float)z}};
    }
  }
}

void object_init(object_data *object, uint8_t objectType, uint8_t ID, T3DVec3 position)
{
  object->ID = ID;
  object->mtxFP = malloc_uncached(sizeof(T3DMat4FP));
  object->position = position;
  object->texID = 0;

  switch (objectType)
  {
  case OBJ_CAR:
    object->scale = (T3DVec3){{0.1f, 0.1f, 0.1f}};
    object->position.v[0] = fmaxf(-GRID_SIZE, fminf(GRID_SIZE, object->position.v[0] + (rand() % 20)));
    object->position.v[2] = fmaxf(-GRID_SIZE, fminf(GRID_SIZE, object->position.v[2] + 45));
    object->yaw = T3D_DEG_TO_RAD(90.0f);
    object->color = rand() % 2 == 0 ? color_from_packed32(0xE10916FF) : color_from_packed32(0x319900FF);
    break;
  case OBJ_BUILDING:
    object->scale = (T3DVec3){{0.3f, 0.3f, 0.3f}};
    object->yaw = 0;
    object->color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF);
    break;
  case OBJ_HYDRANT:
    object->scale = (T3DVec3){{0.06f, 0.06f, 0.06f}};
    object->position.v[0] = fmaxf(-GRID_SIZE, fminf(GRID_SIZE, object->position.v[0] + 25));
    object->position.v[2] = fmaxf(-GRID_SIZE, fminf(GRID_SIZE, object->position.v[2] + 25));
    object->yaw = 0;
    object->color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF);
    break;
  }

  object->visible = true;
  object->hide = false;
}

void object_initBatch(object_type *batch, uint8_t objectType)
{

  batch->type = objectType;

  // Assign model to objects
  switch (batch->type)
  {
  case OBJ_CAR:
    batch->scoreValue = 2;
    batch->model = modelCar;
    batch->collisionRadius = 0.2f;
    break;
  case OBJ_BUILDING:
    batch->scoreValue = 4;
    batch->model = modelBuilding;
    batch->collisionRadius = 0.4f;
    buildings[0] = t3d_model_get_object_by_index(modelBuilding, 1);
    buildings[1] = t3d_model_get_object_by_index(modelBuilding, 0);
    break;
  case OBJ_HYDRANT:
    batch->scoreValue = 1;
    batch->model = modelHydrant;
    batch->collisionRadius = 0.125f;
    break;
  }

  // Initialize batch objects
  generate_grid();
  for (size_t i = 0; i < NUM_OBJECTS; i++)
  {
    object_init(&batch->objects[i], batch->type, i, gridPos[i]);
  }

  // Create model block
  if (objectType == OBJ_BUILDING)
  {
    for (size_t i = 0; i < NUM_OBJECTS; i++)
    {
      rspq_block_begin();
      t3d_matrix_set(batch->objects[i].mtxFP, true);
      t3d_model_draw_object(buildings[0], NULL);

      batch->objects[i].modelBlock = rspq_block_end();
    }
  }
  else
  {
    for (size_t i = 0; i < NUM_OBJECTS; i++)
    {
      rspq_block_begin();

      t3d_matrix_set(batch->objects[i].mtxFP, true);
      rdpq_set_prim_color(batch->objects[i].color);
      t3d_model_draw(batch->model);

      batch->objects[i].modelBlock = rspq_block_end();
    }
  }
}

// Draws a transparent blue triangle with a randomized height
void hydrant_water_spray(T3DVec3 position, T3DViewport *viewport)
{

  T3DVec3 screenPos;
  t3d_viewport_calc_viewspace_pos(viewport, &screenPos, &position);

  int offset = 5;
  float upward = screenPos.y - rand() % 30;

  float v1[] = {screenPos.x, screenPos.y - offset};
  float v2[] = {screenPos.x - offset, upward};
  float v3[] = {screenPos.x + offset, upward};

  rdpq_sync_pipe();
  rdpq_set_mode_standard();
  rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
  rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
  rdpq_set_prim_color(RGBA32(40, 40, 240, 127));

  rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
}

// ALL object behaviors
void object_updateBatch(object_type *batch, T3DViewport *vp, player_data *player)
{
  for (size_t i = 0; i < NUM_OBJECTS; i++)
  {

    if (!batch->objects[i].visible)
      continue; // just skip the object update if not visible

    if (check_collision(&batch->objects[i].position, batch->collisionRadius, &player->playerPos, HITBOX_RADIUS * player->scale.x))
    {
      if (player->scale.x < batch->collisionRadius)
        continue;

      // Lower height according to batch's object size
      batch->objects[i].position.v[1] -= 0.4f / batch->collisionRadius;
      t3d_vec3_lerp(&batch->objects[i].scale, &batch->objects[i].scale, &(T3DVec3){{0.05f, 0.05f, 0.05f}}, fabsf(batch->objects[i].position.v[1]) * 0.001f);

      // SFX
      if (batch->objects[i].position.v[1] == 0.0f)
      {
        switch (batch->type)
        {
        case OBJ_CAR:
          sound_wavPlay(SFX_CAR, false);
          break;
        case OBJ_HYDRANT:
          sound_wavPlay(SFX_HYDRANT, true); // @TODO: Why is this one not playing at all?
          break;
        case OBJ_BUILDING:
          sound_wavPlay(SFX_BUILDING, false);
          break;
        }
      }

      // Cars spin out and get pulled towards the center
      if (batch->type == OBJ_CAR)
      {
        batch->objects[i].yaw -= .1f;
        batch->objects[i].position.x = t3d_lerp(batch->objects[i].position.x, player->playerPos.x, 0.04f);
        batch->objects[i].position.z = t3d_lerp(batch->objects[i].position.z, player->playerPos.z, 0.04f);
        stop[i] = true;
      }

      // Hydrants start spraying water after being collided with
      if (batch->type == OBJ_HYDRANT)
      {
        if (!spray[i])
          spray[i] = true;
      }

      // Buildings swap materials to create flickering illusion, shake and spin
      if (batch->type == OBJ_BUILDING)
      {
        batch->objects[i].texID = (int)fm_floorf(batch->objects[i].position.v[1]) % 8 == 0 ? 0 : 1;
        batch->objects[i].position.x = gridPos[i].x + 1.0f * fm_sinf(batch->objects[i].position.v[1]);
        batch->objects[i].yaw -= 0.01f;
      }

      // When the height is low enough, increment player score and disable object
      while (batch->objects[i].position.v[1] <= -80.0f * batch->collisionRadius)
      {
        player->score += batch->scoreValue;
        batch->objects[i].visible = false;
        break;
      }
    }
    else if (batch->type == OBJ_CAR)
    {

      // Car go vroom vroom
      if (batch->objects[i].position.x <= GRID_SIZE)
      {
        if (!stop[i])
          batch->objects[i].position.x += 0.2f + 0.1f * (rand() % 5);
      }
      else
      {
        batch->objects[i].position.x = -GRID_SIZE;
      }
    }

    // Update matrices
    t3d_mat4fp_from_srt_euler(
        batch->objects[i].mtxFP,
        batch->objects[i].scale.v,
        (float[3]){0, batch->objects[i].yaw, 0},
        batch->objects[i].position.v);
  }
}

void object_drawBatch(object_type *batch)
{
  if (batch->type == OBJ_BUILDING)
  {
    for (size_t i = 0; i < NUM_OBJECTS; i++)
    {
      // For the buildings we draw a material and object for each to allow each to flicker independently
      t3d_model_draw_material(buildings[batch->objects[i].texID]->material, NULL);
      if (batch->objects[i].visible && !batch->objects[i].hide)
      {
        rspq_block_run(batch->objects[i].modelBlock);
      }
    }
  }
  else
  {
    for (size_t i = 0; i < NUM_OBJECTS; i++)
    {
      if (batch->objects[i].visible && !batch->objects[i].hide)
        rspq_block_run(batch->objects[i].modelBlock);
    }
  }
  rspq_wait(); // RSPQ crashes if we don't wait for the objects to finish
}

void object_destroyBatch(object_type *batch)
{
  for (size_t i = 0; i < NUM_OBJECTS; i++)
  {
    free_uncached(batch->objects[i].mtxFP);
    rspq_block_free(batch->objects[i].modelBlock);
  }

  t3d_model_free(batch->model);
}

#endif // OBJECTS_H