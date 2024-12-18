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
#include <t3d/tpx.h>


#define DEBUG_RDP 0
#define DEBUG_CAM_ON 1
#define DEBUG_CAM_OFF 0
uint8_t debugCam = DEBUG_CAM_OFF;





#define RAD_360 6.28318530718f
static T3DViewport viewport;

//static T3DVec3 rotAxis;
static uint8_t colorAmbient[4] = {0xFF, 0xFF, 0xFF, 0xFF};
//static uint8_t colorDir[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static T3DVec3 lightDirVec = {{1.0f, 1.0f, 0.001f}};
// SkyBlue 142,224,240
const uint8_t skyColor[3] = {142,224,240};

static T3DVec3 camPos = {{0, 14.0f, 10.0f}};
static T3DVec3 camTarget = {{0, 0,0.0}};
static float fov = 45.0f; // Initial field of view

// TODO: i dont like this
static T3DModel *models[MODEL_COUNT];
T3DAnim animIdles[AF_ECS_TOTAL_ENTITIES];
T3DAnim animWalks[AF_ECS_TOTAL_ENTITIES];
T3DAnim animAttacks[AF_ECS_TOTAL_ENTITIES];



// ============ ANIMATIONS ============
const char* idlePath = "Idle";
const char* walkPath = "Walk";
const char* attackPath = "Attack";

// we need a seperate skelton anim for each skeleton
T3DSkeleton skeletons[AF_ECS_TOTAL_ENTITIES];
T3DSkeleton skeletonBlends[AF_ECS_TOTAL_ENTITIES];

// ============ PARTICLES ===============
// TODO


// Holds our actor data, relevant for t3d is 'modelMat'.
// add a void* called commandBuff to mesh component
// add 4x4 matrix to mesh component

float get_time_s()  { return (float)((double)get_ticks_ms() / 1000.0); }
float get_time_ms() { return (float)((double)get_ticks_us() / 1000.0); }
void AF_Renderer_LoadAnimation(AF_CSkeletalAnimation* _animation, int _i);
void AF_Renderer_DrawParticles(AF_Entity* entity);
void Tile_Scroll(void* userData, rdpq_texparms_t *tileParams, rdpq_tile_t tile);
// Animation stuff
// TODO: move to component
//static T3DModel *modelMap;
//static T3DModel *modelShadow;
float lastTime;


rspq_syncpoint_t syncPoint;
rspq_block_t* skinnedBufferList;
rspq_block_t* staticBufferList;
//rspq_block_t* scrollingBufferUVList;

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

//float* testScrollValue;


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

/*
====================
AF_LoadTexture
Load texture functions
unused
====================
*/
uint32_t AF_LoadTexture(const char* _texturePath){
    
   //debugf("AF_Renderer_T3D: AF_LoadTexture: To be implemented\n");
   return 0;
}

