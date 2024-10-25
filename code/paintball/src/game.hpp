#ifndef __GAME_H
#define __GAME_H

#include <libdragon.h>

#include "../../core.h"
#include "../../minigame.h"

#ifdef __cplusplus

#include <functional>

class Game
{
    private:
        void timer_callback();
    public:
        Game();
        ~Game();
        void update(float deltatime);
        void fixed_update(float deltatime);
};

#else

typedef struct Game Game;

#endif

#ifdef __cplusplus
extern "C" {
#endif

Game* Game_new();
void Game_update(Game* self, float delta);
void Game_fixed_update(Game* self, float delta);
void Game_destroy(Game* self);


#ifdef __cplusplus
}
#endif

#endif /* __GAME_H */

