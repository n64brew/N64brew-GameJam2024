#include <libdragon.h>
#include "../../../core.h"
#include "sequence_game_initialize.h"
#include "sequence_game.h"

int random_between(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void initialize_characters()
{
    if (characters == NULL)
    {
        characters = malloc(4 * sizeof(struct Character));
        characters[0].x = random_between(PLAYER_1_SPAWN_X1, PLAYER_1_SPAWN_X2);
        characters[0].y = random_between(PLAYER_1_SPAWN_Y1, PLAYER_1_SPAWN_Y2);
        characters[1].x = random_between(PLAYER_2_SPAWN_X1, PLAYER_2_SPAWN_X2);
        characters[1].y = random_between(PLAYER_2_SPAWN_Y1, PLAYER_2_SPAWN_Y2);
        characters[2].x = random_between(PLAYER_3_SPAWN_X1, PLAYER_3_SPAWN_X2);
        characters[2].y = random_between(PLAYER_3_SPAWN_Y1, PLAYER_3_SPAWN_Y2);
        characters[3].x = random_between(PLAYER_4_SPAWN_X1, PLAYER_4_SPAWN_X2);
        characters[3].y = random_between(PLAYER_4_SPAWN_Y1, PLAYER_4_SPAWN_Y2);
    }
}

void free_characters()
{
    if (characters != NULL)
    {
        free(characters);
        characters = NULL;
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