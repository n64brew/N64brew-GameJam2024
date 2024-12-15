/*
===============================================================================
GAMEDATA_H

Definition gameplay data struct
===============================================================================
*/
//#include "Observer.h"
#ifndef GAMEPLAYDATA_H
#define GAMEPLAYDATA_H

// Gameplay variables
#define COUNT_DOWN_TIME 120
#define GODS_EAT_COUNT 500
#define MAX_OBSERVERS 10
#define PLAYER_COUNT 4
#define ENEMY_POOL_COUNT 0

// ===== Game State ====
enum GAME_STATE{
    GAME_STATE_MAIN_MENU = 0,
    GAME_STATE_COUNTDOWN = 1,
    GAME_STATE_PLAYING = 2,
    GAME_STATE_PAUSED = 3,
    GAME_STATE_GAME_OVER_LOSE = 4,
    GAME_STATE_GAME_OVER_WIN = 5,
    GAME_STATE_GAME_END = 6,
    GAME_STATE_GAME_RESTART = 7
};


/* ================
GameData 
Struct to hold game data to pass around to functions that need it
================ */
typedef struct GameplayData {
    BOOL isDebug;
    // Gameplay Vars
    int godEatCount;
    float countdownTimer;
    enum GAME_STATE gameState;
    // SpriteSheet
    int currentBucket;
    AF_Entity* playerEntities[PLAYER_COUNT];
    AF_Entity* enemyEntities[ENEMY_POOL_COUNT];
    AF_Entity* rat;
    Vec3 levelPos;
    Vec3 levelBounds;
    //Observer observers[MAX_OBSERVERS];
    //int observerCount;
} GameplayData;

/*==============================
    GameplayData_INIT
    Factor function to initialise the gameplay data
==============================*/
static inline GameplayData GameplayData_INIT(){
    GameplayData gameplayData = {
        .isDebug = FALSE,
        .godEatCount = GODS_EAT_COUNT,
        .countdownTimer = COUNT_DOWN_TIME,
        .gameState = GAME_STATE_MAIN_MENU,
        .currentBucket = 0,
        .playerEntities = {NULL, NULL, NULL, NULL}
    };
    return gameplayData;
}

#endif