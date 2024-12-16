#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

#include "camera.h"

Camera CreateCamera()
{
  return (Camera) {
    //T3DVec3 camPos;
    //T3DVec3 camTarget;
    .cameraFront = (T3DVec3){{0,0,-1}},
    .cameraUp = (T3DVec3){{0,1,0}},
    .cameraOffset = (T3DVec3){{0.f, 0.f, 0.f}},
    .Yaw3rdPerson = 0.f,
    .Pitch3rdPerson = 0.f
  };
}

void CameraInit(Camera* camera, Actor* PlayerActor)
{
  //camera->camTarget = PlayerActor->Position;
  camera->camTarget =  (T3DVec3){{ 0.f, 40.f, 25.f }};//PlayerActor->Position;
  //camera->camTarget.v[2] -= 20.f;
  //camera->camTarget.v[1] += 40.f;

  camera->camPos.v[0] = camera->camTarget.v[0];
  camera->camPos.v[1] = camera->camTarget.v[1] + 10.f;
  camera->camPos.v[2] = camera->camTarget.v[2] + 100.f;
}

void CameraLoop(Camera* camera, Actor* PlayerActor, T3DViewport* viewport)
{
  //camera->camTarget =  (T3DVec3){{ 0.f, 40.f, 25.f }};//PlayerActor->Position;
  //camera->camTarget.v[1] += 40.f;

  camera->camPos.v[0] = camera->camTarget.v[0] + camera->cameraFront.v[0]*-300.f;
  camera->camPos.v[1] = camera->camTarget.v[1] + camera->cameraFront.v[1]*-300.f;
  camera->camPos.v[2] = camera->camTarget.v[2] + camera->cameraFront.v[2]*-300.f;

  t3d_viewport_set_projection(viewport, T3D_DEG_TO_RAD(60.0f), 10.0f, 1000.0f);
  t3d_viewport_look_at(viewport, &camera->camPos, &camera->camTarget, &camera->cameraUp);
    /*T3DMat4 ident;
        t3d_mat4_identity(&ident);

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    viewport->matCamera.m[i][j] = ident.m[i][j];
                }
            }*/
  t3d_viewport_attach(viewport);
}


void OrbitCameraAroundTarget(Camera* ThisCamera, float ChangeX, float ChangeY)
{
  ThisCamera->Yaw3rdPerson += ChangeX;
  ThisCamera->Pitch3rdPerson += ChangeY;
  ThisCamera->cameraFront = (T3DVec3){{
  fm_cosf(ThisCamera->Yaw3rdPerson) * fm_cosf(ThisCamera->Pitch3rdPerson),
  fm_sinf(ThisCamera->Pitch3rdPerson),
  fm_sinf(ThisCamera->Yaw3rdPerson) * fm_cosf(ThisCamera->Pitch3rdPerson)
  }};
  t3d_vec3_norm(&ThisCamera->cameraFront);
}


void UpdateCameraFromInput(Camera* ThisCamera, const joypad_buttons_t* held)
{
  //joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);
  if(held->c_right) 
  {
    float ChangeX = -0.05f;
    OrbitCameraAroundTarget(ThisCamera, ChangeX, 0.0f);
  }
  if(held->c_left) 
  {
    float ChangeX = 0.05f;
    OrbitCameraAroundTarget(ThisCamera, ChangeX, 0.0f);
  }
  if(held->c_up) 
  {
    float ChangeY = 0.05f;
    OrbitCameraAroundTarget(ThisCamera, 0.0f, ChangeY);
  }
  if(held->c_down) 
  {
    float ChangeY = -0.05f;
    OrbitCameraAroundTarget(ThisCamera, 0.0f, ChangeY);
  }
}













//No clip camera, needs work:
  /*cameraOffset = (T3DVec3) {{
    cameraOffset.v[0] + fm_cosf(ChangeX) * fm_cosf(ChangeY),
    cameraOffset.v[1], //+ fm_sinf(ChangeY),
    cameraOffset.v[2] + fm_sinf(ChangeX) * fm_cosf(ChangeY)
  }};*/

  //T3DVec3 TempVec = (T3DVec3){{ ChangeX, ChangeY, 0.f}};
  //t3d_vec3_add(&cameraOffset, &cameraOffset, &TempVec);

  /*cameraOffset = (T3DVec3) {{
    cameraOffset.v[0] + ChangeX,
    cameraOffset.v[1] + ChangeY,
    cameraOffset.v[2]
  }};*/

  //t3d_vec3_add(&camPos, &camPos, &TempVec);
  // t3d_vec3_len2(difference between them)
  /*T3DVec3 TempVec;
  t3d_vec3_diff(&TempVec, &camPos, &camTarget);
  float distance = t3d_vec3_len2(&TempVec);

  distance += ChangeX;*/

  /*T3DVec3 ForwardScaled = {{
    cameraFront.v[0] *= distance,
    cameraFront.v[0] *= distance,
    cameraFront.v[0] *= distance
  }};*/
  // Set new distance by moving the position along the forward vector
  //t3d_vec3_add(&camPos, &camTarget, &ForwardScaled);
  //void t3d_mat4_look_at(T3DMat4 *mat, const T3DVec3 *eye, const T3DVec3 *target, const T3DVec3 *up);
  //also what is this: t3d_mat4_perspective