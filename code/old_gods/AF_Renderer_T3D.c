/*================
Implementation of AF_Renderer
Tiny3D rendering functions


==================*/

#include "AF_Renderer.h"
#include "AF_UI.h"
#include "ECS/Entities/AF_ECS.h"

#include "ECS/Entities/AF_ECS.h"
#include "AF_Physics.h"
#include "Assets.h"

// T3D headers
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>
#include "Assets.h"

#define DEBUG_RDP 0



// T3D variables and defines
/**
 * This shows how you can draw multiple objects (actors) in a scene.
 * Each actor has a scale, rotation, translation (knows as SRT) and a model matrix.
 */
#define ACTOR_COUNT 250
#define RAD_360 6.28318530718f
#define MODEL_SCALE_FACTOR 1.0f//0.075f
//static float objTimeLast = 0.0f;
//static float objTime = 0.0f;
//static float baseSpeed = 1.0f;

//static float rotAngle = 0.0f;
static T3DViewport viewport;
//static T3DVertPacked* vertices;
//static const T3DVec3 camPos = {{0,0,-18}};
//static const T3DVec3 camTarget = {{0,0,0}};
//static T3DMat4 modelMat; // matrix for our model, this is a "normal" float matrix
//static T3DMat4FP* snakeModelMatFP;// = malloc_uncached(sizeof(T3DMat4FP));
//static T3DMat4FP* mapMatFP;//; = malloc_uncached(sizeof(T3DMat4FP));


