#include "damage.hpp"

Bullet::Bullet() :
    pos {0},
    direction {0},
    color {0},
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}) {}

DamageController::DamageController() :
    model({
        t3d_model_load("rom:/paintball/snake.t3dm"),
        t3d_model_free
    }),
    block({nullptr, rspq_block_free}) {}

void DamageController::update(float deltaTime) {

}

void DamageController::fixed_update(float deltaTime) {

}