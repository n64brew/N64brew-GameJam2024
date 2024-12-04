#include <libdragon.h>
#include "../../../core.h"
#include "sequence_game_initialize.h"
#include "sequence_game.h"
#include "sequence_game_input.h"

int random_between(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void display_ducks()
{
    Duck *current = ducks;
    while (current != NULL)
    {
        fprintf(stderr, "[Duck #%i]:%f, ", current->id, current->y);
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
        _x1 = _x + 8;
        _y1 = _y + 16;
        _x2 = _x + 24;
        _y2 = _y + 24;

        currentDuck = ducks;
        while (currentDuck != NULL)
        {
            if (detect_collision(
                    (Rect){.x1 = currentDuck->collision_box_x1, .y1 = currentDuck->collision_box_y1, .x2 = currentDuck->collision_box_x2, .y2 = currentDuck->collision_box_y2},
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
    duck->collision_box_x1 = spawn.x + 8;
    duck->collision_box_y1 = spawn.y + 16;
    duck->collision_box_x2 = spawn.x + 24;
    duck->collision_box_y2 = spawn.y + 24;
    duck->action = DUCK_BASE;
    duck->frames = 0;
    duck->locked_for_frames = 0;
    duck->direction = (i == 0 || i == 2) ? RIGHT : LEFT;

    switch (i)
    {
    case 0:
        duck->base_sprite = sequence_game_mallard_one_base_sprite;
        duck->walk_sprite = sequence_game_mallard_one_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_one_slap_sprite;
        duck->run_sprite = sequence_game_mallard_one_run_sprite;
        break;
    case 1:
        duck->base_sprite = sequence_game_mallard_two_base_sprite;
        duck->walk_sprite = sequence_game_mallard_two_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_two_slap_sprite;
        duck->run_sprite = sequence_game_mallard_two_run_sprite;
        break;
    case 2:
        duck->base_sprite = sequence_game_mallard_three_base_sprite;
        duck->walk_sprite = sequence_game_mallard_three_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_three_slap_sprite;
        duck->run_sprite = sequence_game_mallard_three_run_sprite;
        break;
    case 3:
        duck->base_sprite = sequence_game_mallard_four_base_sprite;
        duck->walk_sprite = sequence_game_mallard_four_walk_sprite;
        duck->slap_sprite = sequence_game_mallard_four_slap_sprite;
        duck->run_sprite = sequence_game_mallard_four_run_sprite;
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
    if (ducks == NULL)
    {
        duck->next = NULL;
        ducks = duck;
        return;
    }

    // Insert at the head if the new value is smaller.
    if (ducks->y >= duck->y)
    {
        duck->next = ducks;
        ducks = duck;
        return;
    }

    // Traverse to find the insertion point
    Duck *current = ducks;
    while (current->next != NULL && current->next->y < duck->y)
    {
        current = current->next;
    }

    // Insert the new duck
    duck->next = current->next;
    current->next = duck;
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

void initialize_controllers()
{
    if (controllers == NULL)
    {
        int playercount = core_get_playercount();
        controllers = malloc(playercount * sizeof(struct Controller));
        for (size_t i = 0; i < playercount; i++)
        {
            controllers[i].start_down = 0;
            controllers[i].start_up = 0;
            controllers[i].start_held_elapsed = 0.0f;
        }
    }
}

void free_controllers()
{
    if (controllers != NULL)
    {
        free(controllers);
        controllers = NULL;
    }
}