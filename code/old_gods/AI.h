/*
===============================================================================
AI_H

Definition for functions for AI 
===============================================================================
*/
#ifndef AI_H
#define AI_H
#include "ECS/Entities/AF_ECS.h"
#include "ECS/Components/AF_CAI_Behaviour.h"
#include "App.h"
#include "AF_Lib_Define.h"



/*
====================
AI_INIT
Definition for AI init
====================
*/
void AI_Init();

/*
====================
AI_UPDATE
Definition for AI update
====================
*/
void AI_Update(AppData* _appData);


/*
====================
AI_EarlyUpdate
Definition for AI early update
====================
*/
void AI_EarlyUpdate(AF_ECS* _ecs);

/*
====================
AI_LateUpdate
functino to be called during late update. allows rendering debug to occur and to occur after movmeent
======
*/
void AI_LateUpdate(AF_ECS* _ecs);

/*
====================
AI_LateRenderUpdate
functino to be called during late update. allows rendering debug to occur and to occur inbetween render calls, allowing debug rendering to occur
======
*/
void AI_LateRenderUpdate(AF_ECS* _ecs);


/*
====================
AI_SHUTDOWN
Definition for AI shutdown 
====================
*/
void AI_Shutdown();

//=====HELPER FUNCTIONS=====


void AI_CreateFollow_Action(AF_Entity* _entity, AF_Entity* _entityToFollow, void* _aiActionFunctionPtr);

#endif // AI_H