//static T3DVec3 rotAxis;
static uint8_t colorAmbient[4] = {80, 50, 50, 0xFF};
static uint8_t colorDir[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static T3DVec3 lightDirVec = {{1.0f, 1.0f, 0.0f}};
//static uint8_t lightDirColor[4] = {120, 120, 120, 0xFF};
//static T3DMat4FP* snakeModelMatFP;
//static rspq_block_t *dplDraw;

static T3DVec3 camPos = {{0, 45.0f, 10.0f}};//{{100.0f,25.0f,0}};
static T3DVec3 camTarget = {{0, 0,-10}};//{{0,0,0}};

//rspq_block_t *dplSnake;
//rspq_block_t *dplMap;
//rspq_block_t *dpls[AF_ECS_TOTAL_ENTITIES];

// TODO: i dont like this
static T3DModel *models[MODEL_COUNT];
// setup and have ready animation data 
//T3DSkeleton skeletons[MODEL_COUNT];
//T3DSkeleton skeletonBlends[MODEL_COUNT];
//T3DAnim animIdles[MODEL_COUNT];
//T3DAnim animWalks[MODEL_COUNT];
//T3DAnim animAttacks[MODEL_COUNT];

typedef struct Renderer_Animation{
  BOOL has;
  T3DModel* model; 
  T3DSkeleton skeleton; 
  T3DSkeleton skeletonBlend;
  const char* animIdlePath;
  T3DAnim animIdle;
  const char* animWalkPath;
  T3DAnim animWalk;
  const char* animAttackPath;
  T3DAnim animAttack;
  float currentAnimationSpeed;
  float blend;
} Renderer_Animation;

Renderer_Animation animations[MODEL_COUNT];

const char* snakeIdlePath = "Snake_Idle";
const char* snakeWalkPath = "Snake_Walk";
const char* snakeAttackPath = "Snake_Attack";

// Holds our actor data, relevant for t3d is 'modelMat'.
/*
typedef struct {
  uint32_t id;
  float pos[3];
  float rot[3];
  float scale[3];

  rspq_block_t *dpl;
  T3DMat4FP *modelMat;
} Actor;
*/

// add a void* called commandBuff to mesh component
// add 4x4 matrix to mesh component

//static int actorCount = 1;

//static Actor actors[ACTOR_COUNT];
//static const int triangleCount[2] = {12, 60}; // used for the debug overlay

float get_time_s()  { return (float)((double)get_ticks_ms() / 1000.0); }
float get_time_ms() { return (float)((double)get_ticks_us() / 1000.0); }
void AF_Renderer_LoadAnimation(Renderer_Animation* _animation);

// Animation stuff
// TODO: move to component
static T3DModel *modelMap;// = t3d_model_load("rom:/map.t3dm");
static T3DModel *modelShadow;// = t3d_model_load("rom:/shadow.t3dm");

float lastTime;

// TODO: move to component
//static T3DSkeleton skel;
//static T3DSkeleton skelBlend;
//static T3DAnim animIdle;
//T3DAnim animWalk;
//T3DAnim animAttack;
//bool isAttack;
//T3DVec3 moveDir;
//T3DVec3 playerPos;
//float rotY = 0.0f;
//float currSpeed = 0.0f;
//float animBlend = 0.0f;


rspq_syncpoint_t syncPoint;
float newTime;
float deltaTime;

// Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
static T3DModel *snakeModel;// = t3d_model_load("rom:/snake.t3dm");

/*
Actor actor_create(uint32_t id, rspq_block_t *dpl)
{
  float randScale = (rand() % 100) / 3000.0f + 0.03f;
  Actor actor = (Actor){
    .id = id,
    .pos = {0, 0, 0},
    .rot = {0, 0, 0},
    .scale = {randScale, randScale, randScale},
    .dpl = dpl,
    .modelMat = malloc_uncached(sizeof(T3DMat4FP)) // needed for t3d
  };
  t3d_mat4fp_identity(actor.modelMat);
  return actor;
}
*/
/*
void actor_update(Actor *actor) {
  //actor->pos[0] = 0;

  // set some random position and rotation
  //float randRot = (float)fm_fmodf(actor->id * 123.1f, 5.0f);
  //float randDist = (float)fm_fmodf(actor->id * 4645.987f, 30.5f) + 10.0f;

  //actor->rot[0] = fm_fmodf(randRot + objTime * 1.05f, RAD_360);
  //actor->rot[1] = fm_fmodf(randRot + objTime * 1.03f, RAD_360);
  //actor->rot[2] = fm_fmodf(randRot + objTime * 1.1f, RAD_360);

  //actor->pos[0] = randDist * fm_cosf(objTime * 1.6f + randDist);
  //actor->pos[1] = randDist * fm_sinf(objTime * 1.5f + randRot);
  //actor->pos[2] = randDist * fm_cosf(objTime * 1.4f + randDist*randRot);

  // t3d lets you directly construct a fixed-point matrix from SRT
  //t3d_mat4fp_from_srt_euler(actor->modelMat, actor->scale, actor->rot, actor->pos);
}


void actor_draw(Actor *actor) {
  t3d_matrix_set(actor->modelMat, true);
  rspq_block_run(actor->dpl);
}




void actor_delete(Actor *actor) {
  free_uncached(actor->modelMat);
}
*/


// TODO: move this into the app data, perhaps under "RenderData"
// Global Camera
//static camera_t camera;

// Gloab frame counter
//static uint64_t frames = 0;

// Global shader settings
//static GLenum shade_model = GL_SMOOTH;
//static bool fog_enabled = false;

//static const GLfloat environment_color[] = { 0.2f, 0.2f, 0.2f, 1.f };
// Define the RGBA values for the ambient light (e.g., soft white light)
//static const GLfloat ambientLight[] = {0.75f, 0.75f, 0.75f, 1.0f};  // R, G, B, A


// Textures
//static GLuint textures[TEXTURE_COUNT];
//static sprite_t *sprites[TEXTURE_COUNT];

// forward declare
void Renderer_RenderMesh(AF_CMesh* _mesh, AF_CTransform3D* _transform, float _dt);


/*=================
AF_LoadTexture

Loads a texture from a file and creates an OpenGL texture.

Parameters:
- _texturePath: Path to the texture image file.

Returns:
- GLuint as uint32_t: The OpenGL texture ID.

Steps:
1. Load texture data. Log error if loading fails.
2. Generate an OpenGL texture ID. Log error if generation fails.
3. Bind the texture and set GL_NEAREST filtering.
4. Apply texture data and parameters.

=================*/
uint32_t AF_LoadTexture(const char* _texturePath){
    /*
    sprite_t* textureData;
    textureData = sprite_load(_texturePath);
    if(textureData == NULL)
    {
        debugf("Renderer:Init: Failed to load texture\n");
    }

    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    debugf("defaultTextureID %li \n", textureID);
    if(textureID == 0)
    {
        debugf("Renderer:Init: Failed to create texture buffer in glGenTextures\n");
    }

    // Default texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Bind texture to textureData and set texture parameters
    glSpriteTextureN64(GL_TEXTURE_2D, textureData, &(rdpq_texparms_t){.s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE});
    return (uint32_t)textureID;
    */
   debugf("AF_Renderer_T3D: AF_LoadTexture: To be implemented\n");
   return 0;
}

// TODO: turn the params into a struct to pass in
void AF_Renderer_LoadAnimation(Renderer_Animation* _animation){  
    //assert(_animation->skeleton != NULL && "skeleton null \n");
    //assert(_animation->skeletonBlend  != NULL && "skeleton null \n");
    //assert(_animation->animIdle != NULL && "skeleton null \n");
    //assert(_animation->animWalk != NULL && "skeleton null \n");
    //assert(_animation->animAttack != NULL && "skeleton null \n");
    assert(_animation->animIdlePath != NULL);
    assert(_animation->animWalkPath != NULL);
    assert(_animation->animAttackPath != NULL);

   // First instantiate skeletons, they will be used to draw models in a specific pose
    // And serve as the target for animations to modify
   _animation->skeleton = t3d_skeleton_create(_animation->model);
   assert(&_animation->skeleton != NULL);
   _animation->skeletonBlend = t3d_skeleton_clone(&_animation->skeleton, false); // optimized for blending, has no matrices
   assert(&_animation->skeletonBlend != NULL);
   // Now create animation instances (by name), the data in 'model' is fixed,
   // whereas 'anim' contains all the runtime data.
   // Note that tiny3d internally keeps no track of animations, it's up to the user to manage and play them.
   _animation->animIdle = t3d_anim_create(_animation->model, _animation->animIdlePath);// "Snake_Idle");
   assert(&_animation->animIdle != NULL);
   t3d_anim_attach(&_animation->animIdle, &_animation->skeleton); // tells the animation which skeleton to modify

  _animation->animWalk = t3d_anim_create(_animation->model, _animation->animWalkPath);//"Snake_Walk");
  t3d_anim_attach(&_animation->animWalk, &_animation->skeletonBlend);

  // multiple animations can attach to the same skeleton, this will NOT perform any blending
  // rather the last animation that updates "wins", this can be useful if multiple animations touch different bones
  _animation->animAttack = t3d_anim_create(_animation->model, _animation->animAttackPath);// "Snake_Attack");
  assert(&_animation->animAttack != NULL);
  t3d_anim_attach(&_animation->animAttack, &_animation->skeleton);


  t3d_anim_set_looping(&_animation->animAttack, false); // don't loop this animation
  t3d_anim_set_playing(&_animation->animAttack, false); // start in a paused state
  
}

// Init Rendering
void AF_Renderer_Init(AF_ECS* _ecs){
    assert(_ecs != NULL && "AF_Renderer_T3D: Renderer_Init has null ecs referenced passed in \n");
    debugf("AF_Renderer_T3D: AF_Renderer_Init: To be implemented\n");
   	
	debugf("InitRendering\n");

    // Tindy 3D Init stuff
    t3d_init((T3DInitParams){}); // Init library itself, use empty params for default settings
    
    
    // register some font
    //rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

    //snakeModelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    //mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
    //t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0, 0, -10});



    // Load a some models
    //rspq_block_t *dpls[MODEL_COUNT];
    /*T3DModel *models[2] = {
        t3d_model_load("rom:/old_gods/box.t3dm"),
        t3d_model_load("rom:/old_gods/food.t3dm")
    };*/
   
   
   
    // bulk load an instance of each model type only once.
    for(int i = 0; i < MODEL_COUNT; ++i){
         models[i] = t3d_model_load(model_paths[i]);
         // scale the model
         // TODO read teh model scale from a variable in the mesh
         // load animations
         
    }

    // setup the animations
    animations[MODEL_BOX].has = FALSE;
    animations[MODEL_FOOD].has = FALSE;
    animations[MODEL_SHADOW].has = FALSE;
    animations[MODEL_MAP].has = FALSE;

    animations[MODEL_SNAKE].has = TRUE;
    animations[MODEL_SNAKE].model = models[MODEL_SNAKE];
    animations[MODEL_SNAKE].animIdlePath = snakeIdlePath;
    animations[MODEL_SNAKE].animWalkPath = snakeWalkPath;
    animations[MODEL_SNAKE].animAttackPath = snakeAttackPath;
    AF_Renderer_LoadAnimation(&animations[MODEL_SNAKE]);
   
    

    // ===========Animation models
    modelMap = models[MODEL_MAP];// t3d_model_load("rom:/old_gods/map.t3dm");
    modelShadow = models[MODEL_SHADOW];//t3d_model_load("rom:/old_gods/shadow.t3dm");

    // Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
    snakeModel = models[MODEL_SNAKE];//t3d_model_load("rom:/old_gods/snake.t3dm");
    

   


  

    // Setup the render instruction block for the snake
    /*
  rspq_block_begin();
    t3d_matrix_push(snakeModelMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw_skinned(snakeModel, &skel); // as in the last example, draw skinned with the main skeleton

    rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
    t3d_model_draw(modelShadow);
    t3d_matrix_pop(1);
  dplSnake = rspq_block_end();




  rspq_block_begin();
    t3d_matrix_push(mapMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(modelMap);
    t3d_matrix_pop(1);
  dplMap = rspq_block_end();
*/

    // ===========


    
    
    // this creates a set of actors that has assigned a dpls to each actor to then
    /*
    for(int i=0; i<ACTOR_COUNT; ++i) {
        //actors[i] = actor_create(i, dpls[i*3 % MODEL_COUNT]);//2]);
        actors[i] = actor_create(i, dpls[i* (MODEL_COUNT+1) % MODEL_COUNT]);//2]);
    }*/

    
    
    
    t3d_vec3_norm(&lightDirVec);

    // TODO: delete all this
    // Allocate vertices (make sure to have an uncached pointer before passing it to the API!)
    // For performance reasons, 'T3DVertPacked' contains two vertices at once in one struct.
    //vertices = malloc_uncached(sizeof(T3DVertPacked) * 2);

    //uint16_t norm = t3d_vert_pack_normal(&(T3DVec3){{ 0, 0, 1}}); // normals are packed in a 5.6.5 format
    /*vertices[0] = (T3DVertPacked){
        //.posA = {-16, -16, 0}, .rgbaA = 0xFF0000'FF, .normA = norm,
        //.posB = { 16, -16, 0}, .rgbaB = 0x00FF00'FF, .normB = norm,
        .posA = {-16, -16, 0}, .rgbaA = 0xFF0000FF, .normA = norm,
        .posB = { 16, -16, 0}, .rgbaB = 0x00FF00FF, .normB = norm,
    };
    vertices[1] = (T3DVertPacked){
        //.posA = { 16,  16, 0}, .rgbaA = 0x0000FF'FF, .normA = norm,
        //.posB = {-16,  16, 0}, .rgbaB = 0xFF00FF'FF, .normB = norm,
        .posA = { 16,  16, 0}, .rgbaA = 0x0000FFFF, .normA = norm,
        .posB = {-16,  16, 0}, .rgbaB = 0xFF00FFFF, .normB = norm,
    };*/

    //rotAngle = 0.0f;

    //T3DVec3 newAxis = {{-1.0f, 2.5f, 0.25f}};
    //rotAxis = newAxis;
    //t3d_vec3_norm(&rotAxis);

    // create a viewport, this defines the section to draw to (by default the whole screen)
    // and contains the projection & view (camera) matrices
    viewport = t3d_viewport_create();


    // Setup lights
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);
    //dplDraw = NULL;


    // ========= Animation stuff
    lastTime = get_time_s() - (1.0f / 60.0f);
    syncPoint = 0;

    //T3DVec3 ogMoveDir = {{0,0,0}};
    //T3DVec3 ogPlayerPos = {{0,0.15f,0}};
    //moveDir = ogMoveDir;
    //playerPos = ogPlayerPos;

    //rotY = 0.0f;
    animations[MODEL_SNAKE].currentAnimationSpeed = 0.0f;
    animations[MODEL_SNAKE].blend = 0.0f;
    //currSpeed = 0.0f;
    //animBlend = 0.0f;
    //isAttack = false;
}

