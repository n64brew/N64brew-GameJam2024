#include <libdragon.h>
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "../../core.h"
#include "../../minigame.h"

float __sequence_game_start_held_elapsed = 0.0f;

void sequence_game_process_controller(float deltatime)
{
    bool __sequence_game_start_pressed = false;
    bool __sequence_game_start_held = false;

    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);
        joypad_buttons_t held = joypad_get_buttons_held(controllerPort);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

        if (pressed.start)
        {
            __sequence_game_start_pressed = true;
        }

        if (held.start)
        {
            __sequence_game_start_held = true;
        }
    }

    // Pause.
    if (__sequence_game_start_pressed)
    {
        sequence_game_paused = !sequence_game_paused;
    }

    // Exit.
    if (__sequence_game_start_held && sequence_game_paused)
    {
        if (__sequence_game_start_held_elapsed >= GAME_EXIT_DURATION)
        {
            sequence_game_finished = true;
        }
        __sequence_game_start_held_elapsed += deltatime;
    }
    else
    {
        __sequence_game_start_held_elapsed = 0.0f;
    }
}