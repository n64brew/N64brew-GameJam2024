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
#define RAD_360 6.28318530718f
#define MODEL_SCALE_FACTOR 1.0f
static T3DViewport viewport;

//static T3DVec3 rotAxis;
static uint8_t colorAmbient[4] = {80, 50, 50, 0xFF};
static uint8_t colorDir[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static T3DVec3 lightDirVec = {{1.0f, 1.0f, 0.0f}};

static T3DVec3 camPos = {{0, 45.0f, 10.0f}};
static T3DVec3 camTarget = {{0, 0,-10}};

// TODO: i dont like this
static T3DModel *models[MODEL_COUNT];
T3DAnim animIdles[AF_ECS_TOTAL_ENTITIES];
T3DAnim animWalks[AF_ECS_TOTAL_ENTITIES];
T3DAnim animAttacks[AF_ECS_TOTAL_ENTITIES];



// ============ ANIMATIONS ============
const char* snakeIdlePath = "Snake_Idle";
const char* snakeWalkPath = "Snake_Walk";
const char* snakeAttackPath = "Snake_Attack";

// we need a seperate skelton anim for each skeleton
T3DSkeleton skeletons[AF_ECS_TOTAL_ENTITIES];
T3DSkeleton skeletonBlends[AF_ECS_TOTAL_ENTITIES];
//AF_Animation animations[AF_ECS_TOTAL_ENTITIES];



// Holds our actor data, relevant for t3d is 'modelMat'.
// add a void* called commandBuff to mesh component
// add 4x4 matrix to mesh component

float get_time_s()  { return (float)((double)get_ticks_ms() / 1000.0); }
float get_time_ms() { return (float)((double)get_ticks_us() / 1000.0); }
void AF_Renderer_LoadAnimation(AF_CSkeletalAnimation* _animation, int _i);

// Animation stuff
// TODO: move to component
static T3DModel *modelMap;
static T3DModel *modelShadow;
float lastTime;


rspq_syncpoint_t syncPoint;
float newTime;
float deltaTime;

// Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
static T3DModel *snakeModel;// = t3d_model_load("rom:/snake.t3dm");



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
void Renderer_UpdateAnimations(AF_CSkeletalAnimation* _animation, float _dt);

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
void AF_Renderer_LoadAnimation(AF_CSkeletalAnimation* _animation, int _i){  

    // return if model doesn't have a skeleton
    // TODO: fix this
   
    //assert(_animation->animIdlePath != NULL);
    //assert(_animation->animWalkPath != NULL);
    //assert(_animation->animAttackPath != NULL);

   // First instantiate skeletons, they will be used to draw models in a specific pose
   // model skeleton)
    debugf("AF_Renderer_LoadAnimation: create skeleton \n");
    assert(_animation->model != NULL && "AF_Renderer_LoadAnimation: failed to create skeleton\n");
    skeletons[_i] = t3d_skeleton_create( (T3DModel*)_animation->model);
    _animation->skeleton = (void*)&skeletons[_i];

    debugf("AF_Renderer_LoadAnimation: clone skeleton \n");
    // Model skeletonblend
    skeletonBlends[_i] = t3d_skeleton_clone(&skeletons[_i], false); // optimized for blending, has no matrices
    _animation->skeletonBlend = (void*)&skeletonBlends[_i];
    // Now create animation instances (by name), the data in 'model' is fixed,
    // whereas 'anim' contains all the runtime data.
    // Note that tiny3d internally keeps no track of animations, it's up to the user to manage and play them.
    // create idle animation   

    debugf("AF_Renderer_LoadAnimation: create idle \n");
    animIdles[_i] = t3d_anim_create((T3DModel*)_animation->model, snakeIdlePath);// "Snake_Idle");
    _animation->idleAnimationData = (void*)&animIdles[_i];
    // attatch idle animation
    t3d_anim_attach(&animIdles[_i], &skeletons[_i]); // tells the animation which skeleton to modify

    // Create walk animation
    animWalks[_i] = t3d_anim_create((T3DModel*)_animation->model, snakeWalkPath);//"Snake_Walk");
    _animation->walkAnimationData = (void*)&animWalks[_i];
    // attatch walk animation
    t3d_anim_attach(&animWalks[_i], &skeletonBlends[_i]);

    // multiple animations can attach to the same skeleton, this will NOT perform any blending
    // rather the last animation that updates "wins", this can be useful if multiple animations touch different bones
    // Create attack animation
    animAttacks[_i] = t3d_anim_create((T3DModel*)_animation->model, snakeAttackPath);// "Snake_Attack");
    _animation->attackAnimationData = (void*)&animAttacks[_i];

    // attatch attack animation
    t3d_anim_attach(&animAttacks[_i], &skeletons[_i]);

    // setup attack animation
    t3d_anim_set_looping(&animAttacks[_i], false); // don't loop this animation
    t3d_anim_set_playing(&animAttacks[_i], false); // start in a paused state
    // model blend
}

// Init Rendering
void AF_Renderer_Init(AF_ECS* _ecs){
    assert(_ecs != NULL && "AF_Renderer_T3D: Renderer_Init has null ecs referenced passed in \n");
    debugf("AF_Renderer_T3D: AF_Renderer_Init: To be implemented\n");
   	
	debugf("InitRendering\n");

    // Tindy 3D Init stuff
    t3d_init((T3DInitParams){}); // Init library itself, use empty params for default settings
   
   
    // bulk load an instance of each model type only once.
    for(int i = 0; i < MODEL_COUNT; ++i){
         models[i] = t3d_model_load(model_paths[i]);
         // scale the model
         // TODO read teh model scale from a variable in the mesh
         // load animations
    }

    lastTime = get_time_s() - (1.0f / 60.0f);
    syncPoint = 0;
    
  
    // if mesh has animations
    //AF_Renderer_LoadAnimation(//MODEL_SNAKE);
    //debugf("AF_Renderer_T3d: Renderer_Init: loading animat again.\n");
    //AF_Renderer_LoadAnimation(MODEL_SNAKE);
    

    // ===========Animation models
    modelMap = models[MODEL_MAP];
    modelShadow = models[MODEL_SHADOW];

    // Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
    snakeModel = models[MODEL_SNAKE];

    t3d_vec3_norm(&lightDirVec);

    // TODO: delete all this
    // Allocate vertices (make sure to have an uncached pointer before passing it to the API!)
    // For performance reasons, 'T3DVertPacked' contains two vertices at once in one struct.

    // create a viewport, this defines the section to draw to (by default the whole screen)
    // and contains the projection & view (camera) matrices
    viewport = t3d_viewport_create();


    // Setup lights
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);
    //dplDraw = NULL;


    
}

