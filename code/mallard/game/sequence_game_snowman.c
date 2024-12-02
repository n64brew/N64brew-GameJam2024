#include <libdragon.h>
#include "sequence_game_initialize.h"
#include "sequence_game_snowman.h"
#include "sequence_game_input.h"

void display_snowmen()
{
    Snowman *current = snowmen;
    while (current != NULL)
    {
        fprintf(stderr, "(%f ,%f)\n", current->x, current->y);
        current = current->next;
    }
    fprintf(stderr, "NULL\n");
}

Vector2 get_snowman_spawn()
{
    int x, y;
    float x1, y1, x2, y2;

    int attempts = 0;
    while (attempts < 20)
    {
        x = random_between(SNOWMAN_MIN_X, SNOWMAN_MAX_X);
        y = random_between(SNOWMAN_MIN_Y, SNOWMAN_MAX_Y);
        x1 = x;
        y1 = y + 8;
        x2 = x + 12;
        y2 = y + 16;

        if (!detect_collision((Rect){.x1 = ducks[0].collision_box_x1, .y1 = ducks[0].collision_box_y1, .x2 = ducks[0].collision_box_x2, .y2 = ducks[0].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}) &&
            !detect_collision((Rect){.x1 = ducks[1].collision_box_x1, .y1 = ducks[1].collision_box_y1, .x2 = ducks[1].collision_box_x2, .y2 = ducks[1].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}) &&
            !detect_collision((Rect){.x1 = ducks[2].collision_box_x1, .y1 = ducks[2].collision_box_y1, .x2 = ducks[2].collision_box_x2, .y2 = ducks[2].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}) &&
            !detect_collision((Rect){.x1 = ducks[3].collision_box_x1, .y1 = ducks[3].collision_box_y1, .x2 = ducks[3].collision_box_x2, .y2 = ducks[3].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}))
            return (Vector2){.x = x, .y = y};

        attempts++;
    }

    return (Vector2){.x = -1.0F, .y = -1.0F};
}

Snowman *create_snowman()
{
    Snowman *snowman = (Snowman *)malloc(sizeof(Snowman));
    Vector2 spawn = get_snowman_spawn();
    snowman->x = spawn.x;
    snowman->y = spawn.y;
    snowman->collision_box_x1 = spawn.x;
    snowman->collision_box_y1 = spawn.y + 8;
    snowman->collision_box_x2 = spawn.x + 12;
    snowman->collision_box_y2 = spawn.y + 16;
    snowman->action = SNOWMAN_IDLE;
    snowman->frames = 0;
    snowman->locked_for_frames = 0;
    snowman->time = 0.0f;
    snowman->idle_sprite = sequence_game_snowman_idle_sprite;
    snowman->jump_sprite = sequence_game_snowman_jump_sprite;
    return snowman;
}

void add_snowman()
{
    Snowman *snowman = create_snowman();

    if (snowman->x == -1.0F && snowman->y == -1.0F)
    {
        fprintf(stderr, "Failed to spawn snowman\n");
        free(snowman);
        return;
    }

    // Insert at the head if the list is empty or the new value is smaller
    if (snowmen == NULL || snowmen->y >= snowman->y)
    {
        snowman->next = snowmen;
        snowmen = snowman;
        return;
    }

    // Traverse to find the insertion point
    Snowman *current = snowmen;
    while (current->next != NULL && current->next->y < snowman->y)
    {
        current = current->next;
    }

    // Insert the new snowman
    snowman->next = current->next;
    current->next = snowman;

    // display_snowmen();
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