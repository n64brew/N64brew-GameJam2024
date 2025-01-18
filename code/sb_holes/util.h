#ifndef UTIL_H
#define UTIL_H

// Global defines
// #define DEBUG_RDP

#define HITBOX_RADIUS 40.f

#define INTRO_DELAY 5.0f
#define COUNTDOWN_DELAY 3.0f
#define GO_DELAY 1.0f
#define WIN_DELAY 5.0f
#define WIN_SHOW_DELAY 2.0f

#define NUM_OBJECTS 9

#define SCREEN_WIDTH display_get_width()
#define SCREEN_HEIGHT display_get_height()

// STRUCTS
typedef struct
{
  T3DMat4FP *mtxFP;
  rspq_block_t *block;
  T3DModel *model;
} map_data;

typedef struct
{
  T3DVec3 position;
  T3DVec3 target;

} camera_data;

enum OBJ_TYPES
{
  OBJ_HYDRANT,
  OBJ_CAR,
  OBJ_BUILDING,
  NUM_OBJ_TYPES
};

typedef struct
{
  uint8_t ID;
  uint8_t texID;
  T3DObject *model;
  T3DMat4FP *mtxFP;
  T3DVec3 position;
  T3DVec3 scale;
  float yaw;
  bool visible;
  bool hide;
  color_t color;
  rspq_block_t *modelBlock;
} object_data;

typedef struct
{
  uint8_t type;
  uint8_t scoreValue;
  T3DModel *model;
  float collisionRadius;
  object_data objects[NUM_OBJECTS];
} object_type;

typedef struct
{
  joypad_buttons_t pressed;
  joypad_buttons_t held;
  joypad_buttons_t released;
} control_data;

typedef struct
{
  PlyNum plynum;
  T3DMat4FP *modelMatFP;
  rspq_block_t *dplHole;
  T3DVec3 moveDir;
  T3DVec3 playerPos;
  T3DVec3 scale;
  float rotY;
  float currSpeed;
  bool isAlive;
  PlyNum ai_targetPlayer;
  object_type *ai_targetBatch;
  object_data *ai_targetObject;
  uint8_t ai_reactionspeed;
  uint8_t score;
  control_data btn;
} player_data;

typedef struct
{
  size_t playerCount;
  float introTimer;
  float countDownTimer;
  bool isEnding;
  float endTimer;
  PlyNum winner;
  uint8_t scene;
  rspq_syncpoint_t syncPoint;
} game_data;

typedef void (*SceneLoop)(game_data *, float);
typedef struct
{
  uint8_t ID;
  uint8_t colorAmbient[4];
  uint8_t colorDir[4];
  T3DVec3 lightDirVec;
  SceneLoop loop;
  SceneLoop fixedLoop;
} scene_data;

enum SCENE_ID
{
  INTRO,
  GAMEPLAY,
  PAUSE,
  ENDING,
  NUM_SCENES
};

// Global variables
extern camera_data cam[MAXPLAYERS];

// COLLISION

float vec2_dist_squared(T3DVec3 *pos0, T3DVec3 *pos1);
bool check_collision(T3DVec3 *pos0, float radii0, T3DVec3 *pos1, float radii1);

// Calculate squared distance between two positions' X and Z coordinates
float vec2_dist_squared(T3DVec3 *pos0, T3DVec3 *pos1)
{
  float dx = pos1->v[0] - pos0->v[0];
  float dy = pos1->v[2] - pos0->v[2];
  float result = dx * dx + dy * dy;
  return result < FM_EPSILON ? 0.0f : result; // Clamp results near 0
}

// Return whether point is within a radius (radii0 == 0), or if 2 radii overlap (radii0 != 0)
bool check_collision(T3DVec3 *pos0, float radii0, T3DVec3 *pos1, float radii1)
{

  float distSq = vec2_dist_squared(pos0, pos1);

  assert(radii0 >= 0.0f && radii1 >= 0.0f);

  if (radii0 != 0)
  {
    // Assume checking if radii overlap
    float radiiSum = radii0 + radii1;
    float radiiSumSq = radiiSum * radiiSum;

    return distSq <= radiiSumSq;
  }
  else
  {

    // Assume checking if pos0 is within pos1's radius
    return distSq <= radii1 * radii1;
  }
}

#endif // UTIL_H