#ifndef GAMEJAM2024_RIPPERGAMEAI_H
#define GAMEJAM2024_RIPPERGAMEAI_H 

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "./rippergame.h"

/*********************************
    Structs exclusive to AI
*********************************/

typedef enum
{
    StateWaiting,
    StateWandering,
    StateFollowing
} AIMachineState;

typedef enum
{
    targetTypeNone,
    targetTypeObjective,
    targetTypePlayer
} AITargetType;

typedef struct
{
    bool isAnAI;
    int controlledEntityIndex;
    int targetIndex;
    T3DVec3 destination;
    int framesRemainingBeforeCheck;
    AIMachineState currentState;
    AITargetType targetType;
} AIDataStruct;

/*********************************
            Functions
*********************************/

void ai_waitingStateEnter(int aiIndex);
void ai_waitingState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed);
void ai_wanderingStateEnter(int aiIndex);
void ai_wanderingState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed);

void ai_init(player_data* a_players, int a_playerDataSize, objective_data* a_objectives, 
        int a_objectiveDataSize, collisionobject_data* a_collisionObjects, int a_collisionObjectSize);
// pass in player, objective and collision pointers
int ai_assign(); // returns an index to the AI to be passed back in for ai_update
void ai_update(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed); // main update function that traverses the state machine, pass in index and deltaTime
void ai_cleanup(); // ensures anything that needs to be freed, is

#endif