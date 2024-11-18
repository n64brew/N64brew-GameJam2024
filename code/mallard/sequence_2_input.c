#include "sequence_2_input.h"
#include "../../core.h"
#include "../../minigame.h"

void sequence_2_process_controller(float deltatime)
{
    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

        // Skip the sequence.
        if (pressed.start)
        {
            sequence_2_finished = true;
        }
    }
}
