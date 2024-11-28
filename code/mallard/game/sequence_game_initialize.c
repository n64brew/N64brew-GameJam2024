#include <libdragon.h>
#include "../../../core.h"
#include "sequence_game_initialize.h"
#include "sequence_game.h"

int random_between(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void initialize_ducks()
{
    if (ducks == NULL)
    {
        ducks = malloc(4 * sizeof(struct Duck));

        ducks[0].action = DUCK_BASE;
        ducks[0].frames = 0;
        ducks[0].locked_for_frames = 0;
        ducks[0].x = random_between(PLAYER_1_SPAWN_X1, PLAYER_1_SPAWN_X2);
        ducks[0].y = random_between(PLAYER_1_SPAWN_Y1, PLAYER_1_SPAWN_Y2);
        ducks[0].direction = RIGHT;
        ducks[0].base_sprite = sequence_game_mallard_one_base_sprite;
        ducks[0].walk_sprite = sequence_game_mallard_one_walk_sprite;
        ducks[0].slap_sprite = sequence_game_mallard_one_slap_sprite;
        ducks[0].run_sprite = sequence_game_mallard_one_run_sprite;

        ducks[1].action = DUCK_BASE;
        ducks[1].frames = 0;
        ducks[1].locked_for_frames = 0;
        ducks[1].x = random_between(PLAYER_2_SPAWN_X1, PLAYER_2_SPAWN_X2);
        ducks[1].y = random_between(PLAYER_2_SPAWN_Y1, PLAYER_2_SPAWN_Y2);
        ducks[1].direction = LEFT;
        ducks[1].base_sprite = sequence_game_mallard_two_base_sprite;
        ducks[1].walk_sprite = sequence_game_mallard_two_walk_sprite;
        ducks[1].slap_sprite = sequence_game_mallard_two_slap_sprite;
        ducks[1].run_sprite = sequence_game_mallard_two_run_sprite;

        ducks[2].action = DUCK_BASE;
        ducks[2].frames = 0;
        ducks[2].locked_for_frames = 0;
        ducks[2].x = random_between(PLAYER_3_SPAWN_X1, PLAYER_3_SPAWN_X2);
        ducks[2].y = random_between(PLAYER_3_SPAWN_Y1, PLAYER_3_SPAWN_Y2);
        ducks[2].direction = RIGHT;
        ducks[2].base_sprite = sequence_game_mallard_three_base_sprite;
        ducks[2].walk_sprite = sequence_game_mallard_three_walk_sprite;
        ducks[2].slap_sprite = sequence_game_mallard_three_slap_sprite;
        ducks[2].run_sprite = sequence_game_mallard_three_run_sprite;

        ducks[3].action = DUCK_BASE;
        ducks[3].frames = 0;
        ducks[3].locked_for_frames = 0;
        ducks[3].x = random_between(PLAYER_4_SPAWN_X1, PLAYER_4_SPAWN_X2);
        ducks[3].y = random_between(PLAYER_4_SPAWN_Y1, PLAYER_4_SPAWN_Y2);
        ducks[3].direction = LEFT;
        ducks[3].base_sprite = sequence_game_mallard_four_base_sprite;
        ducks[3].walk_sprite = sequence_game_mallard_four_walk_sprite;
        ducks[3].slap_sprite = sequence_game_mallard_four_slap_sprite;
        ducks[3].run_sprite = sequence_game_mallard_four_run_sprite;
    }
}

void free_ducks()
{
    if (ducks != NULL)
    {
        free(ducks);
        ducks = NULL;
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