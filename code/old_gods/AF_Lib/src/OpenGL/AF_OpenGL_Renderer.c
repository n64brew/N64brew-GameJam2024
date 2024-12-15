/*
===============================================================================
AF_OpenGL_Renderer Implementation

Implementation of the AF_LIB rendering functions
This implementation is for OpenGL
===============================================================================
*/
#include <stdio.h>
#include "AF_Renderer.h"
#include "AF_Debug.h"
#include "ECS/Components/AF_Component.h"
#include "AF_Log.h"
#include "AF_Vec3.h"
#include "AF_Mat4.h"
#include <GL/glew.h>
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// string to use in logging
const char* openglRendererFileTitle = "AF_OpenGL_Renderer:";

// set up vertex data (and buffer(s)) and configure vertex attributes
// ------------------------------------------------------------------
float vertices2[] = {
    0.5f,  0.5f, 0.0f,  // top right
    0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};

AF_Vertex vertices[4] = {
    { {0.5f,  0.5f, 0.0f}, {0.5f,  0.5f, 0.0f}, {0.5f,  0.5f} },
    { {0.5f, -0.5f, 0.0f}, {0.5f,  0.5f, 0.0f}, {0.5f,  0.5f} },
    { {-0.5f, -0.5f, 0.0f}, {0.5f,  0.5f, 0.0f}, {0.5f,  0.5f} },
    { {-0.5f,  0.5f, 0.0f}, {0.5f,  0.5f, 0.0f}, {0.5f,  0.5f} }
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,  // first Triangle
    1, 2, 3   // second Triangle
};

// OpenGL errors
const char* invalidEnum = "INVALID_ENUM";
const char* invalidValue = "INVALID_VALUE";
const char* invalidOperation = "INVALID_OPERATION";
const char* stackOverflow = "STACK_OVERFLOW";
const char* stackUnderflow = "STACK_UNDERFLOW";
const char* outOfMemory = "OUT_OF_MEMORY";
const char* invalidFrameBufferOperation = "INVALID_FRAMEBUFFER_OPERATION";



/*
====================
AF_CheckGLError
Helper function for checking for GL errors
====================
*/
void AF_CheckGLError(const char* _message){    
    GLenum errorCode = GL_NO_ERROR;
    errorCode = glGetError();
    const char* errorMessage = "";
    if(errorMessage){}
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
	switch (errorCode)
        {
            case GL_INVALID_ENUM:                  errorMessage  = invalidEnum; break;
            case GL_INVALID_VALUE:                 errorMessage  = invalidValue; break;
            case GL_INVALID_OPERATION:             errorMessage  = invalidOperation; break;
            case GL_STACK_OVERFLOW:                errorMessage  = stackOverflow; break;
            case GL_STACK_UNDERFLOW:               errorMessage  = stackUnderflow; break;
            case GL_OUT_OF_MEMORY:                 errorMessage  = outOfMemory; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorMessage  = invalidFrameBufferOperation; break;
        }
    AF_Log_Error(_message,errorMessage);

    }
           //printf("\nGL Error: %i\n", error);
}

/*
====================
AF_Log_Mat4
Take a AF_Mat 4 and log it to the console.
====================
*/
void AF_Log_Mat4(AF_Mat4 _mat4){
	AF_Log("		Row 1: %f %f %f %f\n\
		Row 2: %f %f %f %f\n\
		Row 3: %f %f %f %f\n\
		Row 4: %f %f %f %f\n\n",
		_mat4.rows[0].x, 
		_mat4.rows[0].y,
		_mat4.rows[0].z,
		_mat4.rows[0].w,

		_mat4.rows[1].x,
_mat4.rows[1].y,
		_mat4.rows[1].z,
		_mat4.rows[1].w,

		_mat4.rows[2].x,
		_mat4.rows[2].y,
		_mat4.rows[2].z,
		_mat4.rows[2].w,

		_mat4.rows[3].x,
		_mat4.rows[3].y,
		_mat4.rows[3].z,
		_mat4.rows[3].w);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int AF_Renderer_LoadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);  

    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1){
            format = GL_RED;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
        }
        else if (nrComponents == 4){
            format = GL_RGBA;
        }
            

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Use these filters for normal textures
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR_MIPMAP_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Use these filters for pixel art sprites.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(data);
    }
    else
    {
	AF_Log_Error("Texture failed to load at path %s\n",path);
        stbi_image_free(data);
    }

    return textureID;
}

