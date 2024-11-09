/*
===============================================================================
OBSERVER_H definitions
Definition of observer pattren
===============================================================================
*/

#ifndef OBSERVER_H
#define OBSERVER_H
#include "GameplayData.h"
// Define an event type, such as SCORE_UPDATED
typedef enum {
    EVENT_SCORE_UPDATED,
    // Add more events as needed
} GameEventType;

// Observer callback function type
typedef void (*EventCallback)(GameEventType event, int playerId, int score, GameplayData* gameState);

// Observer struct
typedef struct Observer {
    EventCallback callback;
} Observer;

#endif

