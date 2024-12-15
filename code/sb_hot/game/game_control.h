#ifndef GAME_CONTROLS_H
#define GAME_CONTROLS_H

void game_setControlData(Game *game, Player *player)
{
    for (uint8_t i = 0; i < PLAYER_COUNT; i++)
    {

        if (player[i].control.pressed.start)
        {

            switch (game->state)
            {
            case PAUSE:
                game->state = GAMEPLAY;
                break;
            case GAMEPLAY:
                game->state = PAUSE;
                break;
            case INTRO:
                game->state = MAIN_MENU;
                break;
            case MAIN_MENU:
                game->state = CHARACTER_SELECT;
                break;
            case CHARACTER_SELECT:
                break;
            }
        }
    }
}

#endif