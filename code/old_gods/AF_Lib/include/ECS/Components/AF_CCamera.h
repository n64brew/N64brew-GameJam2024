/*
===============================================================================
AF_CCAMERA_H definitions

Definition for the camera component struct
and camera helper functions
===============================================================================
*/
#ifndef AF_CCAMERA_H
#define AF_CCAMERA_H
#include "AF_Math.h"
#include "ECS/Components/AF_CTransform3D.h"
#include "AF_Mat4.h"
#include "AF_Window.h"
#include "AF_Lib_Define.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
====================
AF_CCamera
Camera struct
====================
*/
typedef struct  {
    // TODO pack this
    BOOL has;
    BOOL enabled;
    Vec3 cameraFront;
    Vec3 cameraUp;
    Vec3 cameraRight;
    Vec3 cameraWorldUp;
    AF_FLOAT yaw;
    AF_FLOAT pitch;
    AF_FLOAT fov;
    AF_FLOAT nearPlane;
    AF_FLOAT farPlane;
    AF_FLOAT aspectRatio;
    AF_FLOAT windowWidth;
    AF_FLOAT windowHeight;
    AF_FLOAT tanHalfFov;
    AF_FLOAT rangeInv;
    BOOL orthographic;
    Mat4 projectionMatrix;
    Mat4 viewMatrix;
    Vec4 backgroundColor;
} AF_CCamera;

/*
====================
AF_CCamera_ZERO
Initialisation constructor function
====================
*/
inline static AF_CCamera AF_CCamera_ZERO(void){
	
	AF_CCamera returnCamera = {
		.cameraFront = {0,0,0},
		.cameraUp = {0,0,0},
		.cameraRight = {0,0,0},
		.cameraWorldUp = {0,0,0},
		.yaw = 0,
		.pitch = 0,
		.fov = 0,
		.nearPlane = 0,
		.aspectRatio = 0,
		.windowWidth = 0,
		.windowHeight = 0,
		.tanHalfFov = 0,
		.rangeInv = 0,
		.orthographic = FALSE,
		.projectionMatrix =  {{
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
	}},
		.viewMatrix = {{
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
	}},
		.backgroundColor = {0,0,0,0} 
	};

	return returnCamera;
}
/*
====================
AF_CCamera_ADD
Add the camera component
Initialise with enable and has set to true
====================
*/
inline static AF_CCamera AF_CCamera_ADD(BOOL _isOrthographic){
		
	AF_CCamera returnCamera = {
		.cameraFront = {0, 0, -1},
		.cameraUp = {0, 1, 0},
		.cameraRight =  {0,0,0},
		.cameraWorldUp = {0,1,0},
		.yaw = 0,
		.pitch = 0,
		.fov = 45,
		.nearPlane = 0,
		.farPlane = 100,
		.aspectRatio = 0,
		.windowWidth = 0,
		.windowHeight = 0,
		.tanHalfFov = AF_Math_Tan(45 / 2), //AF_Math_Tan(halfFov)
		.rangeInv = 1 / 100 - 0, // 1/farPlane - nearPlane
		.orthographic = _isOrthographic,
		.projectionMatrix = {{
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
		}},
		.viewMatrix = {{
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
		}},
		.backgroundColor = {0,0,0,0} 
	};
	return returnCamera;
}

/*
====================
AF_Ccamera_GetOrthographicProjectMatrix
Setup a camera component struct with settings for orthographic camera
====================
*/
/*
inline static Mat4 AF_Camera_GetOrthographicProjectionMatrix(AF_Window* _window, AF_CCamera* _camera){
    // Get the framebuffer width and height as we work in pixels
    _camera->windowWidth = _window->frameBufferWidth;//_window->windowWidth;
    _camera->windowHeight = _window->frameBufferHeight;//_window->windowHeight;
    _camera->fov = 45;
    _camera->nearPlane = 0;
    _camera->farPlane = 100;
    _camera->aspectRatio =  _camera->windowWidth / _camera->windowHeight;
    _camera->tanHalfFov = AF_Math_Tan(_camera->fov / 2);
    _camera->rangeInv = 1 / (_camera->farPlane - _camera->nearPlane);

    AF_FLOAT orthoWidth = 10;
    AF_FLOAT orthoHeight = orthoWidth / _camera->aspectRatio;
    AF_FLOAT right = orthoWidth / 2;   
    AF_FLOAT left = -orthoWidth / 2;
    AF_FLOAT top = orthoHeight / 2;      
    AF_FLOAT bottom = -orthoHeight / 2; 

    // Set the elements of the projection matrix
    Mat4 orthMatrix = {{
			{2/(right -left),0,0,0},
			{0,2/(top-bottom),0,0},
			{0,0,-2 / (_camera->farPlane - _camera->nearPlane),0},
			{0,0,0,1}
	}};
*/
/*
    Mat4 orth_projectionMatrix = identityM4;
    orth_projectionMatrix.rows[0].x = 2 / (right - left);
    orth_projectionMatrix.rows[0].y = 0;
    orth_projectionMatrix.rows[0].z = 0;
    orth_projectionMatrix.rows[0].w = 0;
    
    orth_projectionMatrix.rows[1].x = 0;
    orth_projectionMatrix.rows[1].y = 2 / (top - bottom);
    orth_projectionMatrix.rows[1].z = 0;
    orth_projectionMatrix.rows[1].w = 0; 

    orth_projectionMatrix.rows[2].x = 0;
    orth_projectionMatrix.rows[2].y = 0;
    orth_projectionMatrix.rows[2].z = -2 / (_camera->farPlane - _camera->nearPlane);
    orth_projectionMatrix.rows[2].w = 0;

    orth_projectionMatrix.rows[3].x = 0;
    orth_projectionMatrix.rows[3].y = 0;
    orth_projectionMatrix.rows[3].z = 0;
    orth_projectionMatrix.rows[3].w = 1;

    return orth_projectionMatrix;
*/
/*
	return returnCam;	
}
*/

/*
====================
AF_Camera_CalculateFront
Function to calculate the camera from based on input of yaw and pitch
likely set from the mouse x, y coords
====================
*/
static inline Vec3 AF_Camera_CalculateFront(AF_FLOAT _yaw, AF_FLOAT _pitch){
    // calculate the new Front vector
    AF_FLOAT x = AF_Math_Cos(AF_Math_Radians(_yaw)) * AF_Math_Cos(AF_Math_Radians(_pitch));
    AF_FLOAT y = AF_Math_Sin(AF_Math_Radians(_pitch));
    AF_FLOAT z = AF_Math_Sin(AF_Math_Radians(_yaw)) * AF_Math_Cos(AF_Math_Radians(_pitch));
    Vec3 normalisedVec3 = {x, y, z};
    
    Vec3 newFront= {0,0,0}; 
    newFront = Vec3_NORMALIZE(normalisedVec3);

    return newFront;
}



#ifdef __cplusplus
}
#endif

#endif //AF_CCAMERA_H