// rendering stuff that needs to happen after game start or awake
void AF_Renderer_LateStart(AF_ECS* _ecs){
    // TODO: render this based off the entities with models.
    // Render a model based on the model loaded by referencing its ID found in the mesh->modelID
    
    for(int i=0; i<_ecs->entitiesCount; ++i) {
        AF_CMesh* mesh = &_ecs->meshes[i];
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            // Only process the entities that have mesh componets with a mesh
            // initialise modelsMat
            // TODO: i don't like this
            //modelsMat[i] = malloc_uncached(sizeof(T3DMat4FP));
            T3DMat4FP* meshMat = malloc_uncached(sizeof(T3DMat4FP));
            //debugf("AF_Renderer_T3D: AF_RenderInit modelsMat address: %p \n",meshMat);
            mesh->modelMatrix = (void*)meshMat;
            //mesh->modelMatrix = (Mat4FP*)(sizeof(T3DMat4FP));
            if(mesh->modelMatrix == NULL){
                debugf("AF_Renderer_T3D: AF_RenderInit modelsMat %i mesh ID %i mesh type %i is null\n",i, mesh->meshID, mesh->meshType);
                continue;
            }
            rspq_block_begin();

                
            //t3d_matrix_push(modelsMat[i]);
            t3d_matrix_push(mesh->modelMatrix);
            
            // TODO: put a flag in that can signal a mesh is skinned
            // The snake model needs special color and call to skinned command
            if(mesh->meshID == MODEL_SNAKE){
                rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                t3d_model_draw_skinned(models[MODEL_SNAKE], &animations[MODEL_SNAKE].skeleton); // as in the last example, draw skinned with the main skeleton

                //rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
                //t3d_model_draw(models[MODEL_SHADOW]);
            }
            
            else{
                rdpq_set_prim_color(RGBA32(0, 0, 0, 0xFF));
                //models[i].
                //debugf("AF_Renderer_T3D: AF_RenderInit modelsMat %i mesh ID %i mesh type %i \n",i, mesh->meshID, mesh->meshType);
                //t3d_model_draw(models[mesh->meshType]);
                //t3d_model_draw(models[mesh->meshType]);
                t3d_model_draw(models[mesh->meshID]);
                //rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
                //t3d_model_draw(models[MODEL_SHADOW]);
            }
            t3d_matrix_pop(1);
            //dpls[i] = rspq_block_end();
            // cast to void pointer to store in our mesh data, which knows nothing about T3D
            mesh->displayListBuffer = (void*)rspq_block_end();
        }
    }

}



