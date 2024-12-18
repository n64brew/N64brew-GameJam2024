/* 
===============================================================================
AF_OPENGL_DEBUG_H
Implementation file for the AF_Debug, to be used with OpenGL
===============================================================================
 */
#include "AF_Debug.h"
#include "AF_Vertex.h"
#include <GL/glew.h>
#define GL_SILENCE_DEPRECATION
#include "AF_Log.h"

/*
====================
vertex shader
the vertex shader used for debugging stored as a char* 
====================
*/
const char* vertexShaderSource = 
"#version 330 core\n"
"layout(location = 0) in vec3 position;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = projection * view * model * vec4(position, 1.0);\n"
"}\n";

/*
====================
Fragment shader
the fragment shader used for debugging stored as a char* 
====================
*/
const char* fragmentShaderSource = 
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 lineColor;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = vec4(lineColor, 1.0);\n"
"}\n";

/*
====================
Test verts use for debuging
====================
*/
AF_Vertex debugLineVerts[] = {
	    // Line 1
	{{-0.5f, -0.5f, 0.0f},{0.0f, 0.0f, 0.0f},{0.0f, 0.0f}},
	{{0.5f,  0.5f, 0.0f},{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}
	};


/*
====================
Setup opengl buffers for rendering debug stuff
====================
*/
AF_Mesh AF_Debug_Init_DrawLine(void){
		
	// Setup the buffers
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(debugLineVerts), debugLineVerts, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0);

	// compile the shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Check for compile errors...

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Check for compile errors...

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Setup the returned mesh data
	AF_Mesh returnMesh = AF_Mesh_ZERO();
	//returnMesh.has = TRUE;
	returnMesh.enabled = AF_Component_SetEnabled(returnMesh.enabled, TRUE);
	returnMesh.vertices = &debugLineVerts[0];
	returnMesh.vertexCount = 6;
	returnMesh.indices = NULL;
	returnMesh.indexCount = 0;
	returnMesh.vao = VAO;
	returnMesh.vbo = VBO;
	returnMesh.ibo = 0;
	AF_Material debugMaterial = AF_Material_ZERO();
	debugMaterial.shaderID = (uint32_t)shaderProgram;
	// debugMaterial.textureID = 0;
	returnMesh.material = debugMaterial;

	//AF_Log("AF_OpenGL_Debug:: Init buffers\n");
	return returnMesh;

}


/*
====================
AF_Debug_DrawPoint
Drwa a pixel point at the given screen space location
Provide an x, and y location as well as a size
====================
*/
void AF_Debug_DrawPoint(float _xPos1, float _yPos1, float _size){
	// Draw point to screen
	if(_xPos1 == 0 || _yPos1 == 0 || _size == 0){}
}

/*
====================
AF_Debug_DrawLine
Drwa a line between two points,
Also provide a line thickness size
====================
*/
void AF_Debug_DrawLine(float _xPos1, float _yPos1, float _xPos2, float _yPos2, float _size, AF_Mesh* _debugMesh, AF_Mat4 _viewMatrix, AF_Mat4 _projectionMatrix, AF_Mat4 _modelMatrix){
	
	// Draw Line to screen
	if(_xPos1 == 0 || _yPos1 == 0 || _size == 0 || _xPos2 == 0 || _yPos2 == 0 || _modelMatrix.rows[0].x == 0){}

	if(_debugMesh == NULL){
		AF_Log_Error("AF_Debug_DrawLine: passed a null _debug mesh\n");
	}


	//AF_Log("AF_Debug_DrawLine:: draw debug line\n");

	// Bind the shader program
	int shaderProgram = _debugMesh->material.shaderID;
	glUseProgram(shaderProgram);

	// Set the uniform values
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (float*)&_projectionMatrix.rows[0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (float*)&_viewMatrix.rows[0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (float*)&_modelMatrix.rows[0]);
	glUniform3f(glGetUniformLocation(shaderProgram, "lineColor"), 0.0f, 1.0f, 0.0f); // Set line color to red

	// Bind the VAO
	glBindVertexArray(_debugMesh->vao);

	// Draw lines
	glDrawArrays(GL_LINES, 0, _debugMesh->vertexCount);
	// stop using the shader
	glUseProgram(0);

	// Unbind the VAO
	glBindVertexArray(0);

}
