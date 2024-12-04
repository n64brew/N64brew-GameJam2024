#include <libdragon.h>
#include "../../../core.h"
#include "../../../minigame.h"
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "sequence_game_initialize.h"
#include "sequence_game_graphics.h"
#include "sequence_game_snowman.h"

#define BOOST 2.0
#define SQRT_ONE_HALF 0.70710678118
#define SNOWMAN_SPAWN_FREQUENCY_ONE 3.0f
#define SNOWMAN_SPAWN_FREQUENCY_TWO 2.0f
#define SNOWMAN_SPAWN_FREQUENCY_THREE 1.0f
#define SNOWMAN_SPAWN_FREQUENCY_FOUR 0.5f

float time_elapsed_since_last_snowman_spawn = 0.0f;
float time_elapsed = 0.0f;
float SNOWMAN_SPAWN_FREQUENCY;

void update_snowmen(float deltatime)
{
    if (time_elapsed >= 0.0f && time_elapsed < 15.0f)
    {
        SNOWMAN_SPAWN_FREQUENCY = SNOWMAN_SPAWN_FREQUENCY_ONE;
    }
    else if (time_elapsed >= 15.0f && time_elapsed < 30.0f)
    {
        SNOWMAN_SPAWN_FREQUENCY = SNOWMAN_SPAWN_FREQUENCY_TWO;
    }
    else if (time_elapsed >= 30.0f && time_elapsed < 45.0f)
    {
        SNOWMAN_SPAWN_FREQUENCY = SNOWMAN_SPAWN_FREQUENCY_THREE;
    }
    else if (time_elapsed >= 45.0f && time_elapsed < 60.0f)
    {
        SNOWMAN_SPAWN_FREQUENCY = SNOWMAN_SPAWN_FREQUENCY_FOUR;
    }

    // Update snowmen.
    Snowman *temporary = snowmen;
    while (temporary != NULL)
    {
        // Update the frame counter for the snowman; this is used by the animation when rendering.
        temporary->frames++;

        // Update the time counter for the snowman; this is will be used to render frost/ice/snow around the snowmen that've been around for a while.
        temporary->time += deltatime;

        // Update the collision box for the snowman. Don't do this for each frame, because when it moves around it can get "caught" on other entities, and feels bad.
        temporary->collision_box_x1 = temporary->x;
        temporary->collision_box_y1 = temporary->y + 8;
        temporary->collision_box_x2 = temporary->x + 12;
        temporary->collision_box_y2 = temporary->y + 16;

        temporary = temporary->next;
    }

    // Add snowman.
    if (time_elapsed_since_last_snowman_spawn >= SNOWMAN_SPAWN_FREQUENCY)
    {
        add_snowman();
        time_elapsed_since_last_snowman_spawn = 0.0f;
    }

    time_elapsed += deltatime;
    time_elapsed_since_last_snowman_spawn += deltatime;
}

void ducks_bubble_sort()
{
    bool swapped = true;

    while (swapped)
    {
        Duck **prev = &ducks;
        Duck *curr;
        Duck *next;

        swapped = false;
        for (curr = ducks; curr; prev = &curr->next, curr = curr->next)
        {
            next = curr->next;

            if (next && curr->collision_box_y2 > next->collision_box_y2)
            {
                curr->next = next->next;
                next->next = curr;
                *prev = next;

                swapped = true;
            }
        }
    }
}

// Set the collision box of the ducks based on their current action and frame.
void update_ducks(float deltatime)
{
    for (size_t i = 0; i < 4; i++)
    {
        Duck *duck = get_duck_by_id(i);

        // Update the frame counter for the duck; this is used by the animation when rendering.
        duck->frames++;

        // Update the collision box for the duck. Don't do this for each frame, because when it moves around it can get "caught" on other entities, and feels bad.
        duck->collision_box_x1 = duck->x + 8;
        duck->collision_box_y1 = duck->y + 16;
        duck->collision_box_x2 = duck->x + 24;
        duck->collision_box_y2 = duck->y + 24;
    }

    ducks_bubble_sort();
}

