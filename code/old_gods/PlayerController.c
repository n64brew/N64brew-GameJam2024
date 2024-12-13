#include "PlayerController.h"
#include "AF_Physics.h"
#include "AF_Vec2.h"
#include <libdragon.h>
#include "AF_Renderer.h"


#define STICK_DEAD_ZONE 0.01
#define PLAYER_COUNT 4

#define A_KEY 0			// A Button		
#define B_KEY 1			// B Button
#define START_KEY 2		// Start Button

/*===============
PlayerController_UpdateAllPlayerMovements
// Update all player movements
// loop through the provided entity array 
================*/
void PlayerController_UpdateAllPlayerMovements(AppData* _appData){
	for(int i = 0; i < PLAYER_COUNT; ++i){
		// player 1
		PlayerController_UpdatePlayerMovement(_appData->input.controlSticks[i], _appData->gameplayData.playerEntities[i]);
		PlayerController_UpdatePlayerButtonPress(i, &_appData->input, _appData->gameplayData.playerEntities[i]);
	}	
}

/*===============
PlayerController_UpdatePlayerButtonPress
// Update player button press
================*/
void PlayerController_UpdatePlayerButtonPress(uint8_t _playerIndex, AF_Input* _input, AF_Entity* _entity){
	// Handle attack
		AF_CPlayerData* playerData = _entity->playerData;

		
		// A to attack
		if(_input->keys[_playerIndex][A_KEY].pressed == 1){
			playerData->isAttacking = TRUE;
			PlayerController_Attack(_entity);
			// deal damage to enemies in range


		}else{
			// TODO: make this a delayed off
			playerData->isAttacking = FALSE;
		}

		// B to jump
		if(_input->keys[_playerIndex][B_KEY].pressed == TRUE){
			playerData->isJumping = TRUE;
		}else{
			playerData->isJumping = FALSE;
		}
}

void PlayerController_Attack(AF_Entity* _entity){
	// do collision check in proximity.
	// entity that is another player, then call a hit on that player
	AF_CSkeletalAnimation*  animation = _entity->skeletalAnimation;
	// if this entity has animations, then call play animation
	BOOL hasMeshComponent = AF_Component_GetHas(animation->enabled);
	BOOL isEnabled = AF_Component_GetEnabled(animation->enabled);
	if(hasMeshComponent == TRUE && isEnabled == TRUE){
		AF_Renderer_PlayAnimation(animation);
	}
}

/*===============
PlayerController_UpdatePlayerMovement
convert the input into directions on velocity
apply physics
================*/
 void PlayerController_UpdatePlayerMovement(Vec2 _stick, AF_Entity* _entity){
	int vecX = 0;
	int vecY = 0;
    // Player 1
	if (_stick.y > STICK_DEAD_ZONE){
		vecY = -1;
	}
	if(_stick.y < -STICK_DEAD_ZONE){
		vecY = 1;
	}

	if(_stick.x > STICK_DEAD_ZONE){
		vecX = 1;
	}
	if(_stick.x < -STICK_DEAD_ZONE ){
		vecX = -1;
	}

	if(_stick.x == 0){
		vecX = 0;
	}

	if(_stick.y == 0){
		vecY = 0;
	}
	AF_CPlayerData* playerData = _entity->playerData;
    // update the cube rigidbody velocity
	Vec3 movementForce = {playerData->movementSpeed * vecX, 0, playerData->movementSpeed * vecY};
	
	//_entity->rigidbody->velocity = newVelocity;//newVelocity; 
	AF_Physics_ApplyLinearImpulse(_entity->rigidbody, movementForce);
	// adjust rotation
	float newAngle = atan2f(-vecX, vecY);
	_entity->transform->rot.y = AF_Math_Lerp_Angle(_entity->transform->rot.y, newAngle, 0.25f);
 }