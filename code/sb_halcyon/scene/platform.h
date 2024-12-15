#ifndef PLATFORM_H
#define PLATFORM_H

typedef struct
{

  Box box[3];

} PlatformCollider;

typedef struct
{

  uint32_t id;
  T3DMat4FP *mat;
  T3DObject *obj;
  Vector3 position;
  Vector3 home;
  PlatformCollider collider;
  color_t color;
  int colorID;
  uint32_t platformTimer;
  bool contact;

} Platform;

#define OFFSET 350
#define GRID_SIZE OFFSET // Size of each grid cell
#define MAX_GRID_CELLS 7 // Adjust based on level size

typedef struct
{
  size_t platformIndices[PLATFORM_COUNT]; // Indices of platforms in this cell
  size_t count;                           // Number of platforms in this cell
} PlatformGridCell;

PlatformGridCell platformGrid[MAX_GRID_CELLS][MAX_GRID_CELLS];

Platform hexagons[PLATFORM_COUNT];

Platform innerRing[6];
Platform outerRing[12];

int innerRingID[] = {4, 5, 8, 10, 13, 14};
int outerRingID[] = {0, 1, 2, 3, 6, 7, 11, 12, 15, 16, 17, 18};

// Forward Declarations

void platform_init(Platform *platform, T3DModel *model, Vector3 position, color_t color);
void platform_loop(Platform *platform, Actor *actor, int diff);
void platform_drawBatch(void);
void platform_hexagonGrid(Platform *platform, T3DModel *model, float z, color_t color);
void platform_destroy(Platform *platform);

// Definitions

void platform_init(Platform *platform, T3DModel *model, Vector3 position, color_t color)
{

  static uint32_t platformIdx = 0;

  platform->id = platformIdx;
  platform->mat = malloc_uncached(sizeof(T3DMat4FP)); // needed for t3d
  platform->position = position;
  platform->home = position;

  // Initialize the three boxes for collision within each hexagon
  for (int j = 0; j < 3; j++)
  {
    platform->collider.box[j] = (Box){
        .size = {200.0f, 300.0f, 75.0f},
        .center = platform->position,
        .rotation = {
            // Set only Z rotation explicitly
            0.0f,
            0.0f,
            (j == 0) ? 0.0f : (j == 1) ? 30.0f
                                       : -30.0f // Z rotation for boxes[0], boxes[1], and boxes[2]
        }};
  }

  platform->color = color; // Set color

  platform->colorID = -1;

  platform->platformTimer = 0;

  platform->contact = false;

  platformIdx++;
}

void platform_assignGrid(Platform *platforms)
{
  memset(platformGrid, 0, sizeof(platformGrid)); // Clear the grid

  for (size_t i = 0; i < PLATFORM_COUNT; i++)
  {
    int xCell = (int)fm_floorf((platforms[i].position.x + (OFFSET * 2)) / GRID_SIZE);
    int yCell = (int)fm_floorf((platforms[i].position.y + (OFFSET * 2)) / GRID_SIZE);

    if (xCell >= 0 && xCell < MAX_GRID_CELLS && yCell >= 0 && yCell < MAX_GRID_CELLS)
    {
      PlatformGridCell *cell = &platformGrid[xCell][yCell];
      if (cell->count < PLATFORM_COUNT) // Ensure we don't exceed bounds
        cell->platformIndices[cell->count++] = i;
    }
  }
}

//// BEHAVIORS ~ Start ////

// Example behavior: Oscillate platform x position to simulate shake
void platform_shake(Platform *platform, float time)
{
  float amplitude = 10.0f;        // Maximum oscillation distance from home
  float baseX = platform->home.x; // Use a stored "home" position for the base

  // Oscillate `platform->position.x` around `baseX`
  platform->position.x = baseX + amplitude * fm_sinf(time);
}

// Example behavior: Lower platform over time
void platform_updateHeight(Platform *platform, float time)
{
  if (platform->position.z > -150.0f)
    platform->position.z = platform->position.z - time;
}