void AF_Renderer_SetTexture(const unsigned int _shaderID, const char* _shaderVarName, int _textureID){
    glUseProgram(_shaderID); // Bind the shader program
    glUniform1i(glGetUniformLocation(_shaderID, _shaderVarName), _textureID); // Tell the shader to set the "Diffuse_Texture" variable to use texture id 0
    glUseProgram(0);
}

/*
====================
AF_LIB_InitRenderer
Init OpenGL
====================
*/
int AF_LIB_InitRenderer(AF_Window* _window){
    int success = 1;
    AF_Log("%s Initialized %s\n", openglRendererFileTitle, _window->title);
    //Initialize GLEW
    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        AF_Log_Error( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
    }

    //Use Vsync
    /*
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
        AF_Log_Error( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
        success = 0;
    }*/

    //Initialize OpenGL
    
    //Initialize clear color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f );
    
    /**/

    //set the glViewport and the perspective
    //glViewport(_window->windowXPos, _window->windowYPos, _window->windowWidth, _window->windowHeight);

        // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    AF_CheckGLError( "Error initializing OpenGL! \n");
    //AF_CheckGLError("SLDGameRenderer::Initialise:: finishing up init: ");
    return success;
} 

/*
====================
AF_LIB_InitMeshBuffers
Init the mesh buffers for OpenGL
====================
*/
// TODO: change name to AF_Renderer_InitMeshBuffers
void AF_LIB_InitMeshBuffers(AF_Entity* _entities, uint32_t _entityCount){ 

    if (_entityCount == 0) {
    AF_Log_Error("No meshes to draw!\n");
    return;
    }

    for(uint32_t i = 0; i < _entityCount; i++){
	    AF_Mesh* mesh = _entities[i].mesh;

	    
	    BOOL hasMesh = AF_Component_GetHas(mesh->enabled);
	    // Skip setting up if we don't have a mesh component
	    if(hasMesh == FALSE){
		continue;
	    }

	    if (mesh->indexCount == 0) {
		AF_Log_Error("Mesh has no indices!\n");
		return;
	    }

	    if (!mesh->vertices || !mesh->indices) {
		AF_Log_Error("Invalid vertex or index data!\n");
		return;
	    }

	    int vertexBufferSize = _entityCount * (mesh->vertexCount * sizeof(AF_Vertex));
	    //AF_Log("Init GL Buffers for vertex buffer size of: %i\n",vertexBufferSize);
	    
	    GLuint gVAO, gVBO, gEBO;
	    glGenVertexArrays(1, &gVAO);
	    glGenBuffers(1, &gVBO);
	    glGenBuffers(1, &gEBO);

	    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s)
	    glBindVertexArray(gVAO);
	    glBindBuffer(GL_ARRAY_BUFFER, gVBO);

	    // our buffer needs to be 8 floats (3*pos, 3*normal, 2*tex)
	    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, mesh->vertices, GL_STATIC_DRAW);
	    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	    // Bind the IBO and set the buffer data
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexCount * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW);

	    // Stride is 8 floats wide, 3*pos, 3*normal, 2*tex
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AF_Vertex), (void*)0);
	    glEnableVertexAttribArray(0);
	    
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AF_Vertex), (void*)(3 * sizeof(float)));
	    glEnableVertexAttribArray(1);
	    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AF_Vertex), (void*)(6 * sizeof(float)));
	    glEnableVertexAttribArray(2);

	    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	    glBindBuffer(GL_ARRAY_BUFFER, 0); 

	    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	    glBindVertexArray(0); 

	    mesh->vao = gVAO;
	    mesh->vbo = gVBO;
	    mesh->ibo = gEBO;
	    // Bind the IBO and set the buffer data
	    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, _meshList->meshes->indices, GL_STATIC_DRAW);
	    AF_CheckGLError( "Error InitMesh Buffers for OpenGL! \n");
    }
}




