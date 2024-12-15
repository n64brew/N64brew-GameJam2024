#ifndef GAMEJAM2024_LARCENYGAMEAI_H
#define GAMEJAM2024_LARCENYGAMEAI_H 

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "./larcenygame.h"

/*********************************
    Structs exclusive to AI
*********************************/

typedef enum
{
    StateWaiting,
    StateWandering,
    StateFollowingOtherPlayer,
    StateMoveToObjective,
    StateRunningFromGuard,
    StateStuck

} AIMachineState;

typedef enum
{
    targetTypeNone,
    targetTypeObjective,
    targetTypePlayer,
    targetTypeGuard,
    targetTypeThief
} AITargetType;

typedef struct
{
    bool isAnAI;
    int controlledEntityIndex;
    int targetIndex;
    T3DVec3 destination;
    int framesRemainingBeforeCheck;
    float stuckTimer;
    AIMachineState currentState;
    AIMachineState previousState;
    AITargetType targetType;
} AIDataStruct;

typedef struct
{
    bool isValidTarget;
    int targetIndex;
    AITargetType targetType;
    float distanceToTarget;
} EntitySearchReturnData;

/*********************************
            Functions
*********************************/

void ai_waitingStateEnter(int aiIndex);
void ai_waitingState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed);
void ai_wanderingStateEnter(int aiIndex);
void ai_wanderingState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed);
void ai_followingOtherPlayerStateEnter(int aiIndex);
void ai_followingOtherPlayerState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed);
void ai_moveToObjectiveStateEnter(int aiIndex);
void ai_moveToObjectiveState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed);
void ai_runningFromGuardStateEnter(int aiIndex);
void ai_runningFromGuardState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed);

void ai_findClosestEntityOfType(EntitySearchReturnData* returnStruct, int aiIndex, AITargetType desiredType);
bool ai_checkForProximityBasedStateChanges(int aiIndex);
void ai_findNewState(int aiIndex);
int  ai_getCurrentStateAsInt(int aiIndex);

void ai_init(player_data* a_players, int a_playerDataSize, objective_data* a_objectives, 
        int a_objectiveDataSize, collisionobject_data* a_collisionObjects, int a_collisionObjectSize);
// pass in player, objective and collision pointers
int ai_assign(); // returns an index to the AI to be passed back in for ai_update
void ai_update(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed); // main update function that traverses the state machine, pass in index and deltaTime
void ai_cleanup(); // ensures anything that needs to be freed, is

#endif