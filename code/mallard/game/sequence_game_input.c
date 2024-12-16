#include <libdragon.h>
#include "../../../core.h"
#include "../../../minigame.h"
#include "sequence_game.h"
#include "sequence_game_input.h"
#include "sequence_game_initialize.h"
#include "sequence_game_graphics.h"
#include "sequence_game_snowman.h"
#include "sequence_game_duck.h"

#define BOOST 2.0

float time_elapsed_since_last_snowman_spawn = 0.0f;
float time_elapsed = 0.0f;
int winner = -1;

bool countdown_one_played = false;
bool countdown_two_played = false;
bool countdown_three_played = false;
bool start_played = false;
bool stop_played = false;
bool winner_played = false;

void duck_slap(Duck *duck)
{
    duck->frames_locked_for_slap = 4 * SEQUENCE_GAME_MALLARD_SLAP_FRAMES; // Lock for 12 frames.
    duck->action = DUCK_SLAP;
    duck->frames = 0;
}

void snowmen_bubble_sort()
{
    bool swapped = true;

    while (swapped)
    {
        Snowman **prev = &snowmen;
        Snowman *curr;
        Snowman *next;

        swapped = false;
        for (curr = snowmen; curr; prev = &curr->next, curr = curr->next)
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

void remove_snowman(int id)
{
    // Remove snowman from the snowmen linked list
    Snowman *currentSnowman = snowmen;
    Snowman *previousSnowman = NULL;
    while (currentSnowman != NULL)
    {
        if (currentSnowman->id == id)
        {
            if (previousSnowman == NULL)
            {
                snowmen = currentSnowman->next;
            }
            else
            {
                previousSnowman->next = currentSnowman->next;
            }

            free(currentSnowman);
            break;
        }

        previousSnowman = currentSnowman;
        currentSnowman = currentSnowman->next;
    }
}

void update_snowmen(float deltatime)
{
    if (time_elapsed >= GAME_FADE_IN_DURATION + 3 + GAME_DURATION)
    {
        return;
    }

    if (!sequence_game_paused)
    {
        float SNOWMAN_SPAWN_FREQUENCY;
        if (time_elapsed >= 0.0f && time_elapsed < 15.0f)
            SNOWMAN_SPAWN_FREQUENCY = 1.5f;
        else if (time_elapsed >= 15.0f && time_elapsed < 30.0f)
            SNOWMAN_SPAWN_FREQUENCY = 1.0f;
        else if (time_elapsed >= 30.0f && time_elapsed < 45.0f)
            SNOWMAN_SPAWN_FREQUENCY = 0.75f;
        else
            SNOWMAN_SPAWN_FREQUENCY = 0.5f;

        // Update snowmen.
        Snowman *currentSnowman = snowmen;
        while (currentSnowman != NULL)
        {
            currentSnowman->frames++;
            currentSnowman->time_since_last_hit += deltatime;

            currentSnowman->collision_box_x1 = currentSnowman->x + SNOWMAN_COLLISION_BOX_X1_OFFSET;
            currentSnowman->collision_box_y1 = currentSnowman->y + SNOWMAN_COLLISION_BOX_Y1_OFFSET;
            currentSnowman->collision_box_x2 = currentSnowman->x + SNOWMAN_COLLISION_BOX_X2_OFFSET;
            currentSnowman->collision_box_y2 = currentSnowman->y + SNOWMAN_COLLISION_BOX_Y2_OFFSET;

            currentSnowman->hit_box_x1 = currentSnowman->x + SNOWMAN_HIT_BOX_X1_OFFSET;
            currentSnowman->hit_box_y1 = currentSnowman->y + SNOWMAN_HIT_BOX_Y1_OFFSET;
            currentSnowman->hit_box_x2 = currentSnowman->x + SNOWMAN_HIT_BOX_X2_OFFSET;
            currentSnowman->hit_box_y2 = currentSnowman->y + SNOWMAN_HIT_BOX_Y2_OFFSET;

            if (currentSnowman->frames_locked_for_damage > 0)
            {
                currentSnowman->frames_locked_for_damage--;
            }

            if (currentSnowman->frames_locked_for_damage == 0)
            {
                currentSnowman->action = SNOWMAN_IDLE;

                if (currentSnowman->health <= 0)
                {
                    Snowman *temporary = currentSnowman;
                    currentSnowman = currentSnowman->next;
                    remove_snowman(temporary->id);
                    continue;
                }
            }

            currentSnowman = currentSnowman->next;
        }

        // Add snowman.
        if (time_elapsed_since_last_snowman_spawn >= SNOWMAN_SPAWN_FREQUENCY)
        {
            add_snowman();
            time_elapsed_since_last_snowman_spawn = 0.0f;
        }

        snowmen_bubble_sort();

        if (time_elapsed > GAME_FADE_IN_DURATION + 3)
        {
            time_elapsed_since_last_snowman_spawn += deltatime;
        }
    }
}

typedef struct ValidMovement
{
    bool x;
    bool y;
} ValidMovement;

bool detect_collision(Rect a, Rect b)
{
    return a.x1 < b.x2 && a.x2 > b.x1 && a.y1 < b.y2 && a.y2 > b.y1;
}

ValidMovement validate_movement(Duck *duck, Vector2 movement)
{
    ValidMovement validMovement = (ValidMovement){.x = false, .y = false};

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
                validMovement.x = true;
            }

            if (detect_collision(duckPotentialCollisionBoxY, currentSnowmanCollisionBox))
            {
                validMovement.y = true;
            }
        }

        // Stop checking if we've already collided in both directions.
        if (validMovement.x && validMovement.y)
        {
            return validMovement;
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
                validMovement.x = true;
            }

            if (detect_collision(duckPotentialCollisionBoxY, currentDuckCollisionBox))
            {
                validMovement.y = true;
            }
        }

        // Stop checking if we've already collided in both directions.
        if (validMovement.x && validMovement.y)
        {
            return validMovement;
        }

        // Next duck.
        currentDuck = currentDuck->next;
    }

    return validMovement;
}

