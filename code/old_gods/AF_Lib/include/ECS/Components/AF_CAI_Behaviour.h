/*
===============================================================================
AF_CAI_BEHAVIOUR_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_CAI_BEHAVIOUR_H
#define AF_CAI_BEHAVIOUR_H
#include "AF_Component.h"
#include "ECS/Entities/AF_Entity.h"
#include "AF_AI_Action.h"
#ifdef __cplusplus
extern "C" {    
#endif


#define AF_AI_ACTION_ARRAY_SIZE 8
/*
====================
AF_CAI_BEHAVIOUR
Struct to hold a behaviour array
====================
*/
// Size is 64 bytes
typedef struct {			// 64 or 128 bytes
	PACKED_CHAR enabled;	// 8 bytes
	AF_AI_Action actionsArray[AF_AI_ACTION_ARRAY_SIZE];	// store up to 8 actions to perform
	uint8_t nextAvailableActionSlot;
} AF_CAI_Behaviour;

/*
====================
AF_CAI_BEHAVIOUR_ZERO
Empty constructor for the AF_CAI_BEHAVIOUR component
====================
*/
static inline AF_CAI_Behaviour AF_CAI_Behaviour_ZERO(void){
	PACKED_CHAR component = FALSE;
	component = AF_Component_SetHas(component, FALSE);
	component = AF_Component_SetEnabled(component, FALSE);
	AF_AI_Action emtyActions = AF_AI_Action_Zero();
	AF_CAI_Behaviour returnAI_Action = {
		//.has = true,
		.enabled = component,
		.actionsArray = {emtyActions, emtyActions, emtyActions, emtyActions, emtyActions, emtyActions, emtyActions, emtyActions},
		.nextAvailableActionSlot = 0

	};
	return returnAI_Action;
}

/*
====================
AF_CAI_BEHAVIOUR_ADD
ADD component and set default values
====================
*/
static inline AF_CAI_Behaviour AF_CAI_Behaviour_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	AF_AI_Action emtyActions = AF_AI_Action_Zero();
	AF_CAI_Behaviour returnAI_Action = {
		//.has = true,
		.enabled = component,
		.actionsArray = {emtyActions, emtyActions, emtyActions, emtyActions, emtyActions, emtyActions, emtyActions, emtyActions},
		.nextAvailableActionSlot = 0
	};
	return returnAI_Action;
}


#ifdef __cplusplus
}
#endif

#endif //AF_CAI_BEHAVIOUR_H