// Update Renderer
// TODO: take in an array of entities 
void AF_Renderer_Update(AF_ECS* _ecs, AF_Time* _time){
	assert(_ecs != NULL && "AF_Renderer_T3D: AF_Renderer_Update has null ecs referenced passed in \n");
    //debugf("AF_Renderer_T3D: AF_Renderer_Init: To be implemented\n");


    float newTime = get_time_s();
    //float deltaTime = (newTime - objTimeLast) * baseSpeed;
    //objTimeLast = newTime;
    //objTime += deltaTime;

    float totalRenderTime = get_time_ms();
    
    //for(int i=0; i<actorCount; ++i) {
    //  actor_update(&actors[i]);
   // }
    //totalRenderTime = get_time_ms() - totalRenderTime;

    //t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.0f), 10.0f, 100.0f);
    //t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // ======= Animation stuff =======
    // ======== Update ======== //
    //joypad_poll();

    newTime = get_time_s();
    deltaTime = newTime - lastTime;
    lastTime = newTime;

    //joypad_inputs_t joypad = joypad_get_inputs(JOYPAD_PORT_1);
    //joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    /*
    T3DVec3 newDir = {{
       (float)joypad.stick_x * 0.05f, 0,
      -(float)joypad.stick_y * 0.05f
    }};
    float speed = sqrtf(t3d_vec3_len2(&newDir));

    // Player Attack
    if((btn.a || btn.b) && !animAttack.isPlaying) {
      t3d_anim_set_playing(&animAttack, true);
      t3d_anim_set_time(&animAttack, 0.0f);
      isAttack = true;
    }

    // Player movement
    if(speed > 0.15f && !isAttack) {
      newDir.v[0] /= speed;
      newDir.v[2] /= speed;
      moveDir = newDir;

      //float newAngle = atan2f(moveDir.v[0], moveDir.v[2]);
      //rotY = t3d_lerp_angle(rotY, newAngle, 0.25f);
      currSpeed = t3d_lerp(currSpeed, speed * 0.15f, 0.15f);
    } else {
      currSpeed *= 0.8f;
    }

    */
    // use blend based on speed for smooth transitions
    

    // move player...
    //playerPos.v[0] += moveDir.v[0] * currSpeed;
    //playerPos.v[2] += moveDir.v[2] * currSpeed;
    // ...and limit position inside the box
    //const float BOX_SIZE = 20.0f;//140.0f;
    //if(playerPos.v[0] < -BOX_SIZE)playerPos.v[0] = -BOX_SIZE;
    //if(playerPos.v[0] >  BOX_SIZE)playerPos.v[0] =  BOX_SIZE;
    //if(playerPos.v[2] < -BOX_SIZE)playerPos.v[2] = -BOX_SIZE;
    //if(playerPos.v[2] >  BOX_SIZE)playerPos.v[2] =  BOX_SIZE;

    // position the camera behind the player
    T3DVec3 vec3Zero = {{0.0f,0.0f,0.0f}};
    camTarget = vec3Zero;//playerPos;
    float zDist = 2.5;
    camTarget.v[2] += zDist;//40;
    camPos.v[0] = camTarget.v[0];
    camPos.v[1] = camTarget.v[1] +7;// + 45;
    camPos.v[2] = camTarget.v[2] + 5;//65;

    //t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 10.0f, 150.0f);
    //t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 3.0f, 220.0f);
    //float aspectRatio = 4.0f / 3.0f;  // Aspect ratio for a 320x240 window

    //t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.0f), aspectRatio, 100.0f);

    //
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(45.0f), 1.0f, 1000.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // ========== ANIM BLEND =========

    // if mesh has animation
    // update all the anim datas
    // idle
    // walk
    // attack
    // animIsPlaying
    //animBlend = currSpeed / 0.51f;
    animations[MODEL_SNAKE].blend = animations[MODEL_SNAKE].currentAnimationSpeed / 0.51f;
    //if(animBlend > 1.0f)animBlend = 1.0f;
    if(animations[MODEL_SNAKE].blend  > 1.0f){
      animations[MODEL_SNAKE].blend = 1.0f;
    }
    // Update the animation and modify the skeleton, this will however NOT recalculate the matrices
    //t3d_anim_update(&animIdle, deltaTime);
    t3d_anim_update(&animations[MODEL_SNAKE].animIdle, deltaTime);
    
    //t3d_anim_set_speed(&animWalk, animBlend + 0.15f);
    t3d_anim_update(&animations[MODEL_SNAKE].animWalk, deltaTime);
    
    //t3d_anim_update(&animWalk, deltaTime);
    t3d_anim_update(&animations[MODEL_SNAKE].animAttack, deltaTime);

    /*
    if(isAttack) {
      t3d_anim_update(&animAttack, deltaTime); // attack animation now overrides the idle one
      if(!animAttack.isPlaying)isAttack = false;
    }*/
    // We now blend the walk animation with the idle/attack one
    //t3d_skeleton_blend(&skel, &skel, &skelBlend, animBlend);
    t3d_skeleton_blend(&animations[MODEL_SNAKE].skeleton, &animations[MODEL_SNAKE].skeleton, &animations[MODEL_SNAKE].skeletonBlend, animations[MODEL_SNAKE].blend);

    

    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

    // Now recalc. the matrices, this will cause any model referencing them to use the new pose
    //t3d_skeleton_update(&skel);
    t3d_skeleton_update(&animations[MODEL_SNAKE].skeleton);

    // Update player matrix
    /*
    t3d_mat4fp_from_srt_euler(snakeModelMatFP,
      (float[3]){0.125f, 0.125f, 0.125f},
      (float[3]){0.0f, -rotY, 0},
      playerPos.v
    );*/

    // ======== Draw (3D) ======== //
    
    // This is very expensive
    rdpq_attach(display_get(), display_get_zbuf());

    // Start counting the true render time
    float totalEntityRenderTime = get_time_ms();
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    

    //rspq_block_run(dplMap);
    //rspq_block_run(dplSnake);
    
    syncPoint = rspq_syncpoint_new();
    

    
    // ======== Draw actors (3D) ======== //
    //rdpq_attach(display_get(), display_get_zbuf());
    //t3d_frame_start();
    //t3d_viewport_attach(&viewport);

    //rdpq_set_prim_color(RGBA32(0, 0, 0, 0xFF));

    //t3d_screen_clear_color(RGBA32(100, 120, 160, 0xFF));
    //t3d_screen_clear_depth();

    //t3d_light_set_ambient(colorAmbient);
    //t3d_light_set_directional(0, lightDirColor, &lightDirVec);
    //t3d_light_set_count(1);

    //t3d_matrix_push_pos(1);
    /*
    for(int i=0; i<actorCount; ++i) {
      actor_draw(&actors[i]);
    }*/
    
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        // show debug
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){

            // Update the mesh model matrix based on the entity transform.
            AF_CTransform3D* entityTransform = &_ecs->transforms[i];

            //float modelYRot = entityTransform->rot.y;
            //if(mesh->meshID == MODEL_SNAKE){
            //    modelYRot = -rotY;
            //}
            
            float pos[3] = {entityTransform->pos.x, entityTransform->pos.y, entityTransform->pos.z};
            //float rot[3]= {entityTransform->rot.x, entityTransform->rot.y, entityTransform->rot.z};
            float rot[3]= {entityTransform->rot.x, entityTransform->rot.y, entityTransform->rot.z};
            float scale[3] = {entityTransform->scale.x * MODEL_SCALE_FACTOR, entityTransform->scale.y * MODEL_SCALE_FACTOR, entityTransform->scale.z * MODEL_SCALE_FACTOR};

            //t3d_mat4fp_from_srt_euler((T3DMat4FP*)&mesh->modelMatrix,  scale, rot, pos);
            //t3d_matrix_set((T3DMat4FP*)&mesh->modelMatrix, true);
            /*
            t3d_mat4fp_from_srt_euler(snakeModelMatFP,
                (float[3]){0.125f, 0.125f, 0.125f},
                (float[3]){0.0f, -rotY, 0},
                playerPos.v
                );
            */
            // get the mesh block from the list of meshes that matches the meshID
            //t3d_mat4fp_from_srt_euler(modelsMat[mesh->meshID],  scale, rot, pos);
            //if(modelsMat[i] == NULL){
            //    debugf("AF_Renderer_T3D: AF_RenderUpdate modelsMat %i is null\n",i);
            //    continue;
           // }
            //t3d_mat4fp_from_srt_euler(modelsMat[i],  scale, rot, pos);
            //t3d_mat4fp_from_srt_euler((T3DMat4FP*)mesh->modelMatrix,  scale, rot, pos);
            T3DMat4FP* meshMat = (T3DMat4FP*)mesh->modelMatrix;
            if(meshMat == NULL){
                debugf("AF_Renderer_T3D: AF_RenderUpdate modelsMat %i mesh ID %i mesh type %i is null\n",i, mesh->meshID, mesh->meshType);
                continue;
            }
            t3d_mat4fp_from_srt_euler(meshMat,  scale, rot, pos);
            //rspq_block_run(dpls[mesh->meshID]);
            //rspq_block_run(dpls[mesh->meshID]);
            //rspq_block_run(dpls[i]);
            // Use the display list buffer stored in our mesh data.

            // cast the display buffer back from our void* stored in the mesh component
            rspq_block_run((rspq_block_t*)mesh->displayListBuffer);
            /*
            if(mesh->meshID == MODEL_SNAKE){
                rspq_block_run(dplSnake);
            }else{
                rspq_block_run(dpls[mesh->meshID]);
            }*/
            
        }
    }

    // for each 
    //t3d_matrix_pop(1);
    /**/
    
    // ======== Draw (2D) ======== //
    // ======== Draw (UI) ======== //
    //float posX = 16;
    //float posY = 24;

    rdpq_sync_pipe();
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "[A] Attack: %d", isAttack);

    //posY = 216;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Speed: %.4f", currSpeed); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Blend: %.4f", animBlend); posY += 10;
    totalEntityRenderTime = get_time_ms() - totalEntityRenderTime;
    totalRenderTime = get_time_ms() - totalRenderTime;
    

    //rdpq_sync_pipe();
    int totalTris = 0;
    int totalMeshes = 0;
    for(int i=0; i<_ecs->entitiesCount; ++i) {
      AF_CMesh* mesh = &_ecs->meshes[i];
      if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
        totalMeshes += 1;
        AF_CMesh* mesh = &_ecs->meshes[i];
        totalTris += models[mesh->meshID]->totalVertCount;
      }
        
    }

    

    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 16, 220, "[STICK] Speed : %.2f", baseSpeed);

    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 50, 20, "Entities  : %lu", _ecs->entitiesCount);
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 50, 30, "Meshs  : %i", totalMeshes);
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 50, 40, "Tris  : %i", totalTris);
    
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 50, 50, "Total Render: %.2fms", totalRenderTime);
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 50, 60, "Entity Render: %.2fms", totalEntityRenderTime);
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 50, 70, "FPS   : %.2f", display_get_fps());

    
}



