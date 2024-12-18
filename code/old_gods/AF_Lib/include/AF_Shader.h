/*
===============================================================================
AF_SHADER_H

Definitions for helper functions to load and use shaders 
===============================================================================
*/
#ifndef AF_SHADER_H
#define AF_SHADER_H
#include "AF_Util.h"
#include <stdint.h>
#include "AF_Rect.h"

#ifdef __cplusplus
extern "C" {    
#endif

/*
====================
AF_Shader_CheckCompileErrors
Check glsl errors against opengl 
====================
*/
uint32_t AF_Shader_CheckCompileErrors(uint32_t shader, const char* type);

/*
====================
AF_Shader_GL_Load
Load a glsl shader from the fragement and vertex shader path
REturn the shader ID or return -1 if failed
====================
*/
uint32_t AF_Shader_Load(const char* _vertexShaderPath, const char* _fragmentShaderPath);


#ifdef __cplusplus
}
#endif

#endif //SHADER_H
