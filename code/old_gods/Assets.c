#include "assets.h"

const char *texture_path[TEXTURE_COUNT] = {
    "rom:/old_gods/panel_white_64_64.sprite",       // TEXTURE_ID_0
    "rom:/old_gods/panel_pink_64_64.sprite",        // TEXTURE_ID_1
    "rom:/old_gods/player1_panel_64_64.sprite",      // TEXTURE_ID_2
    "rom:/old_gods/player2_panel_64_64.sprite",      // TEXTURE_ID_3
    "rom:/old_gods/player3_panel_64_64.sprite",      // TEXTURE_ID_4
    "rom:/old_gods/player4_panel_64_64.sprite"      // TEXTURE_ID_5
};


// ============= Models ================
// Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
const char *model_paths[MODEL_COUNT] = {
    "rom:/old_gods/box.t3dm",           // MODEL_BOX
    "rom:/old_gods/food.t3dm",          // MODEL_FOOD
    "rom:/old_gods/map2.t3dm",           // MODEL_MAP
    "rom:/old_gods/snake.t3dm",         // MODEL_SNAKE
    "rom:/old_gods/rat.t3dm",           // MODEL_RAT
    "rom:/old_gods/sea_foam.t3dm",
    "rom:/old_gods/cylinder.t3dm",
    "rom:/old_gods/torus.t3dm"
};

const char* fontPath2 = "rom:/old_gods/ZenDots-Regular.font64";
const char* fontPath3 = "rom:/old_gods/Jumpman_H1.font64"; //"rom:/old_gods/Electrolize-Regular.font64";
const char* fontPath4 = "rom:/old_gods/Jumpman_H2.font64";//"rom:/old_gods/ZenDots-Regular.font64";
const char* fontPath5 = "rom:/old_gods/Jumpman_P.font64";