void process_input_start_button(Controller *controller, joypad_buttons_t pressed, joypad_buttons_t held, joypad_buttons_t released, size_t i, float deltatime)
{
    if (!sequence_game_paused && pressed.start)
    {
        controller->start_down = 1;
    }
    if (!sequence_game_paused && controller->start_down && released.start)
    {
        controller->start_up = 1;
    }
    if (!sequence_game_paused && controller->start_down && controller->start_up)
    {
        sequence_game_paused = true;

        for (size_t j = 0; j < core_get_playercount(); j++)
        {
            Controller *curr = &controllers[j];
            curr->start_down = 0;
            curr->start_up = 0;
            curr->start_held_elapsed = 0.0f;
        }
    }

    if (sequence_game_paused == true && pressed.start)
    {
        controller->start_down = 1;
    }
    if (sequence_game_paused && controller->start_down && released.start)
    {
        controller->start_up = 1;
    }
    if (sequence_game_paused && controller->start_down && controller->start_up)
    {
        // Unpauses the game because it wasn't held long enough.
        if (sequence_game_start_held_elapsed == 0.0f)
        {
            sequence_game_paused = false;
        }

        // Reset the controller state for all controllers.
        for (size_t j = 0; j < core_get_playercount(); j++)
        {
            Controller *curr = &controllers[j];
            curr->start_down = 0;
            curr->start_up = 0;
            curr->start_held_elapsed = 0.0f;
        }
    }

    if (sequence_game_paused)
    {
        if (held.start)
        {
            if (sequence_game_player_holding_start == -1)
            {
                sequence_game_player_holding_start = i;
            }

            if (sequence_game_player_holding_start == i)
            {
                if (sequence_game_start_held_elapsed >= GAME_EXIT_DURATION)
                {
                    sequence_game_should_cleanup = true;
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
            if (sequence_game_player_holding_start == i)
            {
                sequence_game_player_holding_start = -1;
                sequence_game_start_held_elapsed = 0.0f;
            }
        }
    }
}

typedef struct Collision
{
    bool x;
    bool y;
} Collision;

bool detect_collision(Rect a, Rect b)
{
    return a.x1 < b.x2 && a.x2 > b.x1 && a.y1 < b.y2 && a.y2 > b.y1;
}

Collision check_for_duck_collisions(Duck *duck, size_t i, Vector2 movement)
{
    Collision collision = (Collision){.x = false, .y = false};

    Rect duckPotentialCollisionBox = (Rect){
        .x1 = duck->collision_box_x1 + movement.x,
        .y1 = duck->collision_box_y1 + movement.y,
        .x2 = duck->collision_box_x2 + movement.x,
        .y2 = duck->collision_box_y2 + movement.y,
    };
    Rect duckPotentialCollisionBoxX = (Rect){
        .x1 = duck->collision_box_x1 + movement.x,
        .y1 = duck->collision_box_y1,
        .x2 = duck->collision_box_x2 + movement.x,
        .y2 = duck->collision_box_y2,
    };
    Rect duckPotentialCollisionBoxY = (Rect){
        .x1 = duck->collision_box_x1,
        .y1 = duck->collision_box_y1 + movement.y,
        .x2 = duck->collision_box_x2,
        .y2 = duck->collision_box_y2 + movement.y,
    };

    // Check each snowman for collision.
    Snowman *currentSnowman = snowmen;
    while (currentSnowman != NULL)
    {
        Rect currentSnowmanCollisionBox = (Rect){.x1 = currentSnowman->collision_box_x1, .y1 = currentSnowman->collision_box_y1, .x2 = currentSnowman->collision_box_x2, .y2 = currentSnowman->collision_box_y2};

        if (detect_collision(duckPotentialCollisionBox, currentSnowmanCollisionBox))
        {
            if (detect_collision(duckPotentialCollisionBoxX, currentSnowmanCollisionBox))
            {
                collision.x = true;
            }

            if (detect_collision(duckPotentialCollisionBoxY, currentSnowmanCollisionBox))
            {
                collision.y = true;
            }
        }

        // Stop checking if we've already collided in both directions.
        if (collision.x && collision.y)
        {
            return collision;
        }

        currentSnowman = currentSnowman->next;
    }

    // Check each duck for collision.
    Duck *currentDuck = ducks;
    while (currentDuck != NULL)
    {
        // Skip the duck we're testing. It will always be colliding with itself.
        if (duck->id == currentDuck->id)
        {
            // Next duck.
            currentDuck = currentDuck->next;

            continue;
        }

        Rect currentDuckCollisionBox = (Rect){.x1 = currentDuck->collision_box_x1, .y1 = currentDuck->collision_box_y1, .x2 = currentDuck->collision_box_x2, .y2 = currentDuck->collision_box_y2};

        if (detect_collision(duckPotentialCollisionBox, currentDuckCollisionBox))
        {
            if (detect_collision(duckPotentialCollisionBoxX, currentDuckCollisionBox))
            {
                collision.x = true;
            }

            if (detect_collision(duckPotentialCollisionBoxY, currentDuckCollisionBox))
            {
                collision.y = true;
            }
        }

        // Stop checking if we've already collided in both directions.
        if (collision.x && collision.y)
        {
            return collision;
        }

        // Next duck.
        currentDuck = currentDuck->next;
    }

    return collision;
}

void process_input_direction(Duck *duck, Controller *controller, joypad_buttons_t pressed, joypad_buttons_t held, joypad_buttons_t released, joypad_8way_t direction, size_t i, float deltatime)
{
    Vector2 movement;
    Collision collision;

    // Movement
    switch (direction)
    {
    case JOYPAD_8WAY_UP:

        // Position
        movement = (Vector2){.x = 0, .y = -1};
        if (held.b)
            movement = (Vector2){.x = 0, .y = -1 * BOOST};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

        // Direction
        duck->direction = RIGHT;

        // Position
        movement = (Vector2){.x = SQRT_ONE_HALF, .y = -SQRT_ONE_HALF};
        if (held.b)
            movement = (Vector2){.x = SQRT_ONE_HALF * BOOST, .y = -SQRT_ONE_HALF * BOOST};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

    case JOYPAD_8WAY_RIGHT:

        // Direction
        duck->direction = RIGHT;

        // Position
        movement = (Vector2){.x = 1, .y = 0};
        if (held.b)
            movement = (Vector2){.x = 1 * BOOST, .y = 0};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

    case JOYPAD_8WAY_DOWN_RIGHT:

        // Direction
        duck->direction = RIGHT;

        // Position
        movement = (Vector2){.x = SQRT_ONE_HALF, .y = SQRT_ONE_HALF};
        if (held.b)
            movement = (Vector2){.x = SQRT_ONE_HALF * BOOST, .y = SQRT_ONE_HALF * BOOST};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

    case JOYPAD_8WAY_DOWN:

        // Position
        movement = (Vector2){.x = 0, .y = 1};
        if (held.b)
            movement = (Vector2){.x = 0, .y = 1 * BOOST};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

        // Direction
        duck->direction = LEFT;

        // Position
        movement = (Vector2){.x = -SQRT_ONE_HALF, .y = SQRT_ONE_HALF};
        if (held.b)
            movement = (Vector2){.x = -SQRT_ONE_HALF * BOOST, .y = SQRT_ONE_HALF * BOOST};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

    case JOYPAD_8WAY_LEFT:

        // Direction
        duck->direction = LEFT;

        // Position
        movement = (Vector2){.x = -1, .y = 0};
        if (held.b)
            movement = (Vector2){.x = -1 * BOOST, .y = 0};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

    case JOYPAD_8WAY_UP_LEFT:

        // Direction
        duck->direction = LEFT;

        // Position
        movement = (Vector2){.x = -SQRT_ONE_HALF, .y = -SQRT_ONE_HALF};
        if (held.b)
            movement = (Vector2){.x = -SQRT_ONE_HALF * BOOST, .y = -SQRT_ONE_HALF * BOOST};

        collision = check_for_duck_collisions(duck, i, movement);

        if (!collision.x)
        {
            duck->x += movement.x;
        }

        if (!collision.y)
        {
            duck->y += movement.y;
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

    default:
        if (duck->locked_for_frames == 0)
            duck->action = DUCK_BASE;
        break;
    }

    if (duck->x > DUCK_MAX_X)
    {
        duck->x = DUCK_MAX_X;
    }

    if (duck->x < DUCK_MIN_X)
    {
        duck->x = DUCK_MIN_X;
    }

    if (duck->y > DUCK_MAX_Y)
    {
        duck->y = DUCK_MAX_Y;
    }

    if (duck->y < DUCK_MIN_Y)
    {
        duck->y = DUCK_MIN_Y;
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

void sequence_game_update(float deltatime)
{
    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        Controller *controller = &controllers[i];
        Duck *duck = get_duck_by_id(i);

        joypad_port_t controllerPort = core_get_playercontroller(i);
        if (!joypad_is_connected(controllerPort))
            continue;

        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);
        joypad_buttons_t held = joypad_get_buttons_held(controllerPort);
        joypad_buttons_t released = joypad_get_buttons_released(controllerPort);
        joypad_8way_t direction = joypad_get_direction(controllerPort, JOYPAD_2D_ANY);

        process_input_start_button(controller, pressed, held, released, i, deltatime);

        if (!sequence_game_paused)
            process_input_direction(duck, controller, pressed, held, released, direction, i, deltatime);
    }

    if (!sequence_game_paused)
    {
        // Set the collision box of the ducks based on their current action and frame.
        update_ducks(deltatime);

        // Set the collision box of the snowmen based on their current action and frame. Also, add snowmen.
        update_snowmen(deltatime);
    }
}