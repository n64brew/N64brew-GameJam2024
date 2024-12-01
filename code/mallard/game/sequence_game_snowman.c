#include <libdragon.h>
#include "sequence_game_initialize.h"
#include "sequence_game_snowman.h"
#include "sequence_game_input.h"

Snowman *add_snowman(Snowman *snowmen)
{
    bool validSpawn = false;
    int x, y;
    float x1, y1, x2, y2;

    Snowman *snowman = (Snowman *)malloc(sizeof(Snowman));
    snowman->action = SNOWMAN_IDLE;
    snowman->frames = 0;
    snowman->locked_for_frames = 0;

    while (validSpawn == false)
    {
        x = random_between(MIN_X, MAX_X);
        y = random_between(MIN_Y, MAX_Y);
        x1 = x + 1;
        y1 = y + 1;
        x2 = x + 12;
        y2 = y + 16;

        if (!detect_collision((Rect){.x1 = ducks[0].collision_box_x1, .y1 = ducks[0].collision_box_y1, .x2 = ducks[0].collision_box_x2, .y2 = ducks[0].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}) &&
            !detect_collision((Rect){.x1 = ducks[1].collision_box_x1, .y1 = ducks[1].collision_box_y1, .x2 = ducks[1].collision_box_x2, .y2 = ducks[1].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}) &&
            !detect_collision((Rect){.x1 = ducks[2].collision_box_x1, .y1 = ducks[2].collision_box_y1, .x2 = ducks[2].collision_box_x2, .y2 = ducks[2].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}) &&
            !detect_collision((Rect){.x1 = ducks[3].collision_box_x1, .y1 = ducks[3].collision_box_y1, .x2 = ducks[3].collision_box_x2, .y2 = ducks[3].collision_box_y2}, (Rect){.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}))
            validSpawn = true;
    }
    validSpawn = false;

    snowman->x = x;
    snowman->y = y;
    snowman->collision_box_x1 = x1;
    snowman->collision_box_y1 = y1;
    snowman->collision_box_x2 = x2;
    snowman->collision_box_y2 = y2;
    snowman->time = 0.0f;
    snowman->idle_sprite = sequence_game_snowman_idle_sprite;
    snowman->jump_sprite = sequence_game_snowman_jump_sprite;
    snowman->next = snowmen;
    return snowman;
}

void free_snowmen(Snowman *snowmen)
{
    if (snowmen == NULL)
        return;
    free_snowmen(snowmen->next);
    free(snowmen);
}

void list_snowmen(Snowman *snowmen)
{
    while (snowmen != NULL)
    {
        // fprintf(stderr, "Snowman: (%f,%f) for %f\n", snowmen->x, snowmen->y, snowmen->time);
        snowmen = snowmen->next;
    }
}

int count_snowmen(Snowman *snowmen)
{
    int count = 0;
    while (snowmen != NULL)
    {
        count++;
        snowmen = snowmen->next;
    }
    return count;
}