void process_input(Duck *duck, Controller *controller, joypad_buttons_t pressed, joypad_buttons_t held, joypad_buttons_t released, joypad_8way_t direction, size_t i, float deltatime)
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

    // If the game is paused, don't process input.
    if (!sequence_game_paused)
    {
        Vector2 movement;
        ValidMovement validMovement;

        // Stun Lock.
        if (duck->frames_locked_for_damage > 0)
        {
            duck->frames_locked_for_damage--;
            return;
        }

        // Movement
        switch (direction)
        {
        case JOYPAD_8WAY_UP:

            // Position
            movement = (Vector2){.x = 0, .y = -1};
            if (held.b)
                movement = (Vector2){.x = 0, .y = -1 * BOOST};

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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
            movement = (Vector2){.x = 1, .y = -1};
            if (held.b)
                movement = (Vector2){.x = 1 * BOOST, .y = -1 * BOOST};

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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
            movement = (Vector2){.x = 1, .y = 1};
            if (held.b)
                movement = (Vector2){.x = 1 * BOOST, .y = 1 * BOOST};

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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
            movement = (Vector2){.x = -1, .y = 1};
            if (held.b)
                movement = (Vector2){.x = -1 * BOOST, .y = 1 * BOOST};

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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
            movement = (Vector2){.x = -1, .y = -1};
            if (held.b)
                movement = (Vector2){.x = -1 * BOOST, .y = -1 * BOOST};

            validMovement = validate_movement(duck, movement);

            if (!validMovement.x)
            {
                duck->x += movement.x;
            }

            if (!validMovement.y)
            {
                duck->y += movement.y;
            }

            // Action
            if (duck->frames_locked_for_slap == 0)
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
            if (duck->frames_locked_for_slap == 0)
                duck->action = DUCK_IDLE;
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
            duck_slap(duck);
        }

        if (duck->frames_locked_for_slap > 0)
        {
            duck->frames_locked_for_slap--;
        }
    }
}

Snowman *find_nearest_snowman(Duck *duck)
{
    Snowman *nearestSnowman = NULL;
    float nearestDistance = 999999.0f;

    float duck_x = (duck->slap_box_x1 + duck->slap_box_x2) / 2;
    float duck_y = (duck->slap_box_y1 + duck->slap_box_y2) / 2;

    Snowman *currentSnowman = snowmen;
    while (currentSnowman != NULL)
    {
        float snowman_x = (currentSnowman->hit_box_x1 + currentSnowman->hit_box_x2) / 2;
        float snowman_y = (currentSnowman->hit_box_y1 + currentSnowman->hit_box_y2) / 2;
        float distance = fmax(abs(duck_x - snowman_x), abs(duck_y - snowman_y));
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestSnowman = currentSnowman;
        }

        currentSnowman = currentSnowman->next;
    }

    return nearestSnowman;
}