void platform_dropGroup(Platform *platform, int groupID, float time)
{
  int ringSize; // Variable to hold the size of the current ring group

  if (groupID == 1)
  {
    ringSize = sizeof(innerRing) / sizeof(innerRing[0]);
    for (int i = 0; i < ringSize; i++)
    {
      platform_shake(&platform[innerRingID[i]], (time * 20) + rand() % 5);
      platform_updateHeight(&platform[innerRingID[i]], (time * 20) + rand() % 5);
    }
  }
  else
  {
    ringSize = sizeof(outerRing) / sizeof(outerRing[0]);
    for (int i = 0; i < ringSize; i++)
    {
      platform_shake(&platform[outerRingID[i]], (time * 20) + rand() % 5);
      platform_updateHeight(&platform[outerRingID[i]], (time * 20) + rand() % 5);
    }
  }
}

void platform_collideCheck(Platform *platform, Actor *actor)
{
  if (platform->contact)
    return; // If already in collided state, do nothing

  const float squaredDistanceThreshold = 150.0f * 150.0f;

  for (size_t i = 0; i < ACTOR_COUNT; i++)
  {
    // Skip actors that are not grounded (not relevant for collision)
    if (!actor[i].grounded)
      continue;

    // Calculate the difference vector
    Vector3 diff = vector3_difference(&platform->position, &actor[i].body.position);

    // Calculate squared distance
    float distanceSq = vector3_squaredMagnitude(&diff);

    if (distanceSq <= squaredDistanceThreshold)
    {
      platform->contact = true;
      return; // Exit early on first collision
    }
  }
}

void platform_collideCheckOptimized(Platform *platforms, Actor *actor)
{
  int xCell = (int)floorf((actor->body.position.x + (OFFSET * 2)) / GRID_SIZE);
  int yCell = (int)floorf((actor->body.position.y + (OFFSET * 2)) / GRID_SIZE);

  if (xCell < 0 || xCell >= MAX_GRID_CELLS || yCell < 0 || yCell >= MAX_GRID_CELLS)
  {
    return; // Actor is out of bounds
  }

  const float collisionRangeSq = 150.0f * 150.0f;

  // Check platforms in the same and adjacent cells
  for (int dx = -1; dx <= 1; dx++)
  {
    for (int dy = -1; dy <= 1; dy++)
    {
      int nx = xCell + dx;
      int ny = yCell + dy;

      if (nx >= 0 && nx < MAX_GRID_CELLS && ny >= 0 && ny < MAX_GRID_CELLS)
      {
        PlatformGridCell *cell = &platformGrid[nx][ny];
        for (size_t i = 0; i < cell->count; i++)
        {
          size_t platformIndex = cell->platformIndices[i];
          float distanceSq = vector3_squaredDistance(&actor->body.position, &platforms[platformIndex].position);
          if (distanceSq <= collisionRangeSq)
          {
            platforms[platformIndex].contact = true;
            return; // Early exit on collision
          }
        }
      }
    }
  }
}

void platform_loop(Platform *platform, Actor *actor, int diff)
{
  int difficulty = (core_get_playercount() == 4) ? diff : core_get_aidifficulty();

  // Translate collision
  for (int j = 0; j < 3; j++)
    platform->collider.box[j].center = platform->position;

  // Run behaviors
  // if(actor != NULL) platform_collideCheck(platform, actor);
  if (platform->contact)
  {
    platform->platformTimer++;

    // Action 1: Play sound
    if (platform->platformTimer > 0 && platform->platformTimer < 2)
    {
      sound_wavPlay(SFX_STONES, false);

      // Action 2: Shake platform
    }
    else if (platform->platformTimer < 120 - (difficulty * 10))
    {
      platform_shake(platform, platform->platformTimer);

      // Action 3: Reset to idle
    }
    else
    {
      // change the platform color to white
      platform->color = RGBA32(255, 255, 255, 255);
      platform->contact = false;
    }
  }
  else
  {

    // Reset to initial position
    platform->platformTimer = 0;
    // if(platform->position.z < platform->home.z) platform->position.z = platform->position.z + 1.0f + difficulty;
  }

  // Update matrix
  t3d_mat4fp_from_srt_euler(
      platform->mat,
      (float[3]){1.0f, 1.0f, 1.0f},
      (float[3]){0.0f, 0.0f, 0.0f},
      (float[3]){platform->position.x, platform->position.y, platform->position.z});
}

//// BEHAVIORS ~ End ////

//// RENDERING ~ Start ////

// T3D MODEL DRAW BATCHING
#define BATCH_LIMIT 1  // Number of objects per rspq block
#define BLOCK_COUNT 19 // Pre-calculated block count

