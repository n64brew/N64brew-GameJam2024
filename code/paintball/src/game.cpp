#include "game.hpp"

Game::Game() :
    viewport(t3d_viewport_create()),
    font("rom:/squarewave.font64", 1),
    timer({
        new_timer_context(TICKS_FROM_MS(1000), TF_ONE_SHOT, [](int ovfl, void* self) -> void { ((Game*)self)->timer_callback(); }, this),
        delete_timer
    }),
    mapMatFP({
        (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)),
        free_uncached
    })
{
    debugf("Paintball minigame initialized\n");
}


void Game::timer_callback() {
    debugf("Timer callback called\n");
    minigame_end();
}

Game::~Game() {
    debugf("Paintball minigame cleaned up\n");
}

void Game::update(float deltatime) {

}

void Game::fixed_update(float deltatime) {

}

Game* Game_new()
{
    return new Game();
}

void Game_update(Game* self, float delta)
{
    self->update(delta);
}

void Game_fixed_update(Game* self, float delta)
{
    self->fixed_update(delta);
}

void Game_destroy(Game* self)
{
    self->~Game();
}

