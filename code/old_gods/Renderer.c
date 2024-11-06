/*================
Implementation of AF_Renderer
n64 Libdragon rendering functions


==================*/
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>
#include <rspq_profile.h>

#include "AF_Renderer.h"
#include "AF_UI.h"

#include "Cube.h"
#include "Plane.h"
#include "Sphere.h"
#include "Camera.h"
#include "ECS/Entities/AF_ECS.h"
#include "Skinned.h"

#include "AF_Physics.h"
#define DEBUG_RDP 0



// Global Camera
static camera_t camera;

// Gloab frame counter
static uint64_t frames = 0;

// Global shader settings
//static GLenum shade_model = GL_SMOOTH;
//static bool fog_enabled = false;

static const GLfloat environment_color[] = { 0.2f, 0.2f, 0.2f, 1.f };
// Define the RGBA values for the ambient light (e.g., soft white light)
static const GLfloat ambientLight[] = {0.75f, 0.75f, 0.75f, 1.0f};  // R, G, B, A


// Textures
#define TEXTURE_COUNT 10
static GLuint textures[TEXTURE_COUNT];
static const char *texture_path[TEXTURE_COUNT] = {
    "rom:/old_gods/green.sprite",        // 0 player 1
    "rom:/old_gods/red.sprite",          // 1 player 2
    "rom:/old_gods/orange.sprite",       // 2 player 3
    "rom:/old_gods/purple.sprite",       // 3 player 4
    "rom:/old_gods/grey.sprite",         // 4 god
    "rom:/old_gods/diamond0.sprite",     // 5 bucket
    "rom:/old_gods/triangle0.sprite",    // 6 villages
    "rom:/old_gods/checker.sprite",       // 7 level
    "rom:/old_gods/dark.sprite",         // 8 level
    "rom:/old_gods/god.sprite"           // 9 god

};
static sprite_t *sprites[TEXTURE_COUNT];

// forward declare
void InfrequenceGLEnable(void);
void RenderMesh(AF_CMesh* _mesh, AF_CTransform3D* _transform, float _dt);




uint32_t AF_LoadTexture(const char* _texturePath){
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
}


// Init Rendering
void AF_Renderer_Init(AF_ECS* _ecs){

   	if(_ecs == NULL){
		debugf("Renderer: Renderer_Init has null ecs referenced passed in \n");
		return;
	} 
	debugf("InitRendering\n");
	// Set ??
    dfs_init(DFS_DEFAULT_LOCATION);

    // Display resolution
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);

    // setup the rdpq
    rdpq_init();
    // setup
    gl_init();

#if DEBUG_RDP
    rdpq_debug_start();
    rdpq_debug_log(true);
#endif

    // ======== Camera ==========
    // Setup camera things
    camera.distance = -21.0f;
    camera.rotation = 0.0f;
    float aspect_ratio = (float)display_get_width() / (float)display_get_height();
    float near_plane = 1.0f;
    float far_plane = 50.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho( -10, 10, -10, 10, -10, 10 );
    glFrustum(-near_plane*aspect_ratio, near_plane*aspect_ratio, -near_plane, near_plane, near_plane, far_plane);

    // Retrieve the projection matrix
    // Define the projection matrix manually
    float projectionMatrix[16] = {
        near_plane / (aspect_ratio * near_plane), 0, 0, 0,
        0, near_plane / near_plane, 0, 0,
        0, 0, -(far_plane + near_plane) / (far_plane - near_plane), -1,
        0, 0, -(2 * far_plane * near_plane) / (far_plane - near_plane), 0
    };

    // Now you can use projectionMatrix as needed
    for (int i = 0; i < 16; i++) {
        debugf("Projection Matrix [%d]: %f\n", i, projectionMatrix[i]);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // ==========FOG=============
    // Set other opengl things like fog
    glFogf(GL_FOG_START, 5);
    glFogf(GL_FOG_END, 20);
    glFogfv(GL_FOG_COLOR, environment_color);

    // set ambient light
    // Set the global ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    // Set to true if we want light to take into account the camera view distance, otherwise treat the view as being infinite distance away.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);


    //===========Lighting============
    /*
    float light_radius = 10.0f;

    for (uint32_t i = 0; i < 8; i++)
    {
        glEnable(GL_LIGHT0 + i);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, light_diffuse[i]);
        glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 2.0f/light_radius);
        glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 1.0f/(light_radius*light_radius));
    }*/

   // ===========Materials==================
   // Setup materials 
    GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

    

    // Textures
    for (uint32_t i = 0; i < TEXTURE_COUNT; i++)
    {
        sprites[i] = sprite_load(texture_path[i]);
    }
    

    // ===========Primatives and Mesh==============
    // Cube
    setup_cube();

    // Plane
    setup_plane();
    make_plane_mesh();

    // Sphere
    setup_sphere();
    make_sphere_mesh();


    // Setup skeleton
    setup_skeleton();
    setup_animation();

    // =========Textures==================
    glEnable(GL_MULTISAMPLE_ARB);

    glGenTextures(TEXTURE_COUNT, textures);

    #if 0
    GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR;
    #else
        GLenum min_filter = GL_LINEAR;
    #endif
    
    for (uint32_t i = 0; i < TEXTURE_COUNT; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

        glSpriteTextureN64(GL_TEXTURE_2D, sprites[i], &(rdpq_texparms_t){.s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE});
    }/**/

    


    // ===== basic Texture ====

    
    sprite_t* textureData;
    textureData = sprite_load("rom:/old_gods/circle0.sprite");
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

    







    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        debugf("OpenGL Error after glGenTextures: %u\n", (unsigned int)error);
    }

    // Default texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Bind texture to textureData and set texture parameters
    glSpriteTextureN64(GL_TEXTURE_2D, textureData, &(rdpq_texparms_t){.s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE});

   

    for(int i = 0; i < _ecs->entitiesCount; ++i){
        AF_CMesh* mesh = &_ecs->meshes[i];
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE)){
            //_ecs->sprites->spriteData = (void*)textureData;
            /*
            if(mesh->material.textureID == 0){
                mesh->material.textureID = textures[0];
            }else if(mesh->material.textureID == 1){
                mesh->material.textureID = textures[1];
            }
            else if(mesh->material.textureID == 2){
                mesh->material.textureID = textures[2];
            }
            else if(mesh->material.textureID == 3){
                mesh->material.textureID = textures[3];
            }*/
            //mesh->material.textureID = 0;//(uint32_t)textureID;
        }
    }

    // ==========Remaining setup=============


    

    rspq_profile_start();
}