// rendering stuff that needs to happen after game start or awake
void AF_Renderer_LateStart(AF_ECS* _ecs){

    // ========= Animation stuff
    

    // TODO: render this based off the entities with models.
    // Render a model based on the model loaded by referencing its ID found in the mesh->modelID
    
    for(int i=0; i<_ecs->entitiesCount; ++i) {
        AF_CMesh* mesh = &_ecs->meshes[i];
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            
            // ========== ANIMATIONS =========
            // Process objects that have skeletal animations
            AF_CSkeletalAnimation* animation = &_ecs->skeletalAnimations[i];
            BOOL hasComponent = AF_Component_GetHas(animation->enabled);
            BOOL isEnabled = AF_Component_GetEnabled(animation->enabled);
            
            if(hasComponent == TRUE && isEnabled == TRUE){
                animation->model = (void*)models[MODEL_SNAKE];
                debugf("AF_Renderer: create skeletal animation %i\n",i);
                AF_Renderer_LoadAnimation(animation, i);
            }

            // ============ MESH ==============
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
            t3d_matrix_push(mesh->modelMatrix);
            
            // TODO: put a flag in that can signal a mesh is skinned
            // The snake model needs special color and call to skinned command
            if(mesh->meshID == MODEL_SNAKE){
                rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                t3d_model_draw_skinned(models[mesh->meshID], &skeletons[i]);//animations[MODEL_SNAKE].skeleton); // as in the last example, draw skinned with the main skeleton
                //rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
                //t3d_model_draw(models[MODEL_SHADOW]);
            }
            
            else{
                rdpq_set_prim_color(RGBA32(0, 0, 0, 0xFF));
                t3d_model_draw(models[mesh->meshID]);
                //rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
                //t3d_model_draw(models[MODEL_SHADOW]);
            }
            t3d_matrix_pop(1);
            // cast to void pointer to store in our mesh data, which knows nothing about T3D
            mesh->displayListBuffer = (void*)rspq_block_end();
        }
    }

}



