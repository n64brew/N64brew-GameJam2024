/*
===============================================================================
AF_OpenGL_Shader Implementation

This implementation is for OpenGL
===============================================================================
*/
#include "AF_Shader.h"
#include <string.h>
#include <GL/glew.h>
#define GL_SILENCE_DEPRECATION
/*
====================
AF_Shader_CheckCompileErrors
Check glsl errors against opengl 
====================
*/
uint32_t AF_Shader_CheckCompileErrors(uint32_t shader, const char* type)
{
    GLint success = 0;
    GLchar infoLog[1024];
    if(AF_STRING_IS_EMPTY(type)){
        AF_Log_Error("AF_Shader: CheckCompileErrors given an empty type\n");
        return (uint32_t)success;
    }
    if (strcmp(type, "PROGRAM") != 0)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            AF_Log_Error("AF_Shader: SHADER_COMPILATION_ERROR of type: %s %s \n", type, infoLog);
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            AF_Log_Error("AF_Shader: PROGRAM_LINKING_ERROR of type: %s %s \n", type, infoLog);
        }
    }
    return (uint32_t)success;
}

/*
====================
AF_Shader_GL_Load
Load a glsl shader from the fragement and vertex shader path
REturn the shader ID or return -1 if failed
====================
*/
int AF_Shader_Load(const char* _vertexShaderPath, const char* _fragmentShaderPath){
    int returnShaderID = -1;
    // Check if shader paths are empty
    if(AF_STRING_IS_EMPTY(_vertexShaderPath) || AF_STRING_IS_EMPTY(_fragmentShaderPath)){
        AF_Log_Error("AF_Shader: vertex or fragment shader path is empty\n");
        return -1;
    }
    
    char* _vertexShaderSource = AF_Util_ReadFile(_vertexShaderPath);
    char* _fragmentShaderSource = AF_Util_ReadFile(_fragmentShaderPath);

    

    /*
    const char* _vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

    const char* _fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";
    */
    // Null check the loaded shader code

    
    if(_vertexShaderPath == NULL || _fragmentShaderPath == NULL){
        AF_Log_Error("AF_Shader: vertex or fragment shader source is null \n");
        return -1;
    }
    // Check for empty shader code
    if(AF_STRING_IS_EMPTY(_vertexShaderSource) || AF_STRING_IS_EMPTY(_fragmentShaderSource)){
        AF_Log_Error("AF_Shader: vertex or fragment shader source is empty \n");
        return -1;
    }
    /**/
    // 2. compile shaders
    uint32_t vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* _vertexShaderSourceGL[] = {_vertexShaderSource};
    const GLchar* _fragmentShaderSourceGL[] = {_fragmentShaderSource};
    
    glShaderSource(vertex, 1, _vertexShaderSourceGL, NULL);
    glCompileShader(vertex);
    AF_Shader_CheckCompileErrors((uint32_t)vertex, "VERTEX");

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, _fragmentShaderSourceGL, NULL);
    glCompileShader(fragment);
    AF_Shader_CheckCompileErrors((uint32_t)fragment, "FRAGMENT");

    // shader Program
    returnShaderID = glCreateProgram();
    glAttachShader(returnShaderID, vertex);
    glAttachShader(returnShaderID, fragment);
    glLinkProgram(returnShaderID);
    AF_Shader_CheckCompileErrors((uint32_t)returnShaderID, "PROGRAM");

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if(returnShaderID == -1){
        AF_Log_Error("AF_Shader: Loading shader failed, returned shader ID is -1\n");
    }

    // Free the allocated shader source code from the file as it lives on the graphics card now
    free(_vertexShaderSource);
    free(_fragmentShaderSource);

    return returnShaderID;
}