// Mesh rendering switching
void Renderer_RenderMesh(AF_CMesh* _mesh, AF_CTransform3D* _transform, float _dt){
    assert(_mesh != NULL && "AF_Renderer_T3D: Renderer_RenderMesh has null ecs referenced passed in \n");
    //debugf("AF_Renderer_T3D: Renderer_RenderMesh: To be implemented\n");
    // is debug on
    if(_mesh->showDebug == TRUE){
        //render debug
    }
    // Render mesh
    //int isAnimating = 0;
    // Render Shapes
    switch (_mesh->meshType)
    {
    case AF_MESH_TYPE_CUBE:
        /* code */
        
        if(_mesh->isAnimating == TRUE){
            //isAnimating = 1;
        }
        //render_cube(_transform, isAnimating, _dt);
        
        break;
    case AF_MESH_TYPE_PLANE:
        /* code */
        //render_plane(_transform);
    break;

    case AF_MESH_TYPE_SPHERE:
        /* code */
        if(_mesh->isAnimating == TRUE){
           //isAnimating = 1;
        }
        //render_sphere(_transform, isAnimating, _dt);
    break;

    case AF_MESH_TYPE_MESH:
        /* code */
        //render_skinned(_transform, _dt);
    break;
    
    default:
        break;
    }
}

