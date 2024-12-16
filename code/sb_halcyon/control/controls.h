#ifndef CONTROLS_H
#define CONTROLS_H

typedef struct
{

    joypad_buttons_t pressed;
    joypad_buttons_t held;
    joypad_buttons_t released;
    joypad_inputs_t input;

    // RUMBLE
    bool rumble_active;
    bool has_rumbled;
    uint8_t rumble_time;

} ControllerData;

void controllerData_getInputs(ControllerData *data, uint8_t port);
void controllerData_rumbleStart(ControllerData *data, uint8_t port);
void controllerData_rumbleStop(ControllerData *data, uint8_t port);
void controllerData_rumbleFrames(ControllerData *data, uint8_t port, uint8_t frames);

void controllerData_getInputs(ControllerData *data, uint8_t port)
{
    data->pressed = joypad_get_buttons_pressed(port);
    data->held = joypad_get_buttons_held(port);
    data->released = joypad_get_buttons_released(port);
    data->input = joypad_get_inputs(port);

    // Check if the rumble pak has been unplugged
    if (!joypad_get_rumble_supported(port))
        controllerData_rumbleStop(data, port);
}

/* RUMBLE */

// Set rumble to active state
void controllerData_rumbleStart(ControllerData *data, uint8_t port)
{
    joypad_set_rumble_active(port, true);
    data->rumble_time = 0;
    data->rumble_active = true;
}

// Reset rumble to idle state
void controllerData_rumbleStop(ControllerData *data, uint8_t port)
{
    joypad_set_rumble_active(port, false);
    data->rumble_time = 0;
    data->rumble_active = false;
}

// Rumble controller for n number of frames
void controllerData_rumbleFrames(ControllerData *data, uint8_t port, uint8_t frames)
{

    if (data->rumble_active == false)
    {
        controllerData_rumbleStart(data, port);
    }

    data->rumble_time++;

    if (data->rumble_time >= frames)
        controllerData_rumbleStop(data, port);
}

///// 8 WAY /////

#define DEAD_ZONE 50
#define INPUT_DELAY 0.3f

// Treats joystick inputs as digital pad buttons, for menu navigation
void controllerData_8way(ControllerData *data)
{
    static float input_time = 0;
    const float current_time = display_get_delta_time();

    input_time -= current_time;

    if (input_time <= 0.0f)
    {
        if (data->input.stick_y > DEAD_ZONE)
        {
            data->pressed.d_up = 1;
            input_time = INPUT_DELAY;
        }
        else if (data->input.stick_y < -DEAD_ZONE)
        {
            data->pressed.d_down = 1;
            input_time = INPUT_DELAY;
        }

        if (data->input.stick_x > DEAD_ZONE)
        {
            data->pressed.d_right = 1;
            input_time = INPUT_DELAY;
        }
        else if (data->input.stick_x < -DEAD_ZONE)
        {
            data->pressed.d_left = 1;
            input_time = INPUT_DELAY;
        }
    }
}

#endif