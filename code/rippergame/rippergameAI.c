#include "./rippergameAI.h"


/*********************************
            Globals
*********************************/

AIDataStruct aiData[MAXPLAYERS];

player_data* playersRef;
int playerDataSize;
objective_data* objectivesRef;
int objectiveDataSize;
collisionobject_data* collisionObjectsRef;
int collisionObjectSize;


/*********************************
        State functions
*********************************/

void ai_waitingStateEnter(int aiIndex)
{
    aiData[aiIndex].currentState = StateWaiting;
    aiData[aiIndex].framesRemainingBeforeCheck = rand()%60;
}

void ai_waitingState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed)
{
    if(aiData[aiIndex].framesRemainingBeforeCheck <= 0)
    {
        // determine if it's time for a new state
        switch (rand()%2)
        {
            case 1:
                ai_wanderingStateEnter(aiIndex);
                break;
            default:
                ai_waitingStateEnter(aiIndex);
                break;
        }

    }
    else
    {
        *newDir = playersRef[aiIndex].playerPos;
        *speed = 0.0f;

        aiData[aiIndex].framesRemainingBeforeCheck--;
    }
}

void ai_wanderingStateEnter(int aiIndex)
{
    aiData[aiIndex].currentState = StateWandering;
    aiData[aiIndex].framesRemainingBeforeCheck = rand()%300;

    aiData[aiIndex].destination.v[0] = (rand()%170-85) * 0.05f;
    aiData[aiIndex].destination.v[2] = (rand()%170-85) * 0.05f;
}

void ai_wanderingState(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed)
{
    // super basic test AI
    if(aiData[aiIndex].framesRemainingBeforeCheck <= 0)
    {
        // determine if it's time for a new state
        switch (rand()%2)
        {
            case 1:
                ai_wanderingStateEnter(aiIndex);
                break;
            default:
                ai_waitingStateEnter(aiIndex);
                break;
        }
    }
    else
    {
        *newDir = aiData[aiIndex].destination;
    
        aiData[aiIndex].framesRemainingBeforeCheck--;
    }

    *speed = sqrtf(t3d_vec3_len2(newDir));
}

// pass in player, objective and collision pointers
void ai_init(player_data* a_players, int a_playerDataSize, objective_data* a_objectives, 
        int a_objectiveDataSize, collisionobject_data* a_collisionObjects, int a_collisionObjectSize)
{
    // initialise the AI specific globals
    playersRef = a_players;
    playerDataSize = a_playerDataSize;
    objectivesRef = a_objectives;
    objectiveDataSize = a_objectiveDataSize;
    collisionObjectsRef = a_collisionObjects;
    collisionObjectSize = a_collisionObjectSize;

    // initialise the AI data structs
    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        aiData[iDx].isAnAI = false;
        aiData[iDx].controlledEntityIndex = 0;
        aiData[iDx].targetIndex = 0;
        aiData[iDx].destination = (T3DVec3){{0.0f, 0.0, 0.0f}};

        aiData[iDx].currentState = StateWaiting;
        ai_waitingStateEnter(iDx);
        aiData[iDx].targetType = targetTypeNone;
    }

    return;
}

int ai_assign(int a_playerNumber) // returns an index to the AI to be passed back in for ai_update
{
    aiData[a_playerNumber].controlledEntityIndex = a_playerNumber;
    return a_playerNumber;
}

void ai_update(int aiIndex, float deltaTime, T3DVec3* newDir, float* speed) // main update function that traverses the state machine, pass in index and deltaTime
{
    // for now, just put us in the wandering state randomly
    switch(aiData[aiIndex].currentState)
    {
        case StateWaiting:
            ai_waitingState(aiIndex, deltaTime, newDir, speed);
            break;
        case StateWandering:
            ai_wanderingState(aiIndex, deltaTime, newDir, speed);
            break;
        case StateFollowing:    // TODO fix this
        default:
            ai_waitingStateEnter(aiIndex);
            break;

    }
    return;
}

void ai_cleanup() // ensures anything that needs to be freed, is
{
    return;
}