/*================
Implementation of AF_Renderer
Tiny3D rendering functions


==================*/

#include "AF_Renderer.h"
#include "AF_UI.h"

#include "ECS/Entities/AF_ECS.h"
#include "AF_Physics.h"
#include "Assets.h"

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#define DEBUG_RDP 0

/**
 * This shows how you can draw multiple objects (actors) in a scene.
 * Each actor has a scale, rotation, translation (knows as SRT) and a model matrix.
 */
#define ACTOR_COUNT 250
#define RAD_360 6.28318530718f

static float objTimeLast = 0.0f;
static float objTime = 0.0f;
static float baseSpeed = 1.0f;

static float rotAngle = 0.0f;
static T3DViewport viewport;
static T3DVertPacked* vertices;
//static const T3DVec3 camPos = {{0,0,-18}};
//static const T3DVec3 camTarget = {{0,0,0}};
static T3DMat4 modelMat; // matrix for our model, this is a "normal" float matrix
static T3DMat4FP* modelMatFP;// = malloc_uncached(sizeof(T3DMat4FP));
static T3DMat4FP* mapMatFP;//; = malloc_uncached(sizeof(T3DMat4FP));


static T3DVec3 rotAxis;
static uint8_t colorAmbient[4] = {80, 50, 50, 0xFF};
static uint8_t colorDir[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static T3DVec3 lightDirVec = {{1.0f, 1.0f, 0.0f}};
static uint8_t lightDirColor[4] = {120, 120, 120, 0xFF};
//static T3DMat4FP* modelMatFP;
static rspq_block_t *dplDraw;

static T3DVec3 camPos = {{0, 45.0f, 80.0f}};//{{100.0f,25.0f,0}};
static T3DVec3 camTarget = {{0, 0,-10}};//{{0,0,0}};

rspq_block_t *dplSnake;
rspq_block_t *dplMap;

// Holds our actor data, relevant for t3d is 'modelMat'.
typedef struct {
  uint32_t id;
  float pos[3];
  float rot[3];
  float scale[3];

  rspq_block_t *dpl;
  T3DMat4FP *modelMat;
} Actor;

static int actorCount = 25;
static T3DModel *models[2];
static Actor actors[ACTOR_COUNT];
static const int triangleCount[2] = {12, 60}; // used for the debug overlay

float get_time_s()  { return (float)((double)get_ticks_ms() / 1000.0); }
float get_time_ms() { return (float)((double)get_ticks_us() / 1000.0); }


// Animation stuff
static T3DModel *modelMap;// = t3d_model_load("rom:/map.t3dm");
static T3DModel *modelShadow;// = t3d_model_load("rom:/shadow.t3dm");
static T3DSkeleton skel;
static T3DSkeleton skelBlend;
static T3DAnim animIdle;
float lastTime;
T3DAnim animWalk;
T3DAnim animAttack;
bool isAttack;
T3DVec3 moveDir;
T3DVec3 playerPos;
float rotY = 0.0f;
float currSpeed = 0.0f;
float animBlend = 0.0f;
rspq_syncpoint_t syncPoint;
float newTime;
float deltaTime;

// Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
static T3DModel *model;// = t3d_model_load("rom:/snake.t3dm");

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

void actor_update(Actor *actor) {
  actor->pos[0] = 0;

  // set some random position and rotation
  float randRot = (float)fm_fmodf(actor->id * 123.1f, 5.0f);
  float randDist = (float)fm_fmodf(actor->id * 4645.987f, 30.5f) + 10.0f;

  actor->rot[0] = fm_fmodf(randRot + objTime * 1.05f, RAD_360);
  actor->rot[1] = fm_fmodf(randRot + objTime * 1.03f, RAD_360);
  actor->rot[2] = fm_fmodf(randRot + objTime * 1.1f, RAD_360);

  actor->pos[0] = randDist * fm_cosf(objTime * 1.6f + randDist);
  actor->pos[1] = randDist * fm_sinf(objTime * 1.5f + randRot);
  actor->pos[2] = randDist * fm_cosf(objTime * 1.4f + randDist*randRot);

  // t3d lets you directly construct a fixed-point matrix from SRT
  t3d_mat4fp_from_srt_euler(actor->modelMat, actor->scale, actor->rot, actor->pos);
}

void actor_draw(Actor *actor) {
  t3d_matrix_set(actor->modelMat, true);
  rspq_block_run(actor->dpl);
}

void actor_delete(Actor *actor) {
  free_uncached(actor->modelMat);
}

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


// Init Rendering
void AF_Renderer_Init(AF_ECS* _ecs){
    assert(_ecs != NULL && "AF_Renderer_T3D: Renderer_Init has null ecs referenced passed in \n");
    debugf("AF_Renderer_T3D: AF_Renderer_Init: To be implemented\n");
   	
	debugf("InitRendering\n");

    // Tindy 3D Init stuff
    

    t3d_init((T3DInitParams){}); // Init library itself, use empty params for default settings
    
    
    // register some font
    //rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

    modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0, 0, -10});


    // Load a some models
    rspq_block_t *dpls[2];
    T3DModel *models[2] = {
        t3d_model_load("rom:/old_gods/box.t3dm"),
        t3d_model_load("rom:/old_gods/food.t3dm")
    };

    // ===========Animation models
    modelMap = t3d_model_load("rom:/old_gods/map.t3dm");
    modelShadow = t3d_model_load("rom:/old_gods/shadow.t3dm");

    // Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
    model = t3d_model_load("rom:/old_gods/snake.t3dm");
    

    // First instantiate skeletons, they will be used to draw models in a specific pose
  // And serve as the target for animations to modify
  skel = t3d_skeleton_create(model);
  skelBlend = t3d_skeleton_clone(&skel, false); // optimized for blending, has no matrices

  // Now create animation instances (by name), the data in 'model' is fixed,
  // whereas 'anim' contains all the runtime data.
  // Note that tiny3d internally keeps no track of animations, it's up to the user to manage and play them.
  animIdle = t3d_anim_create(model, "Snake_Idle");
  t3d_anim_attach(&animIdle, &skel); // tells the animation which skeleton to modify

  animWalk = t3d_anim_create(model, "Snake_Walk");
  t3d_anim_attach(&animWalk, &skelBlend);

  // multiple animations can attach to the same skeleton, this will NOT perform any blending
  // rather the last animation that updates "wins", this can be useful if multiple animations touch different bones
  animAttack = t3d_anim_create(model, "Snake_Attack");
  t3d_anim_set_looping(&animAttack, false); // don't loop this animation
  t3d_anim_set_playing(&animAttack, false); // start in a paused state
  t3d_anim_attach(&animAttack, &skel);

  rspq_block_begin();
    t3d_matrix_push(modelMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw_skinned(model, &skel); // as in the last example, draw skinned with the main skeleton

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


    // ===========



    for(int i=0; i<2; ++i) {
        rspq_block_begin();
        t3d_model_draw(models[i]);
        dpls[i] = rspq_block_end();
    }

    
    for(int i=0; i<ACTOR_COUNT; ++i) {
        actors[i] = actor_create(i, dpls[i*3 % 2]);
    }

    
    
    t3d_vec3_norm(&lightDirVec);

    // Allocate vertices (make sure to have an uncached pointer before passing it to the API!)
    // For performance reasons, 'T3DVertPacked' contains two vertices at once in one struct.
    vertices = malloc_uncached(sizeof(T3DVertPacked) * 2);

    uint16_t norm = t3d_vert_pack_normal(&(T3DVec3){{ 0, 0, 1}}); // normals are packed in a 5.6.5 format
    vertices[0] = (T3DVertPacked){
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
    };

    rotAngle = 0.0f;

    T3DVec3 newAxis = {{-1.0f, 2.5f, 0.25f}};
    rotAxis = newAxis;
    t3d_vec3_norm(&rotAxis);

    // create a viewport, this defines the section to draw to (by default the whole screen)
    // and contains the projection & view (camera) matrices
    viewport = t3d_viewport_create();

    dplDraw = NULL;


    // ========= Animation stuff
    lastTime = get_time_s() - (1.0f / 60.0f);
    syncPoint = 0;

    T3DVec3 ogMoveDir = {{0,0,0}};
    T3DVec3 ogPlayerPos = {{0,0.15f,0}};
    moveDir = ogMoveDir;
    playerPos = ogPlayerPos;

    rotY = 0.0f;
    currSpeed = 0.0f;
    animBlend = 0.0f;
    isAttack = false;
}




