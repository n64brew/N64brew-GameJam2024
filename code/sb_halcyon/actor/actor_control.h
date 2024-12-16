#ifndef ACTOR_CONTROLS_H
#define ACTOR_CONTROLS_H

// function prototypes

void actorControl_setJump(Actor *actor, ControllerData *control, float frame_time);
void actorControl_moveWithStick(Actor *actor, ControllerData *control, float camera_angle_around, float camera_offset);
void actor_setControlData(Actor *actor, ControllerData *control, float frame_time, float camera_angle_around, float camera_offset);

// function implementations

void actorControl_setJump(Actor *actor, ControllerData *control, float frame_time)
{
    bool canJump = false;
    bool wantJump = false;

    switch (actor->state)
    {
    case FALLING:
        if (control->held.a)
            actor->input.jump_time_buffer += frame_time;
        actor->input.jump_released = true;
        actor->input.jump_hold = false;
        canJump = false;
        break;
    case JUMP:
        if (control->held.a)
            actor->input.jump_time_held += frame_time;
        actor->input.jump_time_buffer = 0;
        canJump = false;
        wantJump = false;
        break;
    case STAND_IDLE:
    case RUNNING:
        actor->input.jump_released = true;
        actor->input.jump_hold = false;
        canJump = true;
        break;
    }

    if (control->pressed.a)
        wantJump = true;
    if (actor->input.jump_time_buffer > 0.0f && actor->input.jump_time_buffer < 0.3f)
        wantJump = true;

    if (wantJump && canJump)
    {

        actor->body.velocity.z = 30;
        actor->input.jump_hold = true;
        actor->input.jump_released = false;
        sound_wavPlay(SFX_JUMP, false);
        actor_setState(actor, JUMP);
    }
    else
    {
        actor->input.jump_released = true;
        actor->input.jump_hold = false;
    }

    if (control->released.a)
    {
        actor->input.jump_time_buffer = 0;
    }
}

void actorControl_moveWithStick(Actor *actor, ControllerData *control, float camera_angle_around, float camera_offset)
{
    int deadzone = 3;
    float stick_magnitude = 0;

    // Store previous camera angle and offset
    static float prev_camera_angle = -1.0f; // Initialize with a value that will trigger the first calculation
    static float prev_camera_offset = -1.0f;

    static float yaw = 0;

    // Check if the camera angle or offset has changed
    bool camera_changed = fabsf(camera_angle_around - prev_camera_angle) > 0.001f || fabsf(camera_offset - prev_camera_offset) > 0.001f;

    do
    {
        // Update the previous camera angle and offset values
        prev_camera_angle = camera_angle_around;
        prev_camera_offset = camera_offset;

        // Only change yaw if the camera angle or offset has changed
        if (fabsf(control->input.stick_x) >= deadzone || fabsf(control->input.stick_y) >= deadzone)
        {
            yaw = deg(fm_atan2f(control->input.stick_x, -control->input.stick_y) - rad(camera_angle_around - (0.5 * camera_offset)));
        }
        break;

    } while (camera_changed);

    actor->target_yaw = yaw;

    if (fabsf(control->input.stick_x) >= deadzone || fabsf(control->input.stick_y) >= deadzone)
    {
        Vector2 stick = {control->input.stick_x, control->input.stick_y};
        stick_magnitude = vector2_magnitude(&stick);
        actor->horizontal_target_speed = stick_magnitude * 7;
    }

    if (stick_magnitude == 0 && actor->state != JUMP && actor->state != FALLING)
    {
        actor->state = STAND_IDLE;
    }

    else if (stick_magnitude > 0 && actor->state != JUMP && actor->state != FALLING)
    {
        actor->state = RUNNING;
    }
}

void actor_setControlData(Actor *actor, ControllerData *control, float frame_time, float camera_angle_around, float camera_offset)
{

    actorControl_setJump(actor, control, frame_time);

    actorControl_moveWithStick(actor, control, camera_angle_around, camera_offset);
}

#endif