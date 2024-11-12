#include "assets.h"

const char *texture_path[TEXTURE_COUNT] = {
    "rom:/old_gods/green.sprite",    // 0 player 1
    "rom:/old_gods/red.sprite",      // 1 player 2
    "rom:/old_gods/orange.sprite",   // 2 player 3
    "rom:/old_gods/purple.sprite",   // 3 player 4
    "rom:/old_gods/grey.sprite",     // 4 god
    "rom:/old_gods/diamond0.sprite", // 5 bucket
    "rom:/old_gods/triangle0.sprite",// 6 villages
    "rom:/old_gods/checker.sprite",  // 7 level
    "rom:/old_gods/dark.sprite",     // 8 level
    "rom:/old_gods/god.sprite"       // 9 god
};


// ============= Models ================
const char *model_paths[MODEL_COUNT] = {
    "rom:/old_gods/box.t3dm",
    "rom:/old_gods/food.t3dm",
    "rom:/old_gods/map.t3dm",
    "rom:/old_gods/shadow.t3dm",
    "rom:/old_gods/snake.t3dm"  // Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
};


const char* cannonFXPath = "rom:/old_gods/cannon.wav64";
const char* laserFXPath = "rom:/old_gods/laser.wav64";
const char* musicFXPath = "rom:/old_gods/monosample8.wav64";

const char* fontPath = "rom:/old_gods/Electrolize-Regular.font64";
const char* fontPath2 = "rom:/old_gods/ZenDots-Regular.font64";