// Update Renderer
// TODO: take in an array of entities 
void AF_Renderer_Update(AF_ECS* _ecs, AF_Time* _time){
	assert(_ecs != NULL && "AF_Renderer_T3D: AF_Renderer_Update has null ecs referenced passed in \n");
    //debugf("AF_Renderer_T3D: AF_Renderer_Init: To be implemented\n");


    float newTime = get_time_s();
    float deltaTime = (newTime - objTimeLast) * baseSpeed;
    objTimeLast = newTime;
    objTime += deltaTime;

    float timeUpdate = get_time_ms();
    for(int i=0; i<actorCount; ++i) {
      actor_update(&actors[i]);
    }
    timeUpdate = get_time_ms() - timeUpdate;

    //t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.0f), 10.0f, 100.0f);
    //t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // ======= Animation stuff =======
    // ======== Update ======== //
    joypad_poll();

    newTime = get_time_s();
    deltaTime = newTime - lastTime;
    lastTime = newTime;

    joypad_inputs_t joypad = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

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

      float newAngle = atan2f(moveDir.v[0], moveDir.v[2]);
      rotY = t3d_lerp_angle(rotY, newAngle, 0.25f);
      currSpeed = t3d_lerp(currSpeed, speed * 0.15f, 0.15f);
    } else {
      currSpeed *= 0.8f;
    }

    // use blend based on speed for smooth transitions
    animBlend = currSpeed / 0.51f;
    if(animBlend > 1.0f)animBlend = 1.0f;

    // move player...
    playerPos.v[0] += moveDir.v[0] * currSpeed;
    playerPos.v[2] += moveDir.v[2] * currSpeed;
    // ...and limit position inside the box
    const float BOX_SIZE = 140.0f;
    if(playerPos.v[0] < -BOX_SIZE)playerPos.v[0] = -BOX_SIZE;
    if(playerPos.v[0] >  BOX_SIZE)playerPos.v[0] =  BOX_SIZE;
    if(playerPos.v[2] < -BOX_SIZE)playerPos.v[2] = -BOX_SIZE;
    if(playerPos.v[2] >  BOX_SIZE)playerPos.v[2] =  BOX_SIZE;

    // position the camera behind the player
    camTarget = playerPos;
    camTarget.v[2] -= 20;
    camPos.v[0] = camTarget.v[0];
    camPos.v[1] = camTarget.v[1] + 45;
    camPos.v[2] = camTarget.v[2] + 65;

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 10.0f, 150.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // Update the animation and modify the skeleton, this will however NOT recalculate the matrices
    t3d_anim_update(&animIdle, deltaTime);
    t3d_anim_set_speed(&animWalk, animBlend + 0.15f);
    t3d_anim_update(&animWalk, deltaTime);

    if(isAttack) {
      t3d_anim_update(&animAttack, deltaTime); // attack animation now overrides the idle one
      if(!animAttack.isPlaying)isAttack = false;
    }

    // We now blend the walk animation with the idle/attack one
    t3d_skeleton_blend(&skel, &skel, &skelBlend, animBlend);

    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

    // Now recalc. the matrices, this will cause any model referencing them to use the new pose
    t3d_skeleton_update(&skel);

    // Update player matrix
    t3d_mat4fp_from_srt_euler(modelMatFP,
      (float[3]){0.125f, 0.125f, 0.125f},
      (float[3]){0.0f, -rotY, 0},
      playerPos.v
    );

    // ======== Draw (3D) ======== //
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    rspq_block_run(dplMap);
    rspq_block_run(dplSnake);

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

    t3d_matrix_push_pos(1);
    for(int i=0; i<actorCount; ++i) {
      actor_draw(&actors[i]);
    }
    t3d_matrix_pop(1);
    /**/




    // ======== Draw (2D) ======== //
    // ======== Draw (UI) ======== //
    float posX = 16;
    float posY = 24;

    rdpq_sync_pipe();
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "[A] Attack: %d", isAttack);

    //posY = 216;
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Speed: %.4f", currSpeed); posY += 10;
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Blend: %.4f", animBlend); posY += 10;

    
    //rdpq_sync_pipe();

    int totalTris = 0;
    for(int i=0; i<actorCount; ++i) {
      totalTris += triangleCount[(i*3) % 2];
    }

    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 16, 180, "    [C] Actors: %d", actorCount);
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 16, 220, "[STICK] Speed : %.2f", baseSpeed);

    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 200, 200, "Tris  : %d", totalTris);
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 200, 210, "Update: %.2fms", timeUpdate);
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 200, 220, "FPS   : %.2f", display_get_fps());

    rdpq_detach_show();
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
void AF_Renderer_Shutdown(void){
    t3d_skeleton_destroy(&skel);
    t3d_skeleton_destroy(&skelBlend);

    t3d_anim_destroy(&animIdle);
    t3d_anim_destroy(&animWalk);
    t3d_anim_destroy(&animAttack);

    t3d_model_free(model);
    t3d_model_free(modelMap);
    t3d_model_free(modelShadow);

    t3d_destroy();
}