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

void increase_snowmen_time_and_frames(Snowman *snowmen, float deltatime)
{
    while (snowmen != NULL)
    {
        snowmen->frames++;
        snowmen->time += deltatime;
        snowmen = snowmen->next;
    }
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