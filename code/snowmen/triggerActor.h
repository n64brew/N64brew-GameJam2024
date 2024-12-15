#ifndef TRIGGERACTOR_HEADER
#define TRIGGERACTOR_HEADER

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include "actor.h"


typedef struct TriggerStruct{
    Actor TriggerActor;

} TriggerStruct;

void CreateTriggerActor(TriggerStruct* triggerStruct);

void TriggerActorInit(struct TriggerStruct* triggerStruct);

void TriggerActorLoop(struct TriggerStruct* triggerStruct, float deltaTime);

void TriggerFree(TriggerStruct* triggerStruct);


#endif