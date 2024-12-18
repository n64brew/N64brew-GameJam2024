/*
===============================================================================
AF_CPLAYERDATA_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#ifndef AF_CPLAYERDATA_H
#define AF_CPLAYERDATA_H
#include "AF_Component.h"
#ifdef __cplusplus
extern "C" {    
#endif

enum PLAYER_FACTION {
	DEFAULT = 0,
	PLAYER = 1,
	ENEMY1 = 2,
	ENEMY2 = 3,
	ENEMY3 = 4,
	ENEMY4 = 5
};
/*
====================
AF_CSprite
====================
*/
// size is 64 bytes
typedef struct  {
	//BOOL has;
	//BOOL enabled;
	PACKED_CHAR enabled;	    // 1 byte
	BOOL isHuman;
	BOOL isCarrying;
	BOOL isCarried;
    int carryingEntity;
    float health;
    BOOL isAlive;
	BOOL isAttacking;
	BOOL isJumping;
	float movementSpeed;
	int score;
	Vec3 startPosition;
	Vec3 targetDestination;
	float spawnTime;
	enum PLAYER_FACTION faction;
	
} AF_CPlayerData;


/*
====================
AF_AF_CPlayerData_ZERO
Empty constructor for the AF_CSprite component
====================
*/
static inline AF_CPlayerData AF_CPlayerData_ZERO(void){
	Vec3 startPosition = {0,0,0};
	AF_CPlayerData returnComponent = {
		//.has = false,
		.enabled = FALSE,
		.isHuman = FALSE,
		.isCarrying = FALSE,
		.isCarried = FALSE,
        .carryingEntity = 0,
        .health = 0,
        .isAlive = FALSE,
		.isAttacking = FALSE,
		.isJumping = FALSE,
		.score = 0,
		.movementSpeed = 0,
		.startPosition = startPosition,
		.targetDestination = startPosition,
		.spawnTime = 0.0f,
		.faction = 0
		};
	return returnComponent;
}

/*
====================
AF_AF_CPlayerData_ADD
ADD component and set default values
====================
*/
static inline AF_CPlayerData AF_CPlayerData_ADD(void){
	PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
	Vec3 startPosition = {0,0,0};
	AF_CPlayerData returnComponent = {
		//.has = true,
		.enabled = component,
		.isHuman = TRUE,
		.isCarrying = FALSE,
		.isCarried = FALSE,
        .carryingEntity = 0,
        .health = 100,
        .isAlive = TRUE,
		.isAttacking = FALSE,
		.isJumping = FALSE,
		.score = 0,
		.movementSpeed = 1.0f,
		.startPosition = startPosition,
		.targetDestination = startPosition,
		.spawnTime = 0.0f,
		.faction = 0
	};
	return returnComponent;
}

#ifdef __cplusplus
}
#endif

#endif