void set_duck_direction(Duck *duck, Snowman *snowman)
{
    if (snowman == NULL)
    {
        return;
    }

    float snowman_x = (snowman->hit_box_x1 + snowman->hit_box_x2) / 2;
    float duck_x = (duck->slap_box_x1 + duck->slap_box_x2) / 2;

    if (duck_x <= snowman_x)
    {
        duck->direction = RIGHT;
    }
    else
    {
        duck->direction = LEFT;
    }
}
void set_duck_movement(Duck *duck, Snowman *snowman)
{
    if (snowman == NULL)
    {
        return;
    }

    Vector2 movement;
    ValidMovement validMovement;
    float snowman_x = (snowman->hit_box_x1 + snowman->hit_box_x2) / 2;
    float snowman_y = (snowman->hit_box_y1 + snowman->hit_box_y2) / 2;
    float duck_x = (duck->slap_box_x1 + duck->slap_box_x2) / 2;
    float duck_y = (duck->slap_box_y1 + duck->slap_box_y2) / 2;
    float dx = snowman_x - duck_x;
    float dy = snowman_y - duck_y;

    float speed;
    switch (difficulty)
    {
    case DIFF_EASY:
        speed = 0.75;
        break;
    case DIFF_MEDIUM:
        speed = 1.25;
        break;
    case DIFF_HARD:
        speed = 1.5;
        break;
    default:
        speed = 1.0;
        break;
    }

    if (dx > 1 && dy > 1)
    {
        movement = (Vector2){.x = 1 * speed, .y = 1 * speed};
    }
    else if (dx > 1 && dy < -1)
    {
        movement = (Vector2){.x = 1 * speed, .y = -1 * speed};
    }
    else if (dx < -1 && dy > 1)
    {
        movement = (Vector2){.x = -1 * speed, .y = 1 * speed};
    }
    else if (dx < -1 && dy < -1)
    {
        movement = (Vector2){.x = -1 * speed, .y = -1 * speed};
    }
    else if (dx > 1)
    {
        movement = (Vector2){.x = 1 * speed, .y = 0};
    }
    else if (dx < -1)
    {
        movement = (Vector2){.x = -1 * speed, .y = 0};
    }
    else if (dy > 1)
    {
        movement = (Vector2){.x = 0, .y = 1 * speed};
    }
    else if (dy < -1)
    {
        movement = (Vector2){.x = 0, .y = -1 * speed};
    }

    validMovement = validate_movement(duck, movement);

    if (!validMovement.x)
    {
        duck->x += movement.x;
    }

    if (!validMovement.y)
    {
        duck->y += movement.y;
    }
}

void set_duck_action(Duck *duck, Snowman *snowman)
{
    if (duck->frames_locked_for_slap != 0)
    {
        duck->action = DUCK_SLAP;
        return;
    }

    if (snowman == NULL)
    {
        duck->action = DUCK_IDLE;
        return;
    }

    Rect duckSlapBox = (Rect){.x1 = duck->slap_box_x1, .y1 = duck->slap_box_y1, .x2 = duck->slap_box_x2, .y2 = duck->slap_box_y2};
    Rect snowmanHitBox = (Rect){.x1 = snowman->hit_box_x1, .y1 = snowman->hit_box_y1, .x2 = snowman->hit_box_x2, .y2 = snowman->hit_box_y2};

    if (detect_collision(duckSlapBox, snowmanHitBox))
    {
        duck_slap(duck);
    }
    else
    {
        duck->action = DUCK_WALK;
    }
}