/*
====================
AF_Renderer_LoadAnimation
// TODO: turn the params into a struct to pass in
====================
*/
void AF_Renderer_LoadAnimation(AF_CSkeletalAnimation* _animation, int _i){  
    // return if model doesn't have a skeleton
    // TODO: fix this

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

    // TODO: for some reason, the animations may not correctly release their memory. Usnure why
    
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

/*
====================
AF_Renderer_Init
// Init Rendering
====================
*/
void AF_Renderer_Init(AF_ECS* _ecs, Vec2 _screenSize){
    //testScrollValue = malloc_uncached(sizeof(float));
    //*testScrollValue = 0;
    assert(_ecs != NULL && "AF_Renderer_T3D: Renderer_Init has null ecs referenced passed in \n");   	
	//debugf("InitRendering\n");

    // Tindy 3D Init stuff
    //t3d_init((T3DInitParams){}); // Init library itself, use empty params for default settings
   
   
    
    // bulk load an instance of each model type only once.
    for(int i = 0; i < MODEL_COUNT; ++i){
         models[i] = t3d_model_load(model_paths[i]);
         // scale the model
         // TODO read teh model scale from a variable in the mesh
         // load animations
    }

    lastTime = get_time_s() - (1.0f / 60.0f);
    syncPoint = 0;

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
    // have to comment this out due to strange bug if playing another 3d game first.
    //t3d_light_set_directional(0, colorDir, &lightDirVec); 
    t3d_light_set_count(1);
    //dplDraw = NULL;
    

    
}

/*
====================
AF_Renderer_LateStart
// rendering stuff that needs to happen after game start or awake
====================
*/
void AF_Renderer_LateStart(AF_ECS* _ecs){

    // ========= Animation stuff ========= 
    // TODO: render this based off the entities with models.
    // Render a model based on the model loaded by referencing its ID found in the mesh->modelID
    int totalSkinnedMeshCommands = 0;
    int totalNormalMeshCommands = 0;
    int totalDrawCommands = 0;

    // Load the skinned meshes, and setup memory
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
                
                //if(ENABLE_ANIMATION == TRUE){
                    AF_Renderer_LoadAnimation(skeletalAnimation, i);
                //}
                
                // ============ MESH ==============
                // Only process the entities that have mesh componets with a mesh
                // initialise modelsMat
                // TODO: i don't like this
                
                T3DMat4FP* meshMat = malloc_uncached(sizeof(T3DMat4FP));
                mesh->modelMatrix = (void*)meshMat;
            }
        }
    }

   // Store Skinned Mesh rspq commands
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
                //skeletalAnimation->model = (void*)models[mesh->meshID];
                
                //AF_Renderer_LoadAnimation(skeletalAnimation, i);
                // ============ MESH ==============
                // Only process the entities that have mesh componets with a mesh
                // initialise modelsMat
                // TODO: i don't like this
                
                //T3DMat4FP* meshMat = malloc_uncached(sizeof(T3DMat4FP));
                //mesh->modelMatrix = (void*)meshMat;

               
                if(hasSkeletalComponent == TRUE){
                    t3d_matrix_push(mesh->modelMatrix);
                    color_t color ={mesh->material.color.r, mesh->material.color.g, mesh->material.color.b, mesh->material.color.a};
                    //rdpq_set_prim_color(color); //RGBA32(255, 255, 255, 255));
                    // draw white / existing vertex color for main mesh
                    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                    //rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                    t3d_model_draw_skinned(models[mesh->meshID], &skeletons[i]);//animations[MODEL_SNAKE].skeleton); // as in the last example, draw skinned with the main skeleton
                    
                    // draw the shadow
                    //rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
                    //t3d_model_draw(models[MODEL_SHADOW]);
                    // draw floaties with player color
                    rdpq_set_prim_color(color);
                    t3d_model_draw(models[MODEL_TORUS]);
                    //rdpq_set_depth_write(TRUE); // Do not overwrite depth
                    totalSkinnedMeshCommands += 1;
                    t3d_matrix_pop(1);
                    totalDrawCommands ++;
                }
                
            }
            
        }
    }

    skinnedBufferList = rspq_block_end();

    // ==== STATIC MESH DRAWING ====
    // Setup the static meshes malloc mesh data
    for(int i=0; i<_ecs->entitiesCount; ++i) {
        AF_CMesh* mesh = &_ecs->meshes[i];
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            // ========== ANIMATIONS =========
            // Process objects that have skeletal animations
            AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
            BOOL hasSkeletalComponent = AF_Component_GetHas(skeletalAnimation->enabled);
            //BOOL isEnabled = AF_Component_GetEnabled(skeletalAnimation->enabled);
        
            if(hasSkeletalComponent == FALSE){
                // ============ MESH ==============
                // Only process the entities that have mesh componets with a mesh
                // initialise modelsMat
                // TODO: i don't like this
                T3DMat4FP* meshMat = malloc_uncached(sizeof(T3DMat4FP));
                mesh->modelMatrix = (void*)meshMat;

            }
        }
    }

    rspq_block_begin();
    for(int i=0; i<_ecs->entitiesCount; ++i) {
        AF_CMesh* mesh = &_ecs->meshes[i];
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            // ========== ANIMATIONS =========
            // Process objects that have skeletal animations
            AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
            BOOL hasSkeletalComponent = AF_Component_GetHas(skeletalAnimation->enabled);
            //BOOL isEnabled = AF_Component_GetEnabled(skeletalAnimation->enabled);
        
            if(hasSkeletalComponent == FALSE){
                // ============ MESH ==============
                // Only process the entities that have mesh componets with a mesh
                // initialise modelsMat
                // TODO: i don't like this
                //T3DMat4FP* meshMat = malloc_uncached(sizeof(T3DMat4FP));
                //mesh->modelMatrix = (void*)meshMat;
               
                //rspq_block_begin();
            
                t3d_matrix_push(mesh->modelMatrix);
                color_t color ={mesh->material.color.r, mesh->material.color.g, mesh->material.color.b, mesh->material.color.a};
                    
                rdpq_set_prim_color(color);//RGBA32(255, 255, 255, 255));
                // Scroll the foam
                
                // TODO: don't be specific to foam
                if(mesh->meshID == MODEL_FOAM || mesh->meshID == MODEL_TRAIL || mesh->meshID == MODEL_ATTACK_WAVE || mesh->meshID == MODEL_SMOKE){

                    // skip drawing foam, as we do it another way.
                }else{
                    t3d_model_draw(models[mesh->meshID]);
                }
                

                totalNormalMeshCommands++;
                t3d_matrix_pop(1);
                totalDrawCommands ++;
            }
                // cast to void pointer to store in our mesh data, which knows nothing about T3D
                //mesh->displayListBuffer = (void*)rspq_block_end();
        }
    }
    staticBufferList = rspq_block_end();
    
}
/*
====================
AF_Renderer_Update
// Update Renderer
// TODO: take in an array of entities 
====================
*/
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


    // Set the viewport with the updated FOV and camera position
    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(fov), 1.0f, 1000.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});

    
    // ======== Update Animations, and collect data about the mesh ======== //
    rendererDebugData.totalTris = 0;
    rendererDebugData.totalMeshes = 0;
    
   
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        // show debug
        AF_CMesh* mesh = &_ecs->meshes[i];
        BOOL hasMesh = AF_Component_GetHas(_ecs->entities[i].mesh->enabled);
        BOOL isEnabled = AF_Component_GetEnabled(_ecs->entities[i].mesh->enabled);
        if((hasMesh == TRUE) && (isEnabled == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            // update the total meshes and tris
            rendererDebugData.totalMeshes += 1;
            rendererDebugData.totalTris += models[mesh->meshID]->totalVertCount;
            
            // ======== ANIMATION ========
            AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
            //assert(skeletalAnimation != NULL);
            if(skeletalAnimation == NULL){
                debugf("AF_Renderer_Update: trying to update a null skeletal animation \n");
                return;
            }

            if(AF_Component_GetHas(skeletalAnimation->enabled) == TRUE){
                // update animation speed based on the movement velocity
                skeletalAnimation->animationSpeed = Vec3_MAGNITUDE(_ecs->rigidbodies[i].velocity);
                
                Renderer_UpdateAnimations( skeletalAnimation, _time->timeSinceLastFrame);
                // this is expensive.
                if(skeletalAnimation->skeleton != NULL){
                    t3d_skeleton_update(skeletalAnimation->skeleton);
                }
                
            }
   
            // ======== MODELS ========
            // Update the mesh model matrix based on the entity transform.
            AF_CTransform3D* entityTransform = &_ecs->transforms[i];
            float pos[3] = {entityTransform->pos.x, entityTransform->pos.y, entityTransform->pos.z};
            float rot[3]= {entityTransform->rot.x, entityTransform->rot.y, entityTransform->rot.z};
            float scale[3] = {entityTransform->scale.x, entityTransform->scale.y, entityTransform->scale.z};

            T3DMat4FP* meshMat = (T3DMat4FP*)mesh->modelMatrix;
            
            if(meshMat == NULL){
                debugf("AF_Renderer_T3D: AF_RenderUpdate modelsMat %i mesh ID %i mesh type %i is null\n",i, mesh->meshID, mesh->meshType);
                continue;
            }
            t3d_mat4fp_from_srt_euler(meshMat,  scale, rot, pos);
        
        }
    }


    
    //scrollingBufferUVList = rspq_block_end();

    // only way to make the foam scroll is force it.
    /*
    for(int i  = 0; i < _ecs->entitiesCount; ++i){
        AF_CMesh* mesh = _ecs->entities[i].mesh;
        AF_CAnimation* animation = _ecs->entities[i].animation;
        BOOL hasMesh = AF_Component_GetHas(_ecs->entities[i].mesh->enabled);
        BOOL isEnabled = AF_Component_GetEnabled(_ecs->entities[i].mesh->enabled);
        BOOL hasAnimation = AF_Component_GetHas(_ecs->entities[i].animation->enabled);
        if(hasMesh == TRUE && hasAnimation == TRUE && isEnabled == TRUE){
            if(mesh->meshID == MODEL_FOAM || mesh->meshID == MODEL_TRAIL || MODEL_ATTACK_WAVE){
               
                // do special drawing for foam.
                T3DMat4FP* meshMat = (T3DMat4FP*)mesh->modelMatrix;
                t3d_matrix_push(meshMat);
                
                // make the trail speed based off the animation component
                float adjustedTileOffset = _time->currentFrame * animation->animationSpeed;
                if(mesh->meshID == MODEL_FOAM || mesh->meshID == MODEL_TRAIL || mesh->meshID == MODEL_ATTACK_WAVE){
                    adjustedTileOffset  = adjustedTileOffset * 2;
                }

                color_t color = {mesh->material.color.r, mesh->material.color.g, mesh->material.color.b, mesh->material.color.a};
                rdpq_set_prim_color(color);
                 
                t3d_model_draw_custom(models[mesh->meshID], (T3DModelDrawConf){
                                .userData = &adjustedTileOffset,
                                //.matrices = meshMat,
                                .tileCb = Tile_Scroll,
                                });
                                
                t3d_matrix_pop(1);
            }
        }
    }*/
     /**/
   
    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
    
    // ======== Draw (3D) ======== //
   
    // This is very expensive
    
    rdpq_attach(display_get(), display_get_zbuf());
    
    // Start counting the true render time
    
    rendererDebugData.totalEntityRenderTime = get_time_ms();
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(skyColor[0], skyColor[1], skyColor[2], 0xFF));
    t3d_screen_clear_depth();
    
    
    
     // Tell the RSP to draw our command list
    rspq_block_run(skinnedBufferList);
    rspq_block_run(staticBufferList);
    // ==== UV Scrolling Drawing ====
    // TODO:
    // I really don't like this
    // stacking up calls to the display list in the setup just didn't show the uv scrolling/callback being called.
    // even though i tested with malloc_uncached values, so had to resort to this slow implementation similar to UV scrolling found in lava example
    //rspq_block_begin();
    
    for(int i=0; i<_ecs->entitiesCount; ++i) {
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        //if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
            AF_CAnimation* animation = _ecs->entities[i].animation;
            BOOL hasMesh = AF_Component_GetHas(_ecs->entities[i].mesh->enabled);
            BOOL isEnabled = AF_Component_GetEnabled(_ecs->entities[i].mesh->enabled);
            BOOL hasAnimation = AF_Component_GetHas(_ecs->entities[i].animation->enabled);
            if(hasMesh == TRUE && hasAnimation == TRUE && isEnabled == TRUE){
            
                if(mesh->meshID == MODEL_FOAM || mesh->meshID == MODEL_TRAIL || MODEL_ATTACK_WAVE){
                    // do special drawing for foam.
                    T3DMat4FP* meshMat = (T3DMat4FP*)mesh->modelMatrix;
                    t3d_matrix_push(meshMat);
                    animation->uvScrollingSpeed  = fm_fmodf((_time->currentFrame * animation->animationSpeed), 32.0f);
                    color_t color = {mesh->material.color.r, mesh->material.color.g, mesh->material.color.b, mesh->material.color.a};
                    rdpq_set_prim_color(color);
                    t3d_model_draw_custom(models[mesh->meshID], (T3DModelDrawConf){
                                    .userData = &animation->uvScrollingSpeed,//adjustedTileOffset,
                                    //.matrices = meshMat,
                                    .tileCb = Tile_Scroll,
                                    });
                                    
                    t3d_matrix_pop(1);
                }
            }
        }
    }
    
    //scrollingBufferUVList = rspq_block_end();

    //rspq_block_run(scrollingBufferUVList);


    // ======== DRAW PARTICLES ========
    //AF_Renderer_DrawParticles(&_ecs->entities[0]);
    
    // sync the RSP
    syncPoint = rspq_syncpoint_new();
    
    
    // ======== Draw (2D) ======== //
    // ======== Draw (UI) ======== //
    rdpq_sync_pipe();
    rendererDebugData.totalEntityRenderTime = get_time_ms() - rendererDebugData.totalEntityRenderTime;
    rendererDebugData.totalRenderTime = get_time_ms() - rendererDebugData.totalRenderTime;
    
    // ======== DEBUG Editor =========
    /*
    joypad_buttons_t pressed1 = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    // TODO: Move this to outside renderer
    if (pressed1.c_up & 1) {
        if(debugCam == DEBUG_CAM_OFF){
            debugCam = DEBUG_CAM_ON;
        }else{
            debugCam = DEBUG_CAM_OFF;
        }
    }

    if(debugCam == DEBUG_CAM_ON ){
        Renderer_DebugCam(&rendererDebugData);
    }
    */
}

