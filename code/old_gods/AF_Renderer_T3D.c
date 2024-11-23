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
#define DEBUG_CAM_ON 1
#define DEBUG_CAM_OFF 0
uint8_t debugCam = DEBUG_CAM_OFF;


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

static T3DVec3 camPos = {{0, 14.0f, 10.0f}};
static T3DVec3 camTarget = {{0, 0,1.0}};
static float fov = 45.0f; // Initial field of view

// TODO: i dont like this
static T3DModel *models[MODEL_COUNT];
T3DAnim animIdles[AF_ECS_TOTAL_ENTITIES];
T3DAnim animWalks[AF_ECS_TOTAL_ENTITIES];
T3DAnim animAttacks[AF_ECS_TOTAL_ENTITIES];



// ============ ANIMATIONS ============
const char* idlePath = "Idle";//"Snake_Idle";
const char* walkPath = "Walk";//"Snake_Walk";
const char* attackPath = "Attack";//"Snake_Attack";

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
//static T3DModel *modelMap;
//static T3DModel *modelShadow;
float lastTime;


rspq_syncpoint_t syncPoint;
rspq_block_t* bufferList;

typedef struct RendererDebugData {
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 16, 220, "[STICK] Speed : %.2f", baseSpeed);
    uint16_t entitiesCount;
    uint16_t totalMeshes;
    uint16_t totalTris;
    float totalRenderTime;
    float totalEntityRenderTime;
} RendererDebugData;

RendererDebugData rendererDebugData;

float newTime;
float deltaTime;

// Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
//static T3DModel *snakeModel;// = t3d_model_load("rom:/snake.t3dm");



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
void Renderer_DebugCam();
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
    assert(_animation->model != NULL && "AF_Renderer_LoadAnimation: failed to create skeleton\n");
    skeletons[_i] = t3d_skeleton_create( (T3DModel*)_animation->model);
    _animation->skeleton = (void*)&skeletons[_i];

    // Model skeletonblend
    skeletonBlends[_i] = t3d_skeleton_clone(&skeletons[_i], false); // optimized for blending, has no matrices
    _animation->skeletonBlend = (void*)&skeletonBlends[_i];
    // Now create animation instances (by name), the data in 'model' is fixed,
    // whereas 'anim' contains all the runtime data.
    // Note that tiny3d internally keeps no track of animations, it's up to the user to manage and play them.
    // create idle animation   

    animIdles[_i] = t3d_anim_create((T3DModel*)_animation->model, idlePath);// "Snake_Idle");
    _animation->idleAnimationData = (void*)&animIdles[_i];
    // attatch idle animation
    t3d_anim_attach(&animIdles[_i], &skeletons[_i]); // tells the animation which skeleton to modify

    // Create walk animation
    animWalks[_i] = t3d_anim_create((T3DModel*)_animation->model, walkPath);//"Snake_Walk");
    _animation->walkAnimationData = (void*)&animWalks[_i];
    // attatch walk animation
    t3d_anim_attach(&animWalks[_i], &skeletonBlends[_i]);

    // multiple animations can attach to the same skeleton, this will NOT perform any blending
    // rather the last animation that updates "wins", this can be useful if multiple animations touch different bones
    // Create attack animation
    animAttacks[_i] = t3d_anim_create((T3DModel*)_animation->model, attackPath);// "Snake_Attack");
    _animation->attackAnimationData = (void*)&animAttacks[_i];

    // attatch attack animation
    t3d_anim_attach(&animAttacks[_i], &skeletons[_i]);

    // setup attack animation
    t3d_anim_set_looping(&animAttacks[_i], false); // don't loop this animation
    t3d_anim_set_playing(&animAttacks[_i], false); // start in a paused state
    // model blend
}

