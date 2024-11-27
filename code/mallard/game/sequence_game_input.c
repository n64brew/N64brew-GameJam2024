#include <libdragon.h>
#include "../../../core.h"
#include "../../../minigame.h"
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "sequence_game_initialize.h"
#include "sequence_game_graphics.h"

#define BOOST 2.0
#define SQRT_ONE_HALF 0.70710678118

#define SNOWMAN_SPAWN_FREQUENCY 3.0f
float time_since_last_snowman_spawn = 0.0f;

void sequence_game_update(float deltatime)
{
    if (time_since_last_snowman_spawn >= SNOWMAN_SPAWN_FREQUENCY)
    {
        time_since_last_snowman_spawn = 0.0f;
        snowmen = add_snowman(snowmen);

        int count = 0;
        struct Snowman *curr = snowmen;
        while (curr != NULL)
        {
            // Increment count by 1
            count++;

            // Move pointer to next node
            curr = curr->next;
        }
        fprintf(stderr, "Snowmen spawned: %i\n", count);
    }

    time_since_last_snowman_spawn += deltatime;

    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        struct Controller *controller = &controllers[i];
        struct Duck *duck = &ducks[i];

        duck->frames++;

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

        // Movement
        switch (direction)
        {
        case JOYPAD_8WAY_UP:

            // Position
            if (held.b)
            {
                duck->y -= 1 * BOOST;
            }
            else
            {
                duck->y -= 1;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            break;

        case JOYPAD_8WAY_UP_RIGHT:

            // Position
            if (held.b)
            {
                duck->x += (SQRT_ONE_HALF * BOOST);
                duck->y -= (SQRT_ONE_HALF * BOOST);
            }
            else
            {
                duck->x += SQRT_ONE_HALF;
                duck->y -= SQRT_ONE_HALF;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            // Direction
            duck->direction = RIGHT;
            break;

        case JOYPAD_8WAY_RIGHT:

            // Position
            if (held.b)
            {
                duck->x += 1 * BOOST;
            }
            else
            {
                duck->x += 1;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            // Direction
            duck->direction = RIGHT;
            break;

        case JOYPAD_8WAY_DOWN_RIGHT:

            // Position
            if (held.b)
            {
                duck->x += (SQRT_ONE_HALF * BOOST);
                duck->y += (SQRT_ONE_HALF * BOOST);
            }
            else
            {
                duck->x += SQRT_ONE_HALF;
                duck->y += SQRT_ONE_HALF;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            // Direction
            duck->direction = RIGHT;
            break;

        case JOYPAD_8WAY_DOWN:

            // Position
            if (held.b)
            {
                duck->y += 1 * BOOST;
            }
            else
            {
                duck->y += 1;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            break;

        case JOYPAD_8WAY_DOWN_LEFT:

            // Position
            if (held.b)
            {
                duck->x -= (SQRT_ONE_HALF * BOOST);
                duck->y += (SQRT_ONE_HALF * BOOST);
            }
            else
            {
                duck->x -= SQRT_ONE_HALF;
                duck->y += SQRT_ONE_HALF;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            // Direction
            duck->direction = LEFT;
            break;

        case JOYPAD_8WAY_LEFT:

            // Position
            if (held.b)
            {
                duck->x -= 1 * BOOST;
            }
            else
            {
                duck->x -= 1;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            // Direction
            duck->direction = LEFT;
            break;

        case JOYPAD_8WAY_UP_LEFT:

            // Position
            if (held.b)
            {
                duck->x -= (SQRT_ONE_HALF * BOOST);
                duck->y -= (SQRT_ONE_HALF * BOOST);
            }
            else
            {
                duck->x -= SQRT_ONE_HALF;
                duck->y -= SQRT_ONE_HALF;
            }
            // Action
            if (duck->locked_for_frames == 0)
            {
                if (held.b)
                {
                    duck->action = DUCK_RUN;
                }
                else
                {
                    duck->action = DUCK_WALK;
                }
            }
            // Direction
            duck->direction = LEFT;
            break;

        default:
            if (duck->locked_for_frames == 0)
                duck->action = DUCK_BASE;
            break;
        }

        if (duck->x > MAX_X)
        {
            duck->x = MAX_X;
        }

        if (duck->x < MIN_X)
        {
            duck->x = MIN_X;
        }

        if (duck->y > MAX_Y)
        {
            duck->y = MAX_Y;
        }

        if (duck->y < MIN_Y)
        {
            duck->y = MIN_Y;
        }

        if (pressed.a)
        {
            duck->locked_for_frames = 4 * SEQUENCE_GAME_MALLARD_SLAP_FRAMES; // Lock for 12 frames.
            duck->action = DUCK_SLAP;
            duck->frames = 0;
        }

        if (duck->locked_for_frames > 0)
        {
            duck->locked_for_frames--;
        }
    }
}