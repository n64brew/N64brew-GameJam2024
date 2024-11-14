#include "PlayerController.h"
#include "AF_Physics.h"
#include "AF_Vec2.h"
#include <libdragon.h>
#include "AF_Input.h"
#include "AF_Renderer.h"

#define STICK_DEAD_ZONE 0.01
#define PLAYER_SPEED 10
#define PI 3.14159265358979f
#define PLAYER_COUNT 4

#define A_KEY 0			// A Button		
#define B_KEY 1			// B Button
#define START_KEY 2		// Start Button

void PlayerController_UpdateAllPlayerMovements(AF_Input* _input, AF_Entity* _entities, uint8_t _entityCount){
	for(int i = 0; i < _entityCount; ++i){
		// player 1
		PlayerController_UpdatePlayerMovement(_input->controlSticks[i], &_entities[i]);
		PlayerController_UpdatePlayerButtonPress(_input, &_entities[i]);
	}	
}

/// @brief Interpolates between two angles (radians) by 't', from T3D
float Lerp_Angle(float a, float b, float t) {
  float angleDiff = fmodf((b - a), PI*2);
  float shortDist = fmodf(angleDiff*2, PI*2) - angleDiff;
  return a + shortDist * t;
}

void PlayerController_UpdatePlayerButtonPress(AF_Input* _input, AF_Entity* _entity){
	// Handle attack
	for(int i = 0; i < PLAYER_COUNT; ++i){
		AF_CPlayerData* playerData = _entity->playerData;

		
		// A to attack
		if(_input->keys[i][A_KEY].pressed == TRUE){
			playerData->isAttacking = TRUE;
			PlayerController_Attack(_entity);
		}else{
			playerData->isAttacking = FALSE;
		}

		// B to jump
		if(_input->keys[i][B_KEY].pressed == TRUE){
			playerData->isJumping = TRUE;
		}else{
			playerData->isJumping = FALSE;
		}
	}
}

void PlayerController_Attack(AF_Entity* _entity){
	// do collision check in proximity.
	// entity that is another player, then call a hit on that player
	// Player attack animation
	AF_CMesh* mesh = _entity->mesh;
	// if this entity has animations, then call play animation
	if(mesh->animation.has == TRUE){
		AF_Renderer_PlayAnimation(&mesh->animation);
	}
}

 void PlayerController_UpdatePlayerMovement(Vec2 _stick, AF_Entity* _entity){
	int vecX = 0;
	int vecY = 0;

    // Player 1
	if (_stick.y > STICK_DEAD_ZONE){
		vecY = -1;
		//debugf("Stick y %f:\n",y);
	}
	if(_stick.y < -STICK_DEAD_ZONE){
		vecY = 1;
		//debugf("Stick y %f:\n",y);
	}

	if(_stick.x > STICK_DEAD_ZONE){
		//debugf("Stick y %f:\n",y);
		vecX = 1;
	}
	if(_stick.x < -STICK_DEAD_ZONE ){
		//debugf("Stick x %f:\n",x);
		vecX = -1;
	}

	if(_stick.x == 0){
		vecX = 0;
	}

	if(_stick.y == 0){
		vecY = 0;
	}

    // update the cube rigidbody velocity
	Vec3 newVelocity = {PLAYER_SPEED * vecX, 0, PLAYER_SPEED * vecY};
	_entity->rigidbody->velocity = newVelocity;//newVelocity; 

	// adjust rotation
	float newAngle = atan2f(-vecX, vecY);
	_entity->transform->rot.y = Lerp_Angle(_entity->transform->rot.y, newAngle, 0.25f);
 }