// Init Rendering
void AF_Renderer_Init(AF_ECS* _ecs, Vec2 _screenSize){
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
    //modelMap = models[MODEL_MAP];
    //modelShadow = models[MODEL_SHADOW];

    // Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
    //snakeModel = models[MODEL_SNAKE];

    t3d_vec3_norm(&lightDirVec);

    // TODO: delete all this
    // Allocate vertices (make sure to have an uncached pointer before passing it to the API!)
    // For performance reasons, 'T3DVertPacked' contains two vertices at once in one struct.

    // create a viewport, this defines the section to draw to (by default the whole screen)
    // and contains the projection & view (camera) matrices
    viewport = t3d_viewport_create();
    viewport.size[0] = _screenSize.x;
    viewport.size[1] = _screenSize.y;

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
    rspq_block_begin();
    for(int i=0; i<_ecs->entitiesCount; ++i) {
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            
            // ========== ANIMATIONS =========
            // Process objects that have skeletal animations
            AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
            BOOL hasSkeletalComponent = AF_Component_GetHas(skeletalAnimation->enabled);
            BOOL isEnabled = AF_Component_GetEnabled(skeletalAnimation->enabled);
            
            if(hasSkeletalComponent == TRUE && isEnabled == TRUE){
                skeletalAnimation->model = (void*)models[mesh->meshID];
                AF_Renderer_LoadAnimation(skeletalAnimation, i);
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
            //rspq_block_begin();
           
            t3d_matrix_push(mesh->modelMatrix);
             
            // TODO: put a flag in that can signal a mesh is skinned
            // The snake model needs special color and call to skinned command
            
            if(hasSkeletalComponent == TRUE){
                color_t color ={mesh->material.color.r, mesh->material.color.g, mesh->material.color.b, mesh->material.color.a};
                rdpq_set_prim_color(color); //RGBA32(255, 255, 255, 255));
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
            
            //mesh->displayListBuffer = (void*)rspq_block_end();
        }
    }
    bufferList = rspq_block_end();

}



// Update Renderer
// TODO: take in an array of entities 
void AF_Renderer_Update(AF_ECS* _ecs, AF_Time* _time){
	assert(_ecs != NULL && "AF_Renderer_T3D: AF_Renderer_Update has null ecs referenced passed in \n");
  
    float newTime = get_time_s();
    rendererDebugData.totalRenderTime = get_time_ms();

    // ======= Animation stuff =======
    // ======== Update ======== //
    //joypad_poll();

    newTime = get_time_s();
    deltaTime = newTime - lastTime;
    lastTime = newTime;

    
    /*
    // position the camera behind the player
    T3DVec3 vec3Zero = {{0.0f,0.0f,0.0f}};
    camTarget = vec3Zero;//playerPos;
    float zDist = 2.5;
    camTarget.v[2] += zDist;//40;
    camPos.v[0] = camTarget.v[0];
    camPos.v[1] = camTarget.v[1] +8;// + 45;
    camPos.v[2] = camTarget.v[2] + 8;//65;

    //
    joypad_inputs_t inputs = joypad_get_inputs(JOYPAD_PORT_2);
	joypad_buttons_t pressed1 = joypad_get_buttons_pressed(JOYPAD_PORT_2);

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(60.0f), 1.0f, 1000.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
    */
    // Initialize the camera position, target, and FOV
    //T3DVec3 camTarget = {{0.0f, 0.0f, 0.0f}};
    //T3DVec3 camPos = {{0.0f, 8.0f, 10.5f}};
    

    
    //camTarget.v[2] += zDist;//40;
    //camTarget.v[0] = playerPos.v[0];
    //camTarget.v[1] = playerPos.v[1];
    //camTarget.v[2] = playerPos.v[2];

    // Set the viewport with the updated FOV and camera position
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(fov), 1.0f, 1000.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});


    // ======== Update Animations, and collect data about the mesh ======== //
    rendererDebugData.totalTris = 0;
    rendererDebugData.totalMeshes = 0;
    for(int i = 0; i < _ecs->entitiesCount; ++i){
       
        // show debug
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            // update the total meshes and tris
            rendererDebugData.totalMeshes += 1;
            rendererDebugData.totalTris += models[mesh->meshID]->totalVertCount;
            // ======== ANIMATION ========
            AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
            assert(skeletalAnimation != NULL);
            
            if(AF_Component_GetHas(skeletalAnimation->enabled) == TRUE){
                // update animation speed based on the movement velocity
                skeletalAnimation->animationSpeed = Vec3_MAGNITUDE(_ecs->rigidbodies[i].velocity);
                
                Renderer_UpdateAnimations( skeletalAnimation, _time->timeSinceLastFrame);
            }
   
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
        
        }
    }
    
    // do this part seperate. as its expensive
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
            assert(skeletalAnimation != NULL);
            
            if(AF_Component_GetHas(skeletalAnimation->enabled) == TRUE){
                t3d_skeleton_update(skeletalAnimation->skeleton);
            }
    }
    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
    
    // ======== Draw (3D) ======== //
    
    // This is very expensive
    rdpq_attach(display_get(), display_get_zbuf());

    // Start counting the true render time
    rendererDebugData.totalEntityRenderTime = get_time_ms();
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    // Tell the RSP to draw our command list
    rspq_block_run(bufferList);

    // sync the RSP
    syncPoint = rspq_syncpoint_new();
    
    
    // ======== Draw (2D) ======== //
    // ======== Draw (UI) ======== //
    rdpq_sync_pipe();
    rendererDebugData.totalEntityRenderTime = get_time_ms() - rendererDebugData.totalEntityRenderTime;
    rendererDebugData.totalRenderTime = get_time_ms() - rendererDebugData.totalRenderTime;

    

  
    
    // ======== DEBUG Editor =========
    joypad_buttons_t pressed1 = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    // TODO: Move this to outside renderer
    if (pressed1.start & 1) {
        if(debugCam == DEBUG_CAM_OFF){
            debugCam = DEBUG_CAM_ON;
        }else{
            debugCam = DEBUG_CAM_OFF;
        }
    }

    if(debugCam == DEBUG_CAM_ON){
        Renderer_DebugCam(&rendererDebugData);
    }

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
   
    //_animation->animationBlend = _animation->animationSpeed / 0.51f;
    _animation->animationBlend = _animation->animationSpeed * 1.9607843137254901f;

    if(_animation->animationBlend  > 1.0f){
      _animation->animationBlend = 1.0f;
    }
    // get the anims
    T3DAnim* animAttackData = (T3DAnim*)_animation->attackAnimationData;
    T3DAnim* animIdleData = (T3DAnim*)_animation->idleAnimationData;
    T3DAnim* animWalkData = (T3DAnim*)_animation->walkAnimationData;
    
    // if we are not moving then we can update the idle anim, otherwise skip it
    //if(_animation->animationSpeed < 1.0f){
        // Update the animation and modify the skeleton, this will however NOT recalculate the matrices
        
   // }else{
        
    //}
    
    if(animIdleData->isPlaying == TRUE ){
        t3d_anim_update(animIdleData, _dt);
    }
    
    if(animWalkData->isPlaying == TRUE){
        t3d_anim_set_speed(animWalkData, _animation->animationBlend + 0.15f);
        t3d_anim_update(animWalkData, _dt);
    }
       
     
    //t3d_anim_update(&animWalk, deltaTime);
    // disabled attack anim for now
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
    
    //if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
    
    // Now recalc. the matrices, this will cause any model referencing them to use the new pose
    //t3d_skeleton_update(skeleton);
    /**/
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

    
   
    for (int i = 0; i < MODEL_COUNT; ++i){
      free(models[i]);

      
    }
    // free the malloc'd mat4s
    for(int i = 0; i < AF_ECS_TOTAL_ENTITIES; ++i){
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            free_uncached(mesh->modelMatrix);
        }

        /*
        AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
        if(AF_Component_GetHas(skeletalAnimation->enabled)){
            
            t3d_skeleton_destroy(animations[i].skeleton);
            t3d_skeleton_destroy(animations[i].skeletonBlend);

            t3d_anim_destroy(animations[i].idleAnimationData);
            t3d_anim_destroy(animations[i].idleAnimationData);
            t3d_anim_destroy(animations[i].idleAnimationData);
      */
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