void simulate_input(Duck *duck, float deltatime)
{
    if (!sequence_game_paused)
    {
        if (duck->frames_locked_for_damage > 0)
        {
            duck->frames_locked_for_damage--;
            return;
        }

        float seeking_delay;
        switch (difficulty)
        {
        case DIFF_EASY:
            seeking_delay = DUCK_TIME_SEEKING_TARGET_EASY;
            break;
        case DIFF_MEDIUM:
            seeking_delay = DUCK_TIME_SEEKING_TARGET_MEDIUM;
            break;
        case DIFF_HARD:
            seeking_delay = DUCK_TIME_SEEKING_TARGET_HARD;
            break;
        default:
            seeking_delay = DUCK_TIME_SEEKING_TARGET_MEDIUM;
            break;
        }

        if (duck->time_seeking_target < seeking_delay)
        {
            return;
        }

        Snowman *nearestSnowman = find_nearest_snowman(duck);

        set_duck_direction(duck, nearestSnowman);

        set_duck_movement(duck, nearestSnowman);

        set_duck_action(duck, nearestSnowman);

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

        if (duck->frames_locked_for_slap > 0)
        {
            duck->frames_locked_for_slap--;
        }
    }
}

void evaluate_attack()
{
    if (time_elapsed >= GAME_FADE_IN_DURATION + 3 + GAME_DURATION)
    {
        return;
    }

    if (!sequence_game_paused)
    {
        Duck *currentDuck = ducks;

        while (currentDuck != NULL)
        {
            if (currentDuck->action == DUCK_SLAP)
            {
                Rect currentDuckSlapBox = (Rect){.x1 = currentDuck->slap_box_x1, .y1 = currentDuck->slap_box_y1, .x2 = currentDuck->slap_box_x2, .y2 = currentDuck->slap_box_y2};

                Snowman *currentSnowman = snowmen;
                while (currentSnowman != NULL)
                {
                    Rect currentSnowmanHitBox = (Rect){.x1 = currentSnowman->hit_box_x1, .y1 = currentSnowman->hit_box_y1, .x2 = currentSnowman->hit_box_x2, .y2 = currentSnowman->hit_box_y2};

                    if (detect_collision(currentDuckSlapBox, currentSnowmanHitBox))
                    {
                        if (currentSnowman->time_since_last_hit > SNOWMAN_TIME_BETWEEN_DAMAGE)
                        {
                            // Set action to damage.
                            currentSnowman->action = SNOWMAN_DAMAGE;
                            currentSnowman->frames = 0;
                            currentSnowman->frames_locked_for_damage = 4 * SEQUENCE_GAME_SNOWMAN_DAMAGE_FRAMES;

                            // Reset time since last hit.
                            currentSnowman->time_since_last_hit = 0.0f;

                            // TODO: Sound Effect.

                            // Damage Snowman. If health is 0, remove snowman and reward duck.
                            currentSnowman->health -= 1;

                            // Evaluate if snowman is dead.
                            if (currentSnowman->health <= 0)
                            {
                                // Reward Duck
                                currentDuck->score += 1;
                                currentDuck->time_seeking_target = 0.0f;
                            }

                            // Next Snowman.
                            currentSnowman = currentSnowman->next;
                        }
                        else
                        {
                            // Next Snowman.
                            currentSnowman = currentSnowman->next;
                        }
                    }
                    else
                    {
                        // Next Snowman.
                        currentSnowman = currentSnowman->next;
                    }
                }

                Duck *temporaryDuck = ducks;
                while (temporaryDuck != NULL)
                {
                    if (currentDuck->id == temporaryDuck->id)
                    {
                        temporaryDuck = temporaryDuck->next;
                        continue;
                    }

                    Rect temporaryDuckHitBox = (Rect){.x1 = temporaryDuck->hit_box_x1, .y1 = temporaryDuck->hit_box_y1, .x2 = temporaryDuck->hit_box_x2, .y2 = temporaryDuck->hit_box_y2};

                    if (detect_collision(currentDuckSlapBox, temporaryDuckHitBox))
                    {
                        // TODO: Sound Effect.

                        if (temporaryDuck->time_since_last_hit > DUCK_TIME_BETWEEN_DAMAGE)
                        {
                            // Set action to damage.
                            temporaryDuck->action = DUCK_DAMAGE;
                            temporaryDuck->frames = 0;
                            temporaryDuck->frames_locked_for_damage = 4 * SEQUENCE_GAME_MALLARD_DAMAGE_FRAMES;

                            // Reset time since last hit.
                            temporaryDuck->time_since_last_hit = 0.0f;

                            // Reward Duck
                            currentDuck->score += 0.1f;

                            // Damage Duck
                            temporaryDuck->score -= 0.1f;
                            if (temporaryDuck->score < 0.0f)
                            {
                                temporaryDuck->score = 0.0f;
                            }
                        }
                    }
                    temporaryDuck = temporaryDuck->next;
                }
            }
            currentDuck = currentDuck->next;
        }
    }
}