/*
====================
Renderer_UpdateAnimations
Update the animations
====================
*/
void Renderer_UpdateAnimations(AF_CSkeletalAnimation* _animation, float _dt){
    if(AF_Component_GetHas(_animation->enabled) == FALSE && AF_Component_GetEnabled(_animation->enabled) == FALSE){
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
    // 1.9607843137254901f; used instead of division
    _animation->animationBlend = _animation->animationSpeed * 1.9607843137254901f;

    if(_animation->animationBlend  > 1.0f){
      _animation->animationBlend = 1.0f;
    }
    // get the anims
    T3DAnim* animAttackData = (T3DAnim*)_animation->attackAnimationData;
    T3DAnim* animIdleData = (T3DAnim*)_animation->idleAnimationData;
    T3DAnim* animWalkData = (T3DAnim*)_animation->walkAnimationData;
    if(animAttackData == NULL || animAttackData == NULL || animWalkData == NULL){
        return;
    }
    
    if(animIdleData->isPlaying == TRUE ){
        t3d_anim_update(animIdleData, _dt);
    }
    
    if(animWalkData->isPlaying == TRUE){
        t3d_anim_set_speed(animWalkData, _animation->animationBlend + 0.15f);
        t3d_anim_update(animWalkData, _dt);
    }
       
    
    // disabled attack anim for now
    //if attacking
    
    if(animAttackData->isPlaying){
        t3d_anim_update(animAttackData, _dt);
    }
    
    
    // We now blend the walk animation with the idle/attack one
    T3DSkeleton* skeleton = (T3DSkeleton*)_animation->skeleton;
    T3DSkeleton* skeletonBlend = (T3DSkeleton*)_animation->skeletonBlend;

    if(skeleton == NULL || skeletonBlend == NULL){
        return;
    }
    t3d_skeleton_blend(skeleton, skeleton, skeletonBlend, _animation->animationBlend);
    
    //if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
    
    // Now recalc. the matrices, this will cause any model referencing them to use the new pose
    //t3d_skeleton_update(skeleton);
    /**/
}


/*
====================
Renderer_RenderMesh
// Mesh rendering switching
Render the mesh
====================
*/
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


/*
====================
AF_Renderer_Shutdown
Do shutdown things
====================
*/
void AF_Renderer_Shutdown(AF_ECS* _ecs){
   //debugf("AF_Renderer_T3D: Shutdown\n");

    // Free the display buffers
    if(skinnedBufferList != NULL && staticBufferList != NULL){
        rspq_block_free(skinnedBufferList);
        rspq_block_free(staticBufferList);
    }else{
        debugf("AF_Renderer_Shutdown: Freeing NULL rspq buffer\n");
    }
    
    
    //rspq_block_free(scrollingBufferUVList);

    
    // free the malloc'd mat4s matrix's
    for(int i = 0; i < AF_ECS_TOTAL_ENTITIES; ++i){
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        //if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && mesh->meshType == AF_MESH_TYPE_MESH){

            //debugf("AF_Renderer_T3D: freeing model matrix's %i\n", i);
            if(mesh->modelMatrix != NULL){
                free_uncached(mesh->modelMatrix);
            }
        }
        
        // Free the skeltons
        AF_CSkeletalAnimation* skeletalAnimation = &_ecs->skeletalAnimations[i];
        BOOL hasSkeletalAnimation = AF_Component_GetHas(skeletalAnimation->enabled);
        if(hasSkeletalAnimation == TRUE){
        //if(AF_Component_GetHas(skeletalAnimation->enabled) == TRUE){
            //debugf("AF_Renderer_T3D: destroying skeltons %i \n", i);
            T3DSkeleton* skelton = (T3DSkeleton*) skeletalAnimation->skeleton;
            T3DSkeleton* skelBlend = (T3DSkeleton*) skeletalAnimation->skeletonBlend;
            T3DAnim* animIdle = (T3DAnim*) skeletalAnimation->idleAnimationData;
            T3DAnim* animAttack = (T3DAnim*) skeletalAnimation->attackAnimationData;
            T3DAnim* animWalk = (T3DAnim*) skeletalAnimation->walkAnimationData;
            /*
            if(skelton == NULL || skelBlend == NULL || animIdle == NULL || animAttack == NULL || animWalk == NULL){
                continue;
            }*/
            // destroy the skeletons
            //if(skelton != NULL && skelBlend != NULL && animIdle != NULL && animAttack != NULL && animWalk != NULL){
                
            t3d_skeleton_destroy(skelton);
            t3d_skeleton_destroy(skelBlend);
            
            // Destroy the animations
            t3d_anim_destroy(animIdle);
                
            t3d_anim_destroy(animWalk);
            
            
            t3d_anim_destroy(animAttack);
        }
    }

    // Free the models
    for (int i = 0; i < MODEL_COUNT; ++i){
      //free(models[i]);
      //debugf("AF_Renderer_T3D: freeing models %i\n", i);
      if(models[i] != NULL){
        t3d_model_free(models[i]);
      }
      
    }
}

