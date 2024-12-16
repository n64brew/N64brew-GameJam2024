#include <libdragon.h>
#include "sequence_game_initialize.h"
#include "sequence_game_snowman.h"
#include "sequence_game_input.h"

#define MAX_SNOWMEN 100
int snowman_uuid = 0;

int count_snowmen()
{
    int count = 0;
    Snowman *current = snowmen;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }
    return count;
}

void display_snowmen()
{
    Snowman *current = snowmen;
    while (current != NULL)
    {
        fprintf(stderr, "[Snowman #%i - %f], ", current->id, current->collision_box_y2);
        current = current->next;
    }
    fprintf(stderr, "\n");
}

Vector2 get_snowman_spawn()
{
    int _x, _y;
    float _x1, _y1, _x2, _y2;
    bool _validSpawn;
    Duck *currentDuck;
    while (true)
    {
        _validSpawn = true;
        _x = random_between(SNOWMAN_MIN_X, SNOWMAN_MAX_X);
        _y = random_between(SNOWMAN_MIN_Y, SNOWMAN_MAX_Y);
        _x1 = _x;
        _y1 = _y + 8;
        _x2 = _x + 12;
        _y2 = _y + 16;

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

Snowman *create_snowman()
{
    Snowman *snowman = (Snowman *)malloc(sizeof(Snowman));
    Vector2 spawn = get_snowman_spawn();
    snowman->id = snowman_uuid;
    snowman->x = spawn.x;
    snowman->y = spawn.y;
    snowman->time_since_last_hit = 0.0f;
    snowman->health = 2;
    snowman->action = SNOWMAN_IDLE;
    snowman->idle_sprite = sequence_game_snowman_idle_sprite;
    snowman->damage_sprite = sequence_game_snowman_damage_sprite;
    snowman->jump_sprite = sequence_game_snowman_jump_sprite;
    snowman->frames = 0;
    snowman->frames_locked_for_damage = 0;
    snowman->collision_box_x1 = spawn.x + SNOWMAN_COLLISION_BOX_X1_OFFSET;
    snowman->collision_box_y1 = spawn.y + SNOWMAN_COLLISION_BOX_Y1_OFFSET;
    snowman->collision_box_x2 = spawn.x + SNOWMAN_COLLISION_BOX_X2_OFFSET;
    snowman->collision_box_y2 = spawn.y + SNOWMAN_COLLISION_BOX_Y2_OFFSET;
    snowman->hit_box_x1 = spawn.x + SNOWMAN_HIT_BOX_X1_OFFSET;
    snowman->hit_box_y1 = spawn.y + SNOWMAN_HIT_BOX_Y1_OFFSET;
    snowman->hit_box_x2 = spawn.x + SNOWMAN_HIT_BOX_X2_OFFSET;
    snowman->hit_box_y2 = spawn.y + SNOWMAN_HIT_BOX_Y2_OFFSET;
    snowman_uuid++;
    return snowman;
}

void add_snowman()
{
    if (count_snowmen() >= MAX_SNOWMEN)
    {
        return;
    }

    Snowman *snowman = create_snowman();

    // Insert at the head if the list is empty.
    // Insert at the head if the new value is smaller.
    if (snowmen == NULL || snowmen->y >= snowman->y)
    {
        snowman->next = snowmen;
        snowmen = snowman;
        return;
    }

    // Otherwise, Traverse the snowmen to find the correct insertion point.
    Snowman *current = snowmen;
    while (current->next != NULL && current->next->y < snowman->y)
    {
        current = current->next;
    }

    // Insert the new snowman
    snowman->next = current->next;
    current->next = snowman;
}

void free_snowmen()
{
    Snowman *temporary;
    while (snowmen != NULL)
    {
        temporary = snowmen;
        snowmen = snowmen->next;
        free(temporary);
    }
}