T3DModel *batchModel = NULL;
rspq_block_t *rspqBlocks[BLOCK_COUNT] = {NULL}; // Static array of rspq block pointers
rspq_block_t *materialBlock = NULL;             // Block for single material load and draw

void platform_createBatch(Platform *platform, T3DModel *model)
{
  // Load model once for the entire batch if not already loaded
  if (batchModel == NULL)
  {
    batchModel = model;
  }

  // Create T3DObjects from batch for each platform
  for (size_t i = 0; i < PLATFORM_COUNT; i++)
  {
    platform[i].obj = t3d_model_get_object_by_index(batchModel, 0);
  }

  // Create material RSPQ block to run before drawing objects
  rspq_block_begin();
  t3d_model_draw_material(platform[0].obj->material, NULL);
  materialBlock = rspq_block_end();

  // Initialize the rspq block index and start a new rspq block
  size_t blockIndex = 0;
  rspq_block_begin();

  for (size_t i = 0; i < PLATFORM_COUNT; i++)
  {

    // Set the model matrix and draw
    t3d_matrix_set(platform[i].mat, true);
    t3d_model_draw_object(platform[i].obj, NULL);

    // End the current rspq block and start a new one every n objects
    if ((i + 1) % BATCH_LIMIT == 0 || i == PLATFORM_COUNT - 1)
    {
      rspqBlocks[blockIndex] = rspq_block_end(); // Store the completed rspq block
      blockIndex++;
      if (i < PLATFORM_COUNT - 1)
        rspq_block_begin(); // Start a new rspq block if more objects remain
    }
  }
}

// Iterate through and run RSPQ blocks
void platform_drawBatch(void)
{

  // Draw material once per batch
  rspq_block_run(materialBlock);

  for (size_t i = 0; i < BLOCK_COUNT; i++)
  {
    if (rspqBlocks[i] != NULL) // Check for NULL before running
    {
      rdpq_set_prim_color(hexagons[i].color);
      rspq_block_run(rspqBlocks[i]);
    }
  }
}

// Generate a hexagonal grid of 19 platforms at desired height, with desired model and color
void platform_hexagonGrid(Platform *platform, T3DModel *model, float z, color_t color)
{
  float x_offset = OFFSET; // Horizontal distance between centers of adjacent columns
  float y_offset = OFFSET; // Vertical distance between centers of adjacent rows
  float start_x = 0.0f;    // Starting X coordinate for the first row
  float start_y = -500.0f; // Starting Y coordinate for the first row

  int rows[] = {3, 4, 5, 4, 3}; // Number of hexagons per row
  int hexagon_index = 0;

  for (int row_index = 0; row_index < 5; row_index++)
  {
    int hex_count = rows[row_index];
    float row_start_x = start_x - (hex_count - 1) * x_offset / 2.0f;

    for (int i = 0; i < hex_count; i++)
    {
      platform[hexagon_index].position.x = row_start_x + i * x_offset;
      platform[hexagon_index].position.y = start_y + row_index * y_offset;
      platform[hexagon_index].position.z = z;
      hexagon_index++;
    }
  }

  // Initialize the platforms
  for (size_t p = 0; p < PLATFORM_COUNT; p++)
  {
    platform_init(&platform[p], model, platform[p].position, color);
  }

  platform_assignGrid(platform);

  // Assign platforms to ring by predefined IDs
  for (int g = 0; g < sizeof(innerRingID) / sizeof(innerRingID[0]); g++)
  {
    innerRing[g] = platform[innerRingID[g]];
  }
  for (int h = 0; h < sizeof(outerRingID) / sizeof(outerRingID[0]); h++)
  {
    outerRing[h] = platform[outerRingID[h]];
  }

  platform_createBatch(platform, model);
}

// Frees T3D model, matrices and RSPQ Blocks used for rendering
void platform_destroy(Platform *platform)
{
  rspq_block_free(materialBlock);

  for (size_t b = 0; b < BLOCK_COUNT; b++)
  {
    if (rspqBlocks[b] != NULL)
      rspq_block_free(rspqBlocks[b]);
  }

  for (size_t p = 0; p < PLATFORM_COUNT; p++)
  {
    if (platform[p].mat != NULL)
      free_uncached(platform[p].mat);
  }

  if (batchModel != NULL)
    t3d_model_free(batchModel);
}

#endif // PLATFORM_H