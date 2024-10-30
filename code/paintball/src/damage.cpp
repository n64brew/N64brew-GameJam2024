#include "damage.hpp"

DamageController::DamageController() :
    model({
        t3d_model_load("rom:/paintball/snake.t3dm"),
        t3d_model_free
    }) {}
    