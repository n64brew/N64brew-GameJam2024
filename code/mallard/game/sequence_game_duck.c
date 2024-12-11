#include <libdragon.h>
#include "sequence_game_initialize.h"
#include "sequence_game_input.h"
#include "sequence_game.h"
#include "sequence_game_duck.h"

void display_ducks()
{
    Duck *current = ducks;
    while (current != NULL)
    {
        fprintf(stderr, "[Duck #%i - %f], ", current->id, current->collision_box_y2);
        current = current->next;
    }
    fprintf(stderr, "\n");
}

Vector2 get_duck_spawn(int x1, int y1, int x2, int y2)
{
    int _x, _y;
    float _x1, _y1, _x2, _y2;
    bool _validSpawn;
    Duck *currentDuck;

    while (true)
    {
        _validSpawn = true;
        _x = random_between(x1, x2);
        _y = random_between(y1, y2);
        _x1 = _x + DUCK_COLLISION_BOX_X1_OFFSET;
        _y1 = _y + DUCK_COLLISION_BOX_Y1_OFFSET;
        _x2 = _x + DUCK_COLLISION_BOX_X2_OFFSET;
        _y2 = _y + DUCK_COLLISION_BOX_Y2_OFFSET;

        currentDuck = ducks;
        while (currentDuck != NULL)
        {
            Rect currentDuckCollisionBox = (Rect){.x1 = currentDuck->collision_box_x1, .y1 = currentDuck->collision_box_y1, .x2 = currentDuck->collision_box_x2, .y2 = currentDuck->collision_box_y2};

            if (detect_collision(
                    currentDuckCollisionBox,
                    (Rect){.x1 = _x1, .y1 = _y1, .x2 = _x2, .y2 = _y2}))
            {
                _validSpawn = false;
                break;
            }

            currentDuck = currentDuck->next;
        }

        if (_validSpawn)
        {
            return (Vector2){.x = _x, .y = _y};
        }
    }
}

Duck *create_duck(int i)
{
    Vector2 spawn;
    Duck *duck = (Duck *)malloc(sizeof(Duck));

    switch (i)
    {
    case 0:
        spawn = get_duck_spawn(PLAYER_1_SPAWN_X1, PLAYER_1_SPAWN_Y1, PLAYER_1_SPAWN_X2, PLAYER_1_SPAWN_Y2);
        break;
    case 1:
        spawn = get_duck_spawn(PLAYER_2_SPAWN_X1, PLAYER_2_SPAWN_Y1, PLAYER_2_SPAWN_X2, PLAYER_2_SPAWN_Y2);
        break;
    case 2:
        spawn = get_duck_spawn(PLAYER_3_SPAWN_X1, PLAYER_3_SPAWN_Y1, PLAYER_3_SPAWN_X2, PLAYER_3_SPAWN_Y2);
        break;
    case 3:
        spawn = get_duck_spawn(PLAYER_4_SPAWN_X1, PLAYER_4_SPAWN_Y1, PLAYER_4_SPAWN_X2, PLAYER_4_SPAWN_Y2);
        break;
    default:
        spawn = get_duck_spawn(PLAYER_4_SPAWN_X1, PLAYER_4_SPAWN_Y1, PLAYER_4_SPAWN_X2, PLAYER_4_SPAWN_Y2);
        break;
    }

    duck->id = i;
    duck->x = spawn.x;
    duck->y = spawn.y;
    duck->score = 0.0f;
    duck->action = DUCK_IDLE;
    duck->time_since_last_hit = 0.0f;
    duck->time_seeking_target = 0.0f;
    duck->direction = (i == 0 || i == 2) ? RIGHT : LEFT;
    duck->collision_box_x1 = spawn.x + DUCK_COLLISION_BOX_X1_OFFSET;
    duck->collision_box_y1 = spawn.y + DUCK_COLLISION_BOX_Y1_OFFSET;
    duck->collision_box_x2 = spawn.x + DUCK_COLLISION_BOX_X2_OFFSET;
    duck->collision_box_y2 = spawn.y + DUCK_COLLISION_BOX_Y2_OFFSET;
    duck->slap_box_x1 = spawn.x + (duck->direction == RIGHT ? DUCK_SLAP_BOX_X1_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_X1_OFFSET_FACING_LEFT);
    duck->slap_box_y1 = spawn.y + (duck->direction == RIGHT ? DUCK_SLAP_BOX_Y1_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_Y1_OFFSET_FACING_LEFT);
    duck->slap_box_x2 = spawn.x + (duck->direction == RIGHT ? DUCK_SLAP_BOX_X2_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_X2_OFFSET_FACING_LEFT);
    duck->slap_box_y2 = spawn.y + (duck->direction == RIGHT ? DUCK_SLAP_BOX_Y2_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_Y2_OFFSET_FACING_LEFT);
    duck->hit_box_x1 = spawn.x + (duck->direction == RIGHT ? DUCK_HIT_BOX_X1_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_X1_OFFSET_FACING_LEFT);
    duck->hit_box_y1 = spawn.y + (duck->direction == RIGHT ? DUCK_HIT_BOX_Y1_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_Y1_OFFSET_FACING_LEFT);
    duck->hit_box_x2 = spawn.x + (duck->direction == RIGHT ? DUCK_HIT_BOX_X2_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_X2_OFFSET_FACING_LEFT);
    duck->hit_box_y2 = spawn.y + (duck->direction == RIGHT ? DUCK_HIT_BOX_Y2_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_Y2_OFFSET_FACING_LEFT);
    duck->frames = 0;
    duck->frames_locked_for_slap = 0;
    duck->frames_locked_for_damage = 0;

    switch (i)
    {
    case 0:
        duck->walk_sprite = sequence_game_mallard_one_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_one_slap_sprite;
        duck->run_sprite = sequence_game_mallard_one_run_sprite;
        duck->idle_sprite = sequence_game_mallard_one_idle_sprite;
        duck->damage_sprite = sequence_game_mallard_one_damage_sprite;
        break;
    case 1:
        duck->walk_sprite = sequence_game_mallard_two_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_two_slap_sprite;
        duck->run_sprite = sequence_game_mallard_two_run_sprite;
        duck->idle_sprite = sequence_game_mallard_two_idle_sprite;
        duck->damage_sprite = sequence_game_mallard_two_damage_sprite;
        break;
    case 2:
        duck->walk_sprite = sequence_game_mallard_three_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_three_slap_sprite;
        duck->run_sprite = sequence_game_mallard_three_run_sprite;
        duck->idle_sprite = sequence_game_mallard_three_idle_sprite;
        duck->damage_sprite = sequence_game_mallard_three_damage_sprite;
        break;
    case 3:
        duck->walk_sprite = sequence_game_mallard_four_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_four_slap_sprite;
        duck->run_sprite = sequence_game_mallard_four_run_sprite;
        duck->idle_sprite = sequence_game_mallard_four_idle_sprite;
        duck->damage_sprite = sequence_game_mallard_four_damage_sprite;
        break;
    default:
        break;
    }

    return duck;
}