// Update Renderer
// TODO: take in an array of entities 
void AF_Renderer_Update(AF_ECS* _ecs, AF_Time* _time){
	assert(_ecs != NULL && "AF_Renderer_T3D: AF_Renderer_Update has null ecs referenced passed in \n");
  
    float newTime = get_time_s();
    float totalRenderTime = get_time_ms();

    // ======= Animation stuff =======
    // ======== Update ======== //
    //joypad_poll();

    newTime = get_time_s();
    deltaTime = newTime - lastTime;
    lastTime = newTime;


    // position the camera behind the player
    T3DVec3 vec3Zero = {{0.0f,0.0f,0.0f}};
    camTarget = vec3Zero;//playerPos;
    float zDist = 2.5;
    camTarget.v[2] += zDist;//40;
    camPos.v[0] = camTarget.v[0];
    camPos.v[1] = camTarget.v[1] +7;// + 45;
    camPos.v[2] = camTarget.v[2] + 5;//65;

    //
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(45.0f), 1.0f, 1000.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});


    // Update player matrix
    // ======== Draw (3D) ======== //
    
    // This is very expensive
    rdpq_attach(display_get(), display_get_zbuf());

    // Start counting the true render time
    float totalEntityRenderTime = get_time_ms();
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    syncPoint = rspq_syncpoint_new();
    
    // ======== Draw actors (3D) ======== //
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        // show debug
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            
            // ======== ANIMATION ========
            AF_CSkeletalAnimation* animation = &_ecs->skeletalAnimations[i];
            assert(animation != NULL);

            // update animation speed based on the movement velocity
            animation->animationSpeed = Vec3_MAGNITUDE(_ecs->rigidbodies[i].velocity);
            Renderer_UpdateAnimations(animation, _time->timeSinceLastFrame);


            // ======== MODELS ========
            // Update the mesh model matrix based on the entity transform.
            AF_CTransform3D* entityTransform = &_ecs->transforms[i];
            float pos[3] = {entityTransform->pos.x, entityTransform->pos.y, entityTransform->pos.z};
            float rot[3]= {entityTransform->rot.x, entityTransform->rot.y, entityTransform->rot.z};
            float scale[3] = {entityTransform->scale.x * MODEL_SCALE_FACTOR, entityTransform->scale.y * MODEL_SCALE_FACTOR, entityTransform->scale.z * MODEL_SCALE_FACTOR};

            
            T3DMat4FP* meshMat = (T3DMat4FP*)mesh->modelMatrix;
            if(meshMat == NULL){
                debugf("AF_Renderer_T3D: AF_RenderUpdate modelsMat %i mesh ID %i mesh type %i is null\n",i, mesh->meshID, mesh->meshType);
                continue;
            }
            t3d_mat4fp_from_srt_euler(meshMat,  scale, rot, pos);
            // Use the display list buffer stored in our mesh data.

            // cast the display buffer back from our void* stored in the mesh component
            rspq_block_run((rspq_block_t*)mesh->displayListBuffer);
            
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