/*
====================
AF_LIB_DisplayRenderer
Display the renderer
====================
*/
void AF_LIB_DisplayRenderer(AF_Window* _window, AF_Entity* _cameraEntity, AF_ECS* _ecs, int shaderID){

    AF_CheckGLError( "Error at start of Rendering OpenGL! \n");
    AF_CTransform3D* cameraTransform = _cameraEntity->transform;
    AF_CCamera* _camera = _cameraEntity->camera;
    glClearColor(_camera->backgroundColor.x, _camera->backgroundColor.y, _camera->backgroundColor.z, _camera->backgroundColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable transparent blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // if in 2d mode, disable depth testing
    glDisable(GL_DEPTH_TEST);

    _window->title = _window->title;
    // Get the width and height from the frambuffer instead of the original set window size as open gl works in pixels
    _camera->windowWidth = _window->frameBufferWidth;//_window->windowWidth;
    _camera->windowHeight = _window->frameBufferHeight;//_window->windowHeight;


    // update the game camera with the window width
   
    // update camera vectors
    //TODO: put in switch if using mouse look to calculate front based on yaw and pitch
    AF_Vec3 front = _camera->cameraFront;//AF_Camera_CalculateFront(yaw, pitch);

    // calculate Right
    AF_Vec3 right = AFV3_NORMALIZE(AFV3_CROSS(front, _camera->cameraWorldUp));
	
    // calculate up
    AF_Vec3 up = AFV3_NORMALIZE(AFV3_CROSS(right, front));
	
    // Calculate view matrix:vs
    AF_Mat4 viewMatrix = AF_Math_Lookat(cameraTransform->pos, AFV3_ADD(cameraTransform->pos,front), up);
    _camera->viewMatrix = viewMatrix;

    // Calculate projection matrix
    _camera->projectionMatrix = AF_Camera_GetOrthographicProjectionMatrix(_window, _camera);
    

    // Set the winding order to clockwise
    //glFrontFace(GL_CW); // Clockwise winding order
    //glFrontFace(GL_CCW); // Counterclockwise winding order (default)


    // Enable face culling
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);  // Cull back faces
    //glDisable(GL_CULL_FACE);
	// reuse the main shader to set the projection and view as it will be the same for all entities
	// Start sending data to the shader/ GPU
	// First entity is normally the camera, so
	//int shaderID = _ecs->entities[1].mesh.material.shaderID;
	glUseProgram(shaderID); 

	int projLocation = glGetUniformLocation(shaderID, "projection");
	int viewLocation = glGetUniformLocation(shaderID, "view");

	// Send camrea data to shader
	// Projection Matrix
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, (float*)&_camera->projectionMatrix.rows[0]);

	// View Matrix
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (float*)&_camera->viewMatrix.rows[0]);

 

    for(uint32_t i = 0; i < _ecs->entitiesCount; i++){//_meshList->numMeshes; i++){
	
	AF_Entity* entity = &_ecs->entities[i];
	if(entity == NULL){
		continue;
	}

	// Skip entities that havn't been enabled
	if(entity->flags == FALSE){
		continue;
	}

	AF_Mesh* mesh = entity->mesh;

	BOOL meshHas = AF_Component_GetHas(mesh->enabled);
	BOOL hasEnabled = AF_Component_GetEnabled(mesh->enabled);
	// Skip if there is no rendering component
	if(meshHas == FALSE || hasEnabled == FALSE){
		continue;
	}
	
	// Stor the component values
	AF_CTransform3D* trans = entity->transform;
	if(trans == NULL){
		AF_Log_Error("AF_OpenGL_Renderer::Render: Null transforms\n");
		continue;
	}

	AF_CheckGLError( "Error at useProgram Rendering OpenGL! \n");
	int textureUniformLocation = glGetUniformLocation(shaderID, "image");
	int modelLocation = glGetUniformLocation(shaderID, "model");
    
		
	AF_Vec3* pos = &trans->pos;
	AF_Vec3* rot = &trans->rot;
	AF_Vec3* scale = &trans->scale;

	// convert to vec4 for model matrix
	AF_Vec4 modelPos = {pos->x, pos->y, pos->z, 1.0f};
	AF_Vec4 modelRot = {rot->x, rot->y, rot->z, 1.0f};
	AF_Vec4 modelScale = {scale->x, scale->y, scale->z, 1.0f};

	// apply rotation to postion and scaled matrix
	AF_Mat4 rotatedMatrix = AFM4_ROTATE_V4(AFM4_IDENTITY(), modelRot);//AFM4_DOT_M4( rotatedMatrix, scaleMatrix);
	// Apply scale
	AF_Mat4 scaleMatrix = AFM4_SCALE_V4(AFM4_IDENTITY(), modelScale); 

	// apply rotation to postion and scaled matrix
	AF_Mat4 rsMat = AFM4_DOT_M4(rotatedMatrix, scaleMatrix);
	
	// Construct the final model matrix with translation using row-major order
	AF_Mat4 modelMatrix = {{
	    {rsMat.rows[0].x, rsMat.rows[0].y, rsMat.rows[0].z, modelPos.x},
	    {rsMat.rows[1].x, rsMat.rows[1].y, rsMat.rows[1].z, modelPos.y},
	    {rsMat.rows[2].x, rsMat.rows[2].y, rsMat.rows[2].z, modelPos.z},
	    {rsMat.rows[3].x, rsMat.rows[3].y, rsMat.rows[3].z, 1.0f}
	}};

	//AF_Log_Mat4(modelMatrix);	
	// Set model matrix Mat4 for shader
	glUniformMatrix4fv(modelLocation, 1, GL_TRUE, (float*)&modelMatrix.rows[0]);
	
	
	// if we have a sprite, then render it.
	AF_CSprite* sprite = entity->sprite;
	BOOL hasSprite = AF_Component_GetHas(sprite->enabled);
	BOOL enabledSprite = AF_Component_GetEnabled(sprite->enabled);

	 GLfloat spriteSize[2] = {
			sprite->size.x, /// sprite->spriteSheetSize.x, 
			sprite->size.y// / sprite->spriteSheetSize.y
			};

	if(hasSprite == TRUE && enabledSprite == TRUE){
		// TODO: pack all this info about the sprite into a single data struct 
		// so talking to the shader is cheaper
		// Tell the shader about the sprite size
		int spriteSizeLocation = glGetUniformLocation(shaderID, "spriteSize");
		// Tell the shader about the sprite position
		if (spriteSizeLocation != -1) {
		    glUniform2fv(spriteSizeLocation, 1, spriteSize);
		}

		// Tell the shader about the sprite sheet size
       		int spriteSheetSizeLocation = glGetUniformLocation(shaderID, "spriteSheetSize");
		if(spriteSheetSizeLocation != -1){
			GLfloat spriteSheetSize[2] = {
				sprite->spriteSheetSize.x,
				sprite->spriteSheetSize.y
			};
			glUniform2fv(spriteSheetSizeLocation, 1, spriteSheetSize);
		}
		

		// if we have a Animation, then render it.
		AF_CAnimation* animation = entity->animation;
		BOOL hasAnimation = AF_Component_GetHas(animation->enabled);
		BOOL enabledAnimation = AF_Component_GetEnabled(animation->enabled);
		// setup a vec2 to update the sprite position		
		GLfloat spritePos[2] = {
			sprite->pos.x,
			sprite->pos.y
			};
		
		// if we have animation component, then update the spritePosition 
		// based on the current frame.
		if(hasAnimation == TRUE && enabledAnimation == TRUE){
		// Update the sprite position
			spritePos[0] = spritePos[0] + (spriteSize[0] * animation->currentFrame);
		}
		// Tell the shader about the updated sprite position
			int spritePosLocation = glGetUniformLocation(shaderID, "spritePos");
			if(spritePosLocation != -1){
			    glUniform2fv(spritePosLocation, 1, spritePos);
		}
	
		// Set the texture for the shader
		glUniform1i(textureUniformLocation,0);// _meshList->materials[0].textureID); 
		glActiveTexture(GL_TEXTURE0);
		// TODO implement binding the actual texture
		// unsigned int diffuseTexture = _mesh.material.diffuseTexture;
		glBindTexture(GL_TEXTURE_2D, mesh->material.textureID);
		AF_CheckGLError("Error blBindTexture diffuse ");
	}

	// if we have a sprite, but it isn't enabled then don't contine
	if(hasSprite == TRUE && enabledSprite == FALSE){
		continue;
	}
 
        // draw mesh
        glBindVertexArray(mesh->vao);//_meshList->vao);
        AF_CheckGLError( "Error bind vao Rendering OpenGL! \n");
        //---------------Send command to Graphics API to Draw Triangles------------
	
	unsigned int indexCount = mesh->indexCount;
        if(indexCount == 0){}
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        
        AF_CheckGLError( "Error drawElements Rendering OpenGL! \n");

	// Draw debug collision boundaries
	AF_CCollider* collider = entity->collider;


	// Renderer Debug
	if(mesh->showDebug == TRUE){
		// If we set the line colour we are telling the shader that we want to draw a debug line.
		// If the line colour is black, then shader will just render the texture normally
		// Set the debug line color to GREEN
		glUniform3f(glGetUniformLocation(shaderID, "debugLineColor"), 1.0f, 0.0f, 1.0f); 
		glDrawArrays(GL_LINE_LOOP, 0, mesh->vertexCount);
		// Set the debug color back to 0 so we don't affect other rendering
		glUniform3f(glGetUniformLocation(shaderID, "debugLineColor"), 0.0f, 0.0f, 0.0f); 

	


	// Collider Debug
	// TODO: use packed approach
	BOOL hasCollider = AF_Component_GetHas(collider->enabled);
	BOOL enabledCollider = AF_Component_GetEnabled(collider->enabled);

	if(hasCollider == TRUE && enabledCollider == TRUE){
		// draw debug lines
		// The verts needs to reflect the collider bound
		// Get the min and max points of the bounding box from the collider
		   // Get the bounds values
	    //float x = collider->bounds.x;
	    //float y = collider->bounds.y;
	    //float width = collider->bounds.w;
	    //float height = collider->bounds.h;

	    // Define the 4 vertices of the bounding box
	    
	    float quadVerts[8] = {
		0.5f, 0.5f,//x, y,                   // Bottom-left
		0.5f, -0.5f,//x + width, y,           // Bottom-right
		-0.5f, -0.5f,//x + width, y + height,  // Top-right
		-0.5f, 0.5f//x, y + height           // Top-left
	    };

	
	    //if(boundsVerts[0] == 0){}
	    // Define the 4 edges of the bounding box using indices of the vertices
	    
	    // Bind your VAO or VBO as necessary
	    // Setup the buffers
	    GLuint vao, vbo;
	    glGenVertexArrays(1, &vao);
	    glGenBuffers(1, &vbo);

	    glBindVertexArray(vao);
	    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	    // Update the VBO with the bounding box vertices
	    //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boundsVerts), boundsVerts);

	    // Set up vertex attribute pointers
	    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	    glEnableVertexAttribArray(0);


		
	    // If we set the line colour we are telling the shader that we want to draw a debug line.
	    // If the line colour is black, then shader will just render the texture normally
	    // Set the debug line color to GREEN
	    glUniform3f(glGetUniformLocation(shaderID, "debugLineColor"), 0.0f, 1.0f, 0.0f); 
	    //glDrawArrays(GL_LINE_LOOP, 0, mesh->vertexCount);

	    // Draw the lines based on the indices
	    //glDrawElements(GL_LINES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, boundsIndices);
	    glDrawArrays(GL_LINE_LOOP, 0, 4);

	    // Set the debug color back to 0 so we don't affect other rendering
	    glUniform3f(glGetUniformLocation(shaderID, "debugLineColor"), 0.0f, 0.0f, 0.0f);

	    // Cleanup
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	    glDeleteBuffers(1, &vbo);
	    glDeleteVertexArrays(1, &vao);
		
		//AF_Debug_DrawLine(0.0f,0.0f,0.0f,0.0f,0.0f, mesh, _camera->projectionMatrix, _camera->viewMatrix, modelMatrix);
	}

	}
	        
        glBindVertexArray(0);
        AF_CheckGLError( "Error bindvertexarray(0) Rendering OpenGL! \n");
	
	// Unbind textures
    	//unbind diffuse
    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_2D, 0);

    }
    AF_CheckGLError( "Error at end Rendering OpenGL! \n");
}

/*
====================
AF_LIB_DestroyRenderer
Destroy the renderer
====================
*/
void AF_LIB_DestroyRenderer(AF_ECS* _ecs){
    AF_Log("%s Destroyed\n", openglRendererFileTitle);
    for(uint32_t i  = 0; i < _ecs->entitiesCount; i++){
	    // optional: de-allocate all resources once they've outlived their purpose:
	    // ------------------------------------------------------------------------
	    glDeleteVertexArrays(1, &_ecs->entities[i].mesh->vao);
	    glDeleteBuffers(1, &_ecs->entities[i].mesh->vbo);
	    glDeleteBuffers(1, &_ecs->entities[i].mesh->ibo);
	    glDeleteProgram(_ecs->entities[i].mesh->material.shaderID);

    }
       
        //glDeleteTexture(_meshList->materials[0].textureID);
    AF_CheckGLError( "Error Destroying Renderer OpenGL! \n");
}