void Renderer_DebugCam(RendererDebugData* _rendererDebugData){
    // Read joypad inputs
    joypad_inputs_t inputs = joypad_get_inputs(JOYPAD_PORT_2);
    joypad_buttons_t pressed1 = joypad_get_buttons_held(JOYPAD_PORT_2);

    // Adjust camera FOV
    
    if (pressed1.c_left & 1) {
        fov += 1.0f; // Increase FOV
        
        if (fov > 120.0f) fov = 120.0f; // Limit max FOV
    }
    if (pressed1.c_right & 1) {
        fov -= 1.0f; // Decrease FOV
        if (fov < 30.0f) fov = 30.0f; // Limit min FOV
    }

    // Move camera position along X, Y, Z axes
    float moveSpeed = 0.1f; // Movement speed
    if (inputs.stick_x > 1) { // Move right
        //camPos.v[0] += moveSpeed;
    }
    if (inputs.stick_x < -1) { // Move left
        //camPos.v[0] -= moveSpeed;
    }
    if (inputs.stick_y > 1) { // Move forward (closer to target)
        camPos.v[2] -= moveSpeed;
    }
    if (inputs.stick_y < -1) { // Move backward (away from target)
        camPos.v[2] += moveSpeed;
    }
    if (pressed1.c_up & 1) { // Move up
        camPos.v[1] += moveSpeed;
    }
    if (pressed1.c_down & 1) { // Move down
        camPos.v[1] -= moveSpeed;
    }

  

    // Update the target position (optional if the camera should always point at a fixed location)
    //camTarget = vec3Zero;//playerPos;
    if (pressed1.l & 1) {
        camTarget.v[2] += 1.0f;
    }
    if (pressed1.r & 1) {
        camTarget.v[2] -= 1.0f;
    }


    rdpq_text_printf(NULL, FONT3_ID, 300, 20, 
    "DEBUG_CAM\n\
    FOV: %f\n\
    CAM_POS: x: %f, y: %f, z: %f\n\
    zDist: %f",
    fov, camPos.v[0], camPos.v[1], camPos.v[2], camTarget.v[2]);

    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 16, 220, "[STICK] Speed : %.2f", baseSpeed);
    rdpq_text_printf(NULL, FONT3_ID, 50, 20, "Entities  : %i", _rendererDebugData->entitiesCount);
    rdpq_text_printf(NULL, FONT3_ID, 50, 30, "Meshs  : %i", _rendererDebugData->totalMeshes);
    rdpq_text_printf(NULL, FONT3_ID, 50, 40, "Tris  : %i", _rendererDebugData->totalTris);
    
    rdpq_text_printf(NULL, FONT3_ID, 50, 50, "Total Render: %.2fms", _rendererDebugData->totalRenderTime);
    rdpq_text_printf(NULL, FONT3_ID, 50, 60, "Entity Render: %.2fms", _rendererDebugData->totalEntityRenderTime);
    rdpq_text_printf(NULL, FONT3_ID, 50, 70, "FPS   : %.2f", display_get_fps());
}