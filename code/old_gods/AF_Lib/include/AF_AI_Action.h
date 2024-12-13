/*
===============================================================================
AF_CAI_ACTION_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_AI_ACTION_H
#define AF_AI_ACTION_H

//#include "ECS/Entities/AF_Entity.h"

typedef enum AI_ACTION_Type {
	AI_ACTION_DEFAULT = 0,
	AI_ACTION_IDLE = 1,
	AI_ACTION_GOTO = 2,
	AI_ACTION_ATTACK = 3
} AI_ACTION_Type;


// Typedef for readability (optional but common in C)
typedef struct AF_AI_Action AF_AI_Action;

// Struct to define a function ptr to a behaviour
typedef struct AF_AI_Action {
	BOOL enabled;
	AI_ACTION_Type actionType;
	void* sourceEntity;	// doing this dirty to avoid circular dependency with AF_Entity
	void* targetEntity;
    void (*callback)(AF_AI_Action*);
} AF_AI_Action;

static inline AF_AI_Action AF_AI_Action_Zero(){
	AF_AI_Action zeroAction = {
		.enabled = FALSE,
		.actionType = AI_ACTION_DEFAULT,
		.sourceEntity = NULL,
		.targetEntity = NULL,
		.callback = NULL
	};
	return zeroAction;
}


#endif //AF_AI_ACTION_H

