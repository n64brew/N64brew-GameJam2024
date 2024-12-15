#include "AI.h"
#include "App.h"
#include "ECS/Entities/AF_Entity.h"
#include <libdragon.h>

// Forward declare functions

void ExecuteAIBehaviours(AF_Entity* _entity);

/*
====================
AI_INIT
Implementation for AI init
====================
*/
void AI_Init(){
    debugf("AI_Init\n");
}

/*
====================
AI_UPDATE
Implementation for AI update
====================
*/
void AI_Update(AppData* _appData){
    // if gamestate is not playing, early exitx
    if(_appData->gameplayData.gameState != GAME_STATE_PLAYING){
        return;
    }
    /*
    for(int i = 0; i < PLAYER_COUNT; ++i){
        AF_Entity* entity = _appData->gameplayData.playerEntities[i];
        // update all the AI components
        ExecuteAIBehaviours(entity);
    }*/
    for(int i = 0; i < _appData->ecs.entitiesCount ; ++i){
        AF_Entity* entity = &_appData->ecs.entities[i];
        // update all the AI components
        ExecuteAIBehaviours(entity);
    }


    // update AI behaviour
    //AF_Entity* playerToFollow = _appData->gameplayData.playerEntities[0];
}

/*
====================
ExecuteAIBehaviours
execute the listed ai behaviours
====================
*/
void ExecuteAIBehaviours(AF_Entity* _entity){
     AF_CAI_Behaviour* aiBehaviour = _entity->aiBehaviour;
     BOOL hasAI = AF_Component_GetHas(aiBehaviour->enabled);
     BOOL isAIEnabled = AF_Component_GetEnabled(aiBehaviour->enabled);;
     if(hasAI == TRUE && isAIEnabled == TRUE){
        for(int i = 0; i < AF_AI_ACTION_ARRAY_SIZE; ++i){
            AF_AI_Action* action = &aiBehaviour->actionsArray[i];
            
            // skip acitons that are not enabled
            if(action->enabled == FALSE){
                continue;
            }
            assert(action->callback != NULL);

            // switch to different known AI behaviours
            // Glorious abuse of void*
            // trigger all the AI behaviours)
            switch(action->actionType){
                case AI_ACTION_DEFAULT:
                    // call the attack beahviour passing in relevent data
                break;

                case AI_ACTION_IDLE:
                    // do nothing
                break;

                case AI_ACTION_GOTO:
                    action->callback(action);
                break;

                case AI_ACTION_ATTACK:
                    // call the attack beahviour passing in relevent data
                break;
            }
        }
    }
}

/*
====================
AI_CreateFollow_Action
Create a follow action
====================
*/
void AI_CreateFollow_Action(AF_Entity* _entity, AF_Entity* _entityToFollow, void* _aiActionFunctionPtr){
    AF_CAI_Behaviour* entityAIBehaviour = _entity->aiBehaviour;
    // check if we have components and they are enabled
    if(entityAIBehaviour->nextAvailableActionSlot >= AF_AI_ACTION_ARRAY_SIZE-1){
        debugf("AI: AI_CreateFollow_Action: out of ai action slots\n");
        return;
    }
    
    entityAIBehaviour->enabled = AF_Component_SetEnabled(entityAIBehaviour->enabled, TRUE);
    AF_AI_Action* entityAction = &entityAIBehaviour->actionsArray[entityAIBehaviour->nextAvailableActionSlot];
    entityAction->enabled = TRUE;
    entityAction->actionType = AI_ACTION_GOTO;
    entityAction->sourceEntity = (void*)_entity;
    entityAction->targetEntity = (void*)_entityToFollow;
    entityAction->callback = _aiActionFunctionPtr;

    // progress the action slot counter
    entityAIBehaviour->nextAvailableActionSlot++;
}


/*
====================
AI_EarlyUpdate
Implementation for AI early update
====================
*/
void AI_EarlyUpdate(AF_ECS* _ecs){

}

/*
====================
AI_LateUpdate
functino to be called during late update. allows rendering debug to occur and to occur after movmeent
======
*/
void AI_LateUpdate(AF_ECS* _ecs){

}

/*
====================
AI_LateRenderUpdate
functino to be called during late update. allows rendering debug to occur and to occur inbetween render calls, allowing debug rendering to occur
======
*/
void AI_LateRenderUpdate(AF_ECS* _ecs){

}


/*
====================
AI_SHUTDOWN
Implementation for AI shutdown 
====================
*/
void AI_Shutdown(){
    debugf("AI_Shutdown\n");
}