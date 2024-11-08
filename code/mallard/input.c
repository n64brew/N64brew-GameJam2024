#include "input.h"
#include "../../core.h"
#include "../../minigame.h"

void handle_input(float deltatime)
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

        // Exit the game.
        if (held.b)
        {
            if (!b_btn_held)
            {
                // Record the time when the button is first pressed
                b_btn_held = true;
            }
            else
            {
                b_btn_held_duration += deltatime;
                fprintf(stderr, "Controller %u has held B for %f\n", controllerPort, b_btn_held_duration);

                // If the button is held for more than 3 seconds, end the minigame
                if (b_btn_held_duration > 3.0f)
                {
                    minigame_end();
                }
            }
        }
        else
        {
            b_btn_held = false;
            b_btn_held_duration = 0;
        }

        // Pause the video.
        if (pressed.a)
        {
            paused = !paused;
            fprintf(stderr, "Player pressed A. Video is now %s.\n", paused ? "paused" : "unpaused");
        }
    }
}
