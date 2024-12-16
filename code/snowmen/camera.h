#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#include "actor.h"

typedef struct Camera{
    T3DVec3 camPos;
    T3DVec3 camTarget;
    T3DVec3 cameraFront;
    T3DVec3 cameraUp;
    T3DVec3 cameraOffset;
    float Yaw3rdPerson;
    float Pitch3rdPerson;
} Camera;

Camera CreateCamera();

void CameraInit(Camera* camera, Actor* PlayerActor);

void CameraLoop(Camera* camera, Actor* PlayerActor, T3DViewport* viewport);

void OrbitCameraAroundTarget(Camera* ThisCamera, float ChangeX, float ChangeY);

void UpdateCameraFromInput(Camera* ThisCamera, const joypad_buttons_t* held);








#endif