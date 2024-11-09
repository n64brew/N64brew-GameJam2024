/*
===============================================================================
GAMEDATA_H

Definition gameplay data struct
===============================================================================
*/
#ifndef GAMEPLAYDATA_H
#define GAMEPLAYDATA_H

// Gameplay variables
#define COUNT_DOWN_TIME 120
#define GODS_EAT_COUNT 15

// ===== Game State ====
enum GAME_STATE{
    GAME_STATE_MAIN_MENU = 0,
    GAME_STATE_PLAYING = 1,
    GAME_STATE_GAME_OVER_LOSE = 2,
    GAME_STATE_GAME_OVER_WIN = 3,
};


/*================
GameData 
Struct to hold game data to pass around to functions that need it
*/
typedef struct GameplayData {
    // Gameplay Vars
    int godEatCount;
    float countdownTimer;
    enum GAME_STATE gameState;
    // SpriteSheet
    int currentBucket;
} GameplayData;

// Factor function to initialise the gameplay data
static inline GameplayData GameplayData_INIT(){
    GameplayData gameplayData = {
        .godEatCount = GODS_EAT_COUNT,
        .countdownTimer = COUNT_DOWN_TIME,
        .gameState = GAME_STATE_MAIN_MENU,
        .currentBucket = 0
    };
    return gameplayData;
}

#endif