void Renderer_UpdateAnimations(AF_CSkeletalAnimation* _animation, float _dt){
    if(AF_Component_GetHas(_animation->enabled) == FALSE){
      return;
    }
    // ========== ANIM BLEND =========

    // if mesh has animation
    // update all the anim datas
    // idle
    // walk
    // attack
    // animIsPlaying
    //animBlend = currSpeed / 0.51f;
    _animation->animationBlend = _animation->animationSpeed / 0.51f;
    if(_animation->animationBlend  > 1.0f){
      _animation->animationBlend = 1.0f;
    }
    T3DAnim* animAttackData = (T3DAnim*)_animation->attackAnimationData;
    
     // Update the animation and modify the skeleton, this will however NOT recalculate the matrices
    T3DAnim* animIdleData = (T3DAnim*)_animation->idleAnimationData;
    t3d_anim_update(animIdleData, _dt);
    
    T3DAnim* animWalkData = (T3DAnim*)_animation->walkAnimationData;
    t3d_anim_set_speed(animWalkData, _animation->animationBlend + 0.15f);
    t3d_anim_update(animWalkData, _dt);
    
    //t3d_anim_update(&animWalk, deltaTime);
    //if attacking
    if(animAttackData->isPlaying){
        //debugf("Update animation %f \n", _dt);
        t3d_anim_update(animAttackData, _dt);
        //animAttackData->isPlaying = false;
    }

    // We now blend the walk animation with the idle/attack one
    T3DSkeleton* skeleton = (T3DSkeleton*)_animation->skeleton;
    T3DSkeleton* skeletonBlend = (T3DSkeleton*)_animation->skeletonBlend;

    t3d_skeleton_blend(skeleton, skeleton, skeletonBlend, _animation->animationBlend);

    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

    // Now recalc. the matrices, this will cause any model referencing them to use the new pose
    t3d_skeleton_update(skeleton);
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
    rdpq_detach_show();
    /*
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
        // TODO destroy the skeletal animation
        /*
      t3d_skeleton_destroy(animations[i].skeleton);
      t3d_skeleton_destroy(animations[i].skeletonBlend);

      t3d_anim_destroy(animations[i].idleAnimationData);
      t3d_anim_destroy(animations[i].idleAnimationData);
      t3d_anim_destroy(animations[i].idleAnimationData);
      */
    }
    
   
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

void AF_Renderer_PlayAnimation(AF_CSkeletalAnimation* _animation){
    //debugf("AF_Renderer_PlayAnimation: 1\n");
    assert(_animation != NULL);
  // if the current animation is set to attack and we are not already playing it

    // this is comming back as null
    T3DAnim* animAttackData = (T3DAnim*)_animation->attackAnimationData;
    //debugf("AF_Renderer_PlayAnimation: state %s\n", animAttackData->isPlaying ? "true" : "false");//animAttacks[MODEL_SNAKE].isPlaying ? "true" : "false");
    // Don't progress if animation data isn't setup
    if(animAttackData == NULL){
      //return;
    }

    if(animAttackData->isPlaying == false) {//animAttacks[MODEL_SNAKE].isPlaying == false) {
      
      //debugf("AF_Renderer_PlayAnimation: 1\n");
      //t3d_anim_set_playing(animAttackData, true);
      //t3d_anim_set_time(animAttackData, 0.0f);
      //t3d_anim_set_playing(animAttackData,true);//&animAttacks[MODEL_SNAKE], true);
      //t3d_anim_set_time(animAttackData, 0.0f);//&animAttacks[MODEL_SNAKE], 0.0f);
      //animAttackData->isPlaying = true;
      //animAttacks[MODEL_SNAKE].isPlaying = true;
      
      t3d_anim_set_playing(animAttackData, true);
      t3d_anim_set_time(animAttackData, 0.0f);
      animAttackData->isPlaying = true;
      //player->isAttack = true;
      //player->attackTimer = 0;
    }
  /*
  // play animation
  AF_Animation* animation = &_entity->animation;

  // Move towards the direction of the target
  float dist, norm;
  newDir.v[0] = (target->playerPos.v[0] - player->playerPos.v[0]);
  newDir.v[2] = (target->playerPos.v[2] - player->playerPos.v[2]);
  dist = sqrtf(newDir.v[0]*newDir.v[0] + newDir.v[2]*newDir.v[2]);
  norm = 1/dist;
  newDir.v[0] *= norm;
  newDir.v[2] *= norm;
  speed = 20;

  // Attack if close, and the reaction time has elapsed
  if (dist < 25 && !player->isAttack) {
    if (player->ai_reactionspeed <= 0) {
      t3d_anim_set_playing(&player->animAttack, true);
      t3d_anim_set_time(&player->animAttack, 0.0f);
      player->isAttack = true;
      player->attackTimer = 0;
      player->ai_reactionspeed = (2-core_get_aidifficulty())*5 + rand()%((3-core_get_aidifficulty())*3);
    } else {
      player->ai_reactionspeed--;
    }
  }
  */
}


