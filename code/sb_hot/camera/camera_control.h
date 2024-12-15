#ifndef CAMERA_CONTROLS_H
#define CAMERA_CONTROLS_H

/* CAMERA_CONTROLS.H
here are all the camera control related functions */

int input(int input);

void camera_orbit_withStick(Camera *camera, ControllerData *data);
void camera_orbit_withCButtons(Camera *camera, ControllerData *data);
void camera_aim(Camera *camera, ControllerData *data);

void cameraControl_setOrbitalMovement(Camera *camera, ControllerData *data);

/* input
 auxiliary function for 8 directional movement*/

int input(int input)
{
    if (input == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/* camera_move_stick
changes the camera variables depending on controller input*/

void camera_orbit_withStick(Camera *camera, ControllerData *data)
{
    int deadzone = 8;
    float stick_x = 0;
    float stick_y = 0;

    if (fabsf(data->input.stick_x) >= deadzone || fabsf(data->input.stick_y) >= deadzone)
    {
        stick_x = data->input.stick_x;
        stick_y = data->input.stick_y;
    }

    if (stick_x == 0 && stick_y == 0)
    {
        camera->orbitational_target_velocity.x = 0;
        camera->orbitational_target_velocity.y = 0;
    }

    else if (stick_x != 0 || stick_y != 0)
    {
        camera->orbitational_target_velocity.x = stick_y;
        camera->orbitational_target_velocity.y = stick_x;
    }
}

void camera_orbit_withCButtons(Camera *camera, ControllerData *data)
{
    float input_x = 0;
    float input_y = 0;

    if ((data->held.c_right) || (data->held.c_left) || (data->held.c_up) || (data->held.c_down))
    {

        input_x = input(data->held.c_right) - input(data->held.c_left);
        input_y = input(data->held.c_up) - input(data->held.c_down);
    }

    if (input_x == 0)
        camera->orbitational_target_velocity.y = 0;
    else
        camera->orbitational_target_velocity.y = input_x * camera->settings.orbitational_max_velocity.y;

    if (input_y == 0)
        camera->orbitational_target_velocity.x = 0;
    else
        camera->orbitational_target_velocity.x = input_y * camera->settings.orbitational_max_velocity.x;
}

void cameraControl_freeCam(Camera *camera, ControllerData *data, float time)
{

    const float speed = 500.0f;

    float input_x = 0;
    float input_y = 0;
    float input_z = 0;

    if ((data->held.c_right) || (data->held.c_left) || (data->held.c_up) || (data->held.c_down) || (data->held.a) || (data->held.b))
    {

        input_x = input(data->held.c_right) - input(data->held.c_left);
        input_y = input(data->held.c_up) - input(data->held.c_down);
        input_z = input(data->held.a) - input(data->held.b);
    }

    camera->position.x += input_x * time * speed;
    camera->position.y += input_y * time * speed;
    camera->position.z += input_z * time * speed;
}

void camera_aim(Camera *camera, ControllerData *data)
{
    if (data->held.z)
        camera_setState(camera, AIMING);
    else
        camera_setState(camera, ORBITAL);
}

void cameraControl_setOrbitalMovement(Camera *camera, ControllerData *data)
{
    // camera_orbit_withStick(camera, data_1);
    camera_orbit_withCButtons(camera, data);
    camera_setState(camera, MINIGAME);
    // camera_aim(camera, data);
}

#endif