// Update Renderer
// TODO: take in an array of entities 
void AF_Renderer_Update(AF_ECS* _ecs, AF_Time* _time){
	if(_ecs == NULL){
		debugf("Renderer: Renderer_Update has null ecs referenced passed in \n");
		return;
	} 
    // Get the display and z buffer 
    surface_t *disp = display_get();
    surface_t *zbuf = display_get_zbuf();
    // attatch the buffer to the gpu
    rdpq_attach(disp, zbuf);

    // Begin opengl things
    gl_context_begin();

    // clear the background
    glClearColor(environment_color[0], environment_color[1], environment_color[2], environment_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the matrix mode
    glMatrixMode(GL_MODELVIEW);
   
    // attatch the camera transform
    camera_transform(&camera);

    // Enable opengl things
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    rdpq_set_mode_standard();
    rdpq_mode_filter(FILTER_BILINEAR);
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


    // loop through the objects to render
    // TODO: on the CPU, compbine all similar meshes with the same material and render in less draw calls
    glEnable(GL_TEXTURE_2D);
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        // show debug
        AF_CMesh* mesh = &_ecs->meshes[i];
        
        if((AF_Component_GetHas(mesh->enabled) == TRUE) && (AF_Component_GetEnabled(mesh->enabled) == TRUE)){
            // load the texture
            //if(mesh->material.textureID != 0){
                //debugf("renderer pick textureid %lu ID: %lu \n", mesh->material.textureID, textures[mesh->material.textureID]);
                glBindTexture(GL_TEXTURE_2D, textures[mesh->material.textureID]);
            //}
            
            RenderMesh(mesh, &_ecs->transforms[i], _time->currentFrame);
             AF_CCollider* collider = &_ecs->colliders[i];
            if(collider->showDebug == TRUE){
               
                float collisionColor[4] = {255,0, 0, 1};
                AF_Physics_DrawBox(collider, collisionColor);	
            }
        }

        // Render UI
        // render sprites first
        AF_UI_RendererSprite_Update(&_ecs->sprites[i], _time);
        // render text
        AF_UI_RendererText_Update(&_ecs->texts[i]);
    }
    //debugf("RenderCube: x: %f y: %f z: %f\n", _ecs->transforms[2].pos.x, _ecs->transforms[2].pos.y, _ecs->transforms[2].pos.z);
    //glBindTexture(GL_TEXTURE_2D, textures[(texture_index + 1)%4]);
    //disable the lighting
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    // Draw a primitive with GL_RDPQ_TEXTURING_N64
    //glEnable(GL_RDPQ_TEXTURING_N64);
    //glEnable(GL_RDPQ_MATERIAL_N64);

    // When rendering with GL_RDPQ_TEXTURING_N64 we need to manualy specify the
    // tile size and if a 0.5 offset should be used since the ucode itself cannot
    // determine these. Here we set the tile size to be 32x32 and we apply an offset
    // since we are using bilinear texture filtering
    //glTexSizeN64(32, 32);
    //rdpq_sprite_upload(TILE0, sprites[0], &(rdpq_texparms_t){.s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE});
    //rdpq_set_mode_standard();
    //rdpq_mode_filter(FILTER_BILINEAR);

    // Disable texturing
    //glDisable(GL_RDPQ_TEXTURING_N64);
    //glDisable(GL_RDPQ_MATERIAL_N64);
}



// Mesh rendering switching
void RenderMesh(AF_CMesh* _mesh, AF_CTransform3D* _transform, float _dt){
    // is debug on
    if(_mesh->showDebug == TRUE){
        //render debug
    }
    // Render mesh
    int isAnimating = 0;
    // Render Shapes
    switch (_mesh->meshType)
    {
    case AF_MESH_TYPE_CUBE:
        /* code */
        
        if(_mesh->isAnimating == TRUE){
            isAnimating = 1;
        }
        render_cube(_transform, isAnimating, _dt);
        
        break;
    case AF_MESH_TYPE_PLANE:
        /* code */
        render_plane(_transform);
    break;

    case AF_MESH_TYPE_SPHERE:
        /* code */
        if(_mesh->isAnimating == TRUE){
            isAnimating = 1;
        }
        render_sphere(_transform, isAnimating, _dt);
    break;

    case AF_MESH_TYPE_MESH:
        /* code */
        render_skinned(_transform, _dt);
    break;
    
    default:
        break;
    }
}

void AF_Renderer_Finish(){
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
    
    
}


// Shutdown Renderer
void AF_Renderer_Shutdown(void){

}