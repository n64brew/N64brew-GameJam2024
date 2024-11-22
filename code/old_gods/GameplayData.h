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
#define GODS_EAT_COUNT 5
#define MAX_OBSERVERS 10
#define PLAYER_COUNT 4

// ===== Game State ====
enum GAME_STATE{
    GAME_STATE_MAIN_MENU = 0,
    GAME_STATE_PLAYING = 1,
    GAME_STATE_GAME_OVER_LOSE = 2,
    GAME_STATE_GAME_OVER_WIN = 3,
    GAME_STATE_GAME_END = 4,
    GAME_STATE_GAME_RESTART = 5
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
    AF_Entity* playerEntities[PLAYER_COUNT];
    Vec3 levelPos;
    Vec3 levelBounds;
    //Observer observers[MAX_OBSERVERS];
    //int observerCount;
} GameplayData;

// Factor function to initialise the gameplay data
static inline GameplayData GameplayData_INIT(){
    GameplayData gameplayData = {
        .godEatCount = GODS_EAT_COUNT,
        .countdownTimer = COUNT_DOWN_TIME,
        .gameState = GAME_STATE_MAIN_MENU,
        .currentBucket = 0,
        .playerEntities = {NULL, NULL, NULL, NULL}
    };
    return gameplayData;
}

/*
// Function to register an observer
static inline void GameState_RegisterObserver(GameplayData* gameState, EventCallback callback) {
    if (gameState->observerCount < MAX_OBSERVERS) {
        gameState->observers[gameState->observerCount++].callback = callback;
    }
}

static inline void GameState_NotifyObservers(GameplayData* gameState, GameEventType event, int playerId, int score) {
    for (int i = 0; i < gameState->observerCount; i++) {
        if (gameState->observers[i].callback != NULL) {
            gameState->observers[i].callback(event, playerId, score, gameState);
        }
    }
}
*/
#endif