// Chat GPT
// Helper function to clamp values to int16 range
int16_t clamp_to_int16(float value) {
    if (value > INT16_MAX) return INT16_MAX;
    if (value < INT16_MIN) return INT16_MIN;
    return (int16_t)value;
}

/*
====================
AF_Renderer_PlayAnimation
Play animations
====================
*/
void AF_Renderer_PlayAnimation(AF_CSkeletalAnimation* _animation){
    //debugf("AF_Renderer_PlayAnimation: 1\n");
    assert(_animation != NULL);
  // if the current animation is set to attack and we are not already playing it

    // this is comming back as null
    T3DAnim* animAttackData = (T3DAnim*)_animation->attackAnimationData;
    //debugf("AF_Renderer_PlayAnimation: state %s\n", animAttackData->isPlaying ? "true" : "false");//animAttacks[MODEL_SNAKE].isPlaying ? "true" : "false");
    // Don't progress if animation data isn't setup
    if(animAttackData == NULL){
      return;
    }

      
      t3d_anim_set_playing(animAttackData, true);
      t3d_anim_set_time(animAttackData, 0.0f);
      
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

    // TODO: set the rdpq font 

    rdpq_text_printf(NULL, FONT2_ID, 300, 20, 
    "DEBUG_CAM\n\
    FOV: %f\n\
    CAM_POS: x: %f, y: %f, z: %f\n\
    zDist: %f",
    fov, camPos.v[0], camPos.v[1], camPos.v[2], camTarget.v[2]);

    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 16, 220, "[STICK] Speed : %.2f", baseSpeed);
    rdpq_text_printf(NULL, FONT2_ID, 50, 20, "Entities  : %i", _rendererDebugData->entitiesCount);
    rdpq_text_printf(NULL, FONT2_ID, 50, 30, "Meshs  : %i", _rendererDebugData->totalMeshes);
    rdpq_text_printf(NULL, FONT2_ID, 50, 40, "Tris  : %i", _rendererDebugData->totalTris);
    
    rdpq_text_printf(NULL, FONT2_ID, 50, 50, "Total Render: %.2fms", _rendererDebugData->totalRenderTime);
    rdpq_text_printf(NULL, FONT2_ID, 50, 60, "Entity Render: %.2fms", _rendererDebugData->totalEntityRenderTime);
    rdpq_text_printf(NULL, FONT2_ID, 50, 70, "FPS   : %.2f", display_get_fps());
}

