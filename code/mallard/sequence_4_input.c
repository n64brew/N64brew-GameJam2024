#include "sequence_4_input.h"
#include "../../core.h"
#include "../../minigame.h"

void sequence_4_process_controller(float deltatime)
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
            fprintf(stderr, "Controller %u pressed Start. Sequence will now skip.\n", controllerPort);
            sequence_4_finished = true;
        }
    }
}