void AF_Renderer_Finish(){
    //debugf("AF_Renderer_T3D: AF_Renderer_Finish: To be implemented\n");
    rdpq_detach_show();
    /*
    // Tell opengl to finish up
    gl_context_end();
    // present the frame
    rdpq_detach_show();

    
    
    // For debugging
    if (DEBUG_RDP){
        rspq_profile_next_frame();

        if (((frames++) % 60) == 0) {
            rspq_profile_dump();
            rspq_profile_reset();
            //debugf("frame %lld\n", frames);
        }
        rspq_wait();
    }
    */
}


// Shutdown Renderer
void AF_Renderer_Shutdown(AF_ECS* _ecs){

    for(int i = 0; i < MODEL_COUNT; ++i){
      t3d_skeleton_destroy(&animations[i].skeleton);
      t3d_skeleton_destroy(&animations[i].skeletonBlend);

      t3d_anim_destroy(&animations[i].animIdle);
      t3d_anim_destroy(&animations[i].animWalk);
      t3d_anim_destroy(&animations[i].animAttack);
    }
    /*
    t3d_skeleton_destroy(&skel);
    t3d_skeleton_destroy(&skelBlend);

    t3d_anim_destroy(&animIdle);
    t3d_anim_destroy(&animWalk);
    t3d_anim_destroy(&animAttack);
    */

   
    for (int i = 0; i < MODEL_COUNT; ++i){
      free(models[i]);
    }
    //t3d_model_free(snakeModel);
    t3d_model_free(modelMap);
    t3d_model_free(modelShadow);

    // free the malloc'd mat4s
    for(int i = 0; i < AF_ECS_TOTAL_ENTITIES; ++i){
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){

            //if(modelsMat[i] == NULL){
            //    debugf("AF_Renderer_T3D: AF_RenderShutdow modelsMat %i is null\n",i);
            //    continue;
           // }
            //free_uncached(modelsMat[i]);
            
            free_uncached(mesh->modelMatrix);
        }
    }

    t3d_destroy();
}

// Chat GPT
// Helper function to clamp values to int16 range
int16_t clamp_to_int16(float value) {
    if (value > INT16_MAX) return INT16_MAX;
    if (value < INT16_MIN) return INT16_MIN;
    return (int16_t)value;
}


