#include <libdragon.h>
#include "../../../core.h"
#include "../../../minigame.h"
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "sequence_game_initialize.h"
#include "sequence_game_graphics.h"

void sequence_game_update(float deltatime)
{
    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        struct Controller *controller = &controllers[i];
        struct Character *character = &characters[i];

        character->frames++;
        
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);
        joypad_buttons_t held = joypad_get_buttons_held(controllerPort);
        joypad_buttons_t released = joypad_get_buttons_released(controllerPort);
        joypad_8way_t direction = joypad_get_direction(controllerPort, JOYPAD_2D_ANY);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

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
                controllers->start_down = 0;
                controllers->start_up = 0;
                controllers->start_held_elapsed = 0.0f;
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
            if (sequence_game_start_held_elapsed == 0.0f)
            {
                sequence_game_paused = false;
            }

            for (size_t i = 0; i < core_get_playercount(); i++)
            {
                controllers->start_down = 0;
                controllers->start_up = 0;
                controllers->start_held_elapsed = 0.0f;
            }
        }

        // Quit
        if (sequence_game_paused == true)
        {
            if (held.start == true)
            {
                if (sequence_game_player_holding_start == -1)
                {
                    sequence_game_player_holding_start = i;
                }

                if (sequence_game_player_holding_start == i)
                {
                    if (sequence_game_start_held_elapsed >= GAME_EXIT_DURATION)
                    {
                        sequence_game_finished = true;
                    }
                    if (controller->start_held_elapsed >= GAME_EXIT_THRESHOLD_DURATION)
                    {
                        sequence_game_start_held_elapsed += deltatime;
                    }
                    controller->start_held_elapsed += deltatime;
                }
            }
            else
            {
                controller->start_held_elapsed = 0.0f;
                if (sequence_game_player_holding_start == i)
                {
                    sequence_game_player_holding_start = -1;
                    sequence_game_start_held_elapsed = 0.0f;
                }
            }
        }

        // if (character->locked_for_frames == 0)
        // {
        // Movement
        switch (direction)
        {
        case JOYPAD_8WAY_UP:
            character->y -= 1;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        case JOYPAD_8WAY_UP_RIGHT:
            character->x += 1;
            character->y -= 1;
            character->direction = RIGHT;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        case JOYPAD_8WAY_RIGHT:
            character->x += 1;
            character->direction = RIGHT;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        case JOYPAD_8WAY_DOWN_RIGHT:
            character->x += 1;
            character->y += 1;
            character->direction = RIGHT;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        case JOYPAD_8WAY_DOWN:
            character->y += 1;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        case JOYPAD_8WAY_DOWN_LEFT:
            character->x -= 1;
            character->y += 1;
            character->direction = LEFT;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        case JOYPAD_8WAY_LEFT:
            character->x -= 1;
            character->direction = LEFT;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        case JOYPAD_8WAY_UP_LEFT:
            character->x -= 1;
            character->y -= 1;
            character->direction = LEFT;
            if (character->locked_for_frames == 0)
                character->action = WALK;
            break;
        default:
            if (character->locked_for_frames == 0)
                character->action = BASE;
            break;
        }
        // }

        if (pressed.a)
        {
            character->locked_for_frames = 4 * SEQUENCE_GAME_MALLARD_SLAP_FRAMES; // Lock for 12 frames.
            character->action = SLAP;
            character->frames = 0;
        }

        if (character->locked_for_frames > 0)
        {
            character->locked_for_frames--;
        }
    }
}