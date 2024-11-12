#include "sequence3_input.h"
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
            sequence3_b_btn_held_duration += deltatime;
            fprintf(stderr, "Controller %u has held B for %f\n", controllerPort, sequence3_b_btn_held_duration);

            // If the button is held for more than 3 seconds, end the minigame
            if (sequence3_b_btn_held_duration > 3.0f)
            {
                sequence3_finished = true;
            }
        }
        else
        {
            sequence3_b_btn_held_duration = 0;
        }

        // Pause the video.
        if (pressed.a)
        {
            sequence3_paused = !sequence3_paused;
            fprintf(stderr, "Controller %u pressed A. Video is now %s.\n", controllerPort, sequence3_paused ? "paused" : "unpaused");
        }

        // Rewind the video.
        if (pressed.z)
        {
            sequence3_rewind = true;
            fprintf(stderr, "Controller %u pressed Z. Video will now rewind.\n", controllerPort);
        }
    }
}