void play_sounds()
{
    if (!sequence_game_paused)
    {
        if (GAME_FADE_IN_DURATION < time_elapsed && countdown_one_played == false)
        {
            if (countdown_one_played == false)
            {
                wav64_play(&sfx_countdown, 31);
                countdown_one_played = true;
            }
            return;
        }

        if (GAME_FADE_IN_DURATION + 1 < time_elapsed && countdown_two_played == false)
        {
            if (countdown_two_played == false)
            {
                wav64_play(&sfx_countdown, 31);
                countdown_two_played = true;
            }
            return;
        }

        if (GAME_FADE_IN_DURATION + 2 < time_elapsed && countdown_three_played == false)
        {
            if (countdown_three_played == false)
            {
                wav64_play(&sfx_countdown, 31);
                countdown_three_played = true;
            }
            return;
        }

        if (GAME_FADE_IN_DURATION + 3 < time_elapsed && start_played == false)
        {
            if (start_played == false)
            {
                wav64_play(&sfx_start, 31);
                start_played = true;
            }
            return;
        }
    }
}

void update_time(float deltatime)
{
    if (!sequence_game_paused)
    {
        time_elapsed += deltatime;
    }
}

void process_human_players(float deltatime)
{
    if (time_elapsed >= GAME_FADE_IN_DURATION + 3 + GAME_DURATION)
    {
        return;
    }

    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        Duck *duck = get_duck_by_id(i);
        Controller *controller = &controllers[i];

        joypad_port_t controllerPort = core_get_playercontroller(i);
        if (!joypad_is_connected(controllerPort))
            continue;

        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);
        joypad_buttons_t held = joypad_get_buttons_held(controllerPort);
        joypad_buttons_t released = joypad_get_buttons_released(controllerPort);
        joypad_8way_t direction = joypad_get_direction(controllerPort, JOYPAD_2D_ANY);

        process_input(duck, controller, pressed, held, released, direction, i, deltatime);
    }
}

void process_computer_players(float deltatime)
{
    if (time_elapsed >= GAME_FADE_IN_DURATION + 3 + GAME_DURATION)
    {
        return;
    }

    for (size_t i = core_get_playercount(); i < 4; i++)
    {
        Duck *duck = get_duck_by_id(i);
        simulate_input(duck, deltatime);
    }
}

void update_winner()
{
    if (time_elapsed > GAME_FADE_IN_DURATION + 3 + GAME_DURATION && winner == -1 && stop_played == false)
    {
        Duck *currentDuck = ducks;
        Duck *highestDuck = currentDuck;
        bool draw = false;
        fprintf(stderr, "Evaluating winner\n");
        while (currentDuck != NULL)
        {
            if (currentDuck->id == highestDuck->id)
            {
                currentDuck = currentDuck->next;
                continue;
            }

            if ((int)roundf(currentDuck->score) == (int)roundf(highestDuck->score))
            {
                draw = true;
            }

            if ((int)roundf(currentDuck->score) > (int)roundf(highestDuck->score))
            {
                highestDuck = currentDuck;
                draw = false;
            }

            currentDuck = currentDuck->next;
        }

        if (!draw)
        {
            winner = highestDuck->id;
        }

        wav64_play(&sfx_stop, 31);
        stop_played = true;
    }

    if (time_elapsed > GAME_FADE_IN_DURATION + 3 + GAME_DURATION + 1)
    {
        xm64player_stop(&sequence_game_xm);
    }

    if (time_elapsed > GAME_FADE_IN_DURATION + 3 + GAME_DURATION + 2 && winner_played == false)
    {
        wav64_play(&sfx_winner, 31);
        winner_played = true;
    }

    if (time_elapsed > GAME_FADE_IN_DURATION + 3 + GAME_DURATION + 5)
    {
        sequence_game_should_cleanup = true;
    }
}

void sequence_game_update(float deltatime)
{
    process_human_players(deltatime);
    process_computer_players(deltatime);
    update_time(deltatime);
    update_winner();
    update_ducks(deltatime);
    update_snowmen(deltatime);
    play_sounds();
    evaluate_attack();
}