#include "sequence_3_input.h"
#include "../../core.h"
#include "../../minigame.h"

void sequence_3_process_controller(float deltatime)
{
    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);
        joypad_buttons_t held = joypad_get_buttons_held(controllerPort);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

        // Exit the video.
        if (held.b)
        {
            sequence_3_b_btn_held_duration += deltatime;

            // If the button is held for more than 3 seconds, end the minigame
            if (sequence_3_b_btn_held_duration > 3.0f)
            {
                sequence_3_finished = true;
            }
        }
        else
        {
            sequence_3_b_btn_held_duration = 0;
        }

        // Pause the video.
        if (pressed.a)
        {
            sequence_3_paused = !sequence_3_paused;
        }

        // Rewind the video.
        if (pressed.z)
        {
            sequence_3_rewind = true;
        }

        // Skip the sequence.
        if (pressed.start)
        {
            sequence_3_finished = true;
        }
    }
}
