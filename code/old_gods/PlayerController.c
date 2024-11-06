#include "PlayerController.h"
#include "AF_Physics.h"
#include "AF_Vec2.h"

#define STICK_DEAD_ZONE 0.01
#define PLAYER_SPEED 10

 void UpdatePlayerMovement(Vec2 _stick, AF_Entity* _entity){
   
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

 }