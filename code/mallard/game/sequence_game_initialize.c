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

        for (size_t i = 0; i < 4; i++)
        {
            switch (i)
            {
            case 0:
                characters[i].x = random_between(PLAYER_1_SPAWN_X1, PLAYER_1_SPAWN_X2);
                characters[i].y = random_between(PLAYER_1_SPAWN_Y1, PLAYER_1_SPAWN_Y2);
                break;
            case 1:
                characters[i].x = random_between(PLAYER_2_SPAWN_X1, PLAYER_2_SPAWN_X2);
                characters[i].y = random_between(PLAYER_2_SPAWN_Y1, PLAYER_2_SPAWN_Y2);
                break;
            case 2:
                characters[i].x = random_between(PLAYER_3_SPAWN_X1, PLAYER_3_SPAWN_X2);
                characters[i].y = random_between(PLAYER_3_SPAWN_Y1, PLAYER_3_SPAWN_Y2);
                break;
            default:
                characters[i].x = random_between(PLAYER_4_SPAWN_X1, PLAYER_4_SPAWN_X2);
                characters[i].y = random_between(PLAYER_4_SPAWN_Y1, PLAYER_4_SPAWN_Y2);
                break;
            }
        }
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