#include <libdragon.h>
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "../../core.h"
#include "../../minigame.h"

int __player_holding_start = -1;

struct Controller
{
    unsigned int start_down;
    unsigned int start_up;
    float start_held_elapsed;
};

struct Controller *controllers;

void sequence_game_process_controller(float deltatime)
{
    if (controllers == NULL)
    {
        controllers = malloc(core_get_playercount() * sizeof(struct Controller));
        for (size_t i = 0; i < core_get_playercount(); i++)
        {
            controllers[i].start_down = 0;
            controllers[i].start_up = 0;
            controllers[i].start_held_elapsed = 0.0f;
        }
    }

    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);
        joypad_buttons_t held = joypad_get_buttons_held(controllerPort);
        joypad_buttons_t released = joypad_get_buttons_released(controllerPort);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

        struct Controller *controller = &controllers[i];

        // Pause
        if (sequence_game_paused == false && pressed.start)
        {
            controller->start_down = 1;
        }
        if (sequence_game_paused == false && controller->start_down == 1 && released.start)
        {
            controller->start_up = 1;
        }
        if (sequence_game_paused == false && controller->start_up == 1 && controller->start_down == 1)
        {
            sequence_game_paused = true;

            for (size_t i = 0; i < core_get_playercount(); i++)
            {
                controllers[i].start_down = 0;
                controllers[i].start_up = 0;
                controllers[i].start_held_elapsed = 0.0f;
            }
        }

        // Unpause
        if (sequence_game_paused == true && pressed.start)
        {
            controller->start_down = 1;
        }
        if (sequence_game_paused == true && controller->start_down == 1 && released.start)
        {
            controller->start_up = 1;
        }
        if (sequence_game_paused == true && controller->start_up == 1 && controller->start_down == 1)
        {
            sequence_game_paused = false;

            for (size_t i = 0; i < core_get_playercount(); i++)
            {
                controllers[i].start_down = 0;
                controllers[i].start_up = 0;
                controllers[i].start_held_elapsed = 0.0f;
            }
        }

        // Quit
        if (sequence_game_paused == true)
        {
            if (held.start == true)
            {
                if (__player_holding_start == -1)
                {
                    __player_holding_start = i;
                }

                if (__player_holding_start == i)
                {
                    if (controller->start_held_elapsed >= GAME_EXIT_DURATION)
                    {
                        sequence_game_finished = true;
                    }
                    controller->start_held_elapsed += deltatime;
                    sequence_game_start_held_elapsed = controller->start_held_elapsed;
                }
            }
            else
            {
                controller->start_held_elapsed = 0.0f;
                if (__player_holding_start == i)
                {
                    __player_holding_start = -1;
                    sequence_game_start_held_elapsed = 0.0f;
                }
            }
        }
    }
}