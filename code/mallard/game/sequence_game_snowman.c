#include <libdragon.h>
#include "sequence_game_initialize.h"
#include "sequence_game_snowman.h"

Snowman *add_snowman(Snowman *snowmen)
{
    Snowman *snowman = (Snowman *)malloc(sizeof(Snowman));
    snowman->action = SNOWMAN_IDLE;
    snowman->frames = 0;
    snowman->locked_for_frames = 0;
    snowman->x = random_between(MIN_X, MAX_X);
    snowman->y = random_between(MIN_Y, MAX_Y);
    snowman->collision_box_x1 = snowman->x + 1;
    snowman->collision_box_y1 = snowman->y + 1;
    snowman->collision_box_x2 = snowman->x + 12;
    snowman->collision_box_y2 = snowman->y + 16;
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
        fprintf(stderr, "Snowman: (%f,%f) for %f\n", snowmen->x, snowmen->y, snowmen->time);
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