Duck *get_duck_by_id(int i)
{
    Duck *current = ducks;
    while (current != NULL)
    {
        if (current->id == i)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void add_duck(int i)
{
    Duck *duck = create_duck(i);

    // Insert at the head if the list is empty.
    // Insert at the head if the new value is smaller.
    if (ducks == NULL || ducks->y >= duck->y)
    {
        duck->next = ducks;
        ducks = duck;
        return;
    }

    // Otherwise, Traverse the ducks to find the correct insertion point.
    Duck *currentDuck = ducks;
    while (currentDuck->next != NULL && currentDuck->next->y < duck->y)
    {
        currentDuck = currentDuck->next;
    }

    // Insert the new duck
    duck->next = currentDuck->next;
    currentDuck->next = duck;
}

void initialize_ducks()
{
    if (ducks == NULL)
    {
        for (size_t i = 0; i < 4; i++)
        {
            add_duck(i);
        }
    }
}

void free_ducks()
{
    Duck *temporary;
    while (ducks != NULL)
    {
        temporary = ducks;
        ducks = ducks->next;
        free(temporary);
    }
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

// Update each duck's frame, collision box, and slap box.
void update_ducks(float deltatime)
{
    if( time_elapsed >= GAME_FADE_IN_DURATION + 3 + GAME_DURATION)
    {
        return;
    }
    
    if (!sequence_game_paused)
    {
        Duck *currentDuck = ducks;
        while (currentDuck != NULL)
        {
            currentDuck->frames++;
            currentDuck->time_since_last_hit += deltatime;
            currentDuck->time_seeking_target += deltatime;

            currentDuck->collision_box_x1 = currentDuck->x + DUCK_COLLISION_BOX_X1_OFFSET;
            currentDuck->collision_box_y1 = currentDuck->y + DUCK_COLLISION_BOX_Y1_OFFSET;
            currentDuck->collision_box_x2 = currentDuck->x + DUCK_COLLISION_BOX_X2_OFFSET;
            currentDuck->collision_box_y2 = currentDuck->y + DUCK_COLLISION_BOX_Y2_OFFSET;

            currentDuck->hit_box_x1 = currentDuck->x + (currentDuck->direction == RIGHT ? DUCK_HIT_BOX_X1_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_X1_OFFSET_FACING_LEFT);
            currentDuck->hit_box_y1 = currentDuck->y + (currentDuck->direction == RIGHT ? DUCK_HIT_BOX_Y1_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_Y1_OFFSET_FACING_LEFT);
            currentDuck->hit_box_x2 = currentDuck->x + (currentDuck->direction == RIGHT ? DUCK_HIT_BOX_X2_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_X2_OFFSET_FACING_LEFT);
            currentDuck->hit_box_y2 = currentDuck->y + (currentDuck->direction == RIGHT ? DUCK_HIT_BOX_Y2_OFFSET_FACING_RIGHT : DUCK_HIT_BOX_Y2_OFFSET_FACING_LEFT);

            currentDuck->slap_box_x1 = currentDuck->x + (currentDuck->direction == RIGHT ? DUCK_SLAP_BOX_X1_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_X1_OFFSET_FACING_LEFT);
            currentDuck->slap_box_y1 = currentDuck->y + (currentDuck->direction == RIGHT ? DUCK_SLAP_BOX_Y1_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_Y1_OFFSET_FACING_LEFT);
            currentDuck->slap_box_x2 = currentDuck->x + (currentDuck->direction == RIGHT ? DUCK_SLAP_BOX_X2_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_X2_OFFSET_FACING_LEFT);
            currentDuck->slap_box_y2 = currentDuck->y + (currentDuck->direction == RIGHT ? DUCK_SLAP_BOX_Y2_OFFSET_FACING_RIGHT : DUCK_SLAP_BOX_Y2_OFFSET_FACING_LEFT);

            currentDuck = currentDuck->next;
        }

        ducks_bubble_sort();
    }
}