/*
====================
AF_Renderer_DrawParticles
Draw Particles
Not currently implemented
====================
*/
void AF_Renderer_DrawParticles(AF_Entity* _entity){
    //float partSizeX = 0;
    //float partSizeY = 0;
    
    // ======== Draw (Particles) ======== //

    // Prepare drawing particles.
    // In contrast to t3d which draws triangles, tpx will emit screen-space rectangles.
    // The color of each particle is set as prim. color, and shade is not defined.
    // So we have to set up a few things via rdpq to make that work depending on the desired effect.
    //
    // In our case, we want to combine it with env. color in the CC.
    // In order to have depth, you also need to enable `rdpq_mode_zoverride` so the ucode can set this correctly.
    
    rdpq_sync_pipe();
    
    rdpq_sync_tile();
    rdpq_set_mode_standard();
   
    rdpq_mode_zbuf(true, true);
    rdpq_mode_zoverride(true, 0, 0);
   
    rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,0,ENV,0), (0,0,0,1)));
   
    // tpx is its own ucode, so nothing that was set up in t3d carries over automatically.
    // For convenience, you can call 'tpx_state_from_t3d' which will copy over
    // the current matrix, screen-size and w-normalization factor.
    // Crash caused from this point onwards
    tpx_state_from_t3d();
     
    // tpx also has the same matrix stack functions as t3d, Note that the stack itself is NOT shared
    // so any push/pop here will not affect t3d and vice versa.
    // Also make sure that the first stack operation you do after 'tpx_state_from_t3d' is a push and not a set.
    
    //====tpx_matrix_push(&matPartFP);
    // While each particle has its own size, there is a global scaling factor that can be set.
    // This can only scale particles down, so the range is 0.0 - 1.0.
    //====tpx_state_set_scale(partSizeX, partSizeY);

    // Now draw particles. internally this will load, transform and draw them in one go on the RSP.
    // While the ucode can only handle a 344 at a time, this function will automatically batch them
    // so you can specify an arbitrary amount of particles (as long as it's an even count)
    //tpx_particle_draw(particles, PARTICLE_COUNT);

    // Make sure end up at the same stack level as before.
    //====tpx_matrix_pop(1);
    /**/
    // After all particles are drawn, there is nothing special to do.
    // You can either continue with t3d (remember to revert rdpq settings again) or do other 2D draws.

}

/*
====================
Tile_Scroll
// Hook/callback to modify tile settings set by t3d_model_draw
====================
*/
void Tile_Scroll(void* userData, rdpq_texparms_t *tileParams, rdpq_tile_t tile) {

 
  float offset = *(float*)userData;
   
  if(tile == TILE0) {
    tileParams->s.translate = 0.0f;//offset * 0.5f;
    
    tileParams->t.translate = offset *2.0f;
    
    //tileParams->s.translate = offset * 0.5f;
    
    //tileParams->t.translate = offset * 0.8f;
    
    tileParams->s.translate = fm_fmodf(tileParams->s.translate, 32.0f);
    
    tileParams->t.translate = fm_fmodf(tileParams->t.translate, 32.0f);
    
  }
  /**/
  /**/
  

}