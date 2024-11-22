#include "Scene.h"
#include "ECS/Entities/AF_Entity.h"
#include "Assets.h"
#include "EntityFactory.h"
#include "Assets.h"
#include "AF_Input.h"
#include "PlayerController.h"
// Needed for game jam
#include "../../core.h"
#include <t3d/t3dmath.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include "AF_Physics.h"
#include "AF_Math.h"
#include "AI.h"

// ECS system
AF_Entity* camera = NULL;
//AF_Entity* playerEntities[PLAYER_COUNT] = {NULL};  // Initialize all elements to NULL

// God
AF_Entity* godEntity = NULL;
AF_Entity* godEye1 = NULL;
AF_Entity* godEye2 = NULL;
AF_Entity* godEye3 = NULL;
AF_Entity* godEye4 = NULL;

// Environment
AF_Entity* leftWall = NULL;
AF_Entity* rightWall = NULL;
AF_Entity* backWall = NULL;
AF_Entity* frontWall = NULL;
AF_Entity* levelMapEntity = NULL;

// Pickup
AF_Entity* bucket1 = NULL;
AF_Entity* bucket2 = NULL;
AF_Entity* bucket3 = NULL;
AF_Entity* bucket4 = NULL;

// Villagers
AF_Entity* villager1 = NULL;

// Audio
//AF_Entity* laserSoundEntity = NULL;
//AF_Entity* cannonSoundEntity = NULL;
//AF_Entity* musicSoundEntity = NULL;

// Gameplay Var
uint8_t g_currentBucket = 0;


#define VILLAGER_CARRY_HEIGHT 1

// Minigame vars
float countDownTimer;
bool isEnding;
float endTimer;
PlyNum winner;



#define HITBOX_RADIUS       10.f

#define ATTACK_OFFSET       10.f
#define ATTACK_RADIUS       5.0f
#define ATTACK_FORCE_STRENGTH 10.0f

#define ATTACK_TIME_START   0.333f
#define ATTACK_TIME_END     0.4f

#define COUNTDOWN_DELAY     3.0f
#define GO_DELAY            1.0f
#define WIN_DELAY           5.0f
#define WIN_SHOW_DELAY      2.0f

#define BILLBOARD_YOFFSET   15.0f

#define PLAYER_MOVEMENT_SPEED 1.0f
#define AI_MOVEMENT_SPEED_MOD 0.3f

AF_Color WHITE_COLOR = {255, 255, 255, 255};
AF_Color PLAYER1_COLOR = {255, 0,0, 255};
AF_Color PLAYER2_COLOR = {0, 255,0, 255};
AF_Color PLAYER3_COLOR = {0, 0,255, 255};
AF_Color PLAYER4_COLOR = {255, 255,0, 255};


typedef struct
{
  PlyNum plynum;
  T3DMat4FP* modelMatFP;
  rspq_block_t *dplSnake;
  T3DAnim animAttack;
  T3DAnim animWalk;
  T3DAnim animIdle;
  T3DSkeleton skelBlend;
  T3DSkeleton skel;
  T3DVec3 moveDir;
  T3DVec3 playerPos;
  float rotY;
  float currSpeed;
  float animBlend;
  bool isAttack;
  bool isAlive;
  float attackTimer;
  PlyNum ai_target;
  int ai_reactionspeed;
} player_data;

player_data players[MAXPLAYERS];



// Player AI behaviours

float countDownTimer;
bool isEnding;
float endTimer;
PlyNum winner;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;



// Forward declare functions found in this implementation file
void PlayerController_DamageHandler(AppData* _appData);
void Scene_AIFollowEntity(AF_AI_Action* _aiAction);
void Scene_AIStateMachine(AF_AI_Action* _aiAction);

void PlayerController_DamageHandler(AppData* _appData){
    for(int i = 0; i < PLAYER_COUNT; ++i){
        AF_Entity* playerEntity = _appData->gameplayData.playerEntities[i];
        AF_CPlayerData* playerData = playerEntity->playerData;

        // are any players attacking
        if(playerData->isAttacking == TRUE){
            
            Vec3* playerPos = &playerEntity->transform->pos;
            // check if any players are close to each other in a radius
            for(int x = 0; x < PLAYER_COUNT; ++x){
                
                if (i != x){
                    // skip the player chekcing against itself
                    AF_Entity* otherPlayerEntity = _appData->gameplayData.playerEntities[x];
                    Vec3* otherPlayerPos = &otherPlayerEntity->transform->pos;
                    float playersInRange = Vec3_DISTANCE(*playerPos, *otherPlayerPos);
                  if(playersInRange < ATTACK_RADIUS){
                        // Other player is in range
                        // attack
                        AF_C3DRigidbody* otherPlayerRigidbody = otherPlayerEntity->rigidbody;

                        Vec3 forceVector = Vec3_MULT_SCALAR( Vec3_NORMALIZE((Vec3_MINUS(*playerPos, *otherPlayerPos))), -ATTACK_FORCE_STRENGTH);
                        AF_Physics_ApplyLinearImpulse(otherPlayerRigidbody, forceVector);
                    }
                }
            }
        }else{

        }
    }
}


void Scene_Awake(AppData* _appData){
    Scene_SetupEntities(_appData);
}

void Scene_Start(AppData* _appData){
    // TODO: get rid of global state, current bucket
    debugf("Scene: Scene Start: TODO: get rid of global state, current bucket");
    Scene_SpawnBucket(&g_currentBucket);
}

void Scene_Update(AppData* _appData){
    // handle restart/mainMenu
    if(_appData->gameplayData.gameState == GAME_STATE_GAME_RESTART){
        // do restart things
        // reset the player score
        for(int i = 0; i < PLAYER_COUNT; ++i){
            AF_Entity* entity = _appData->gameplayData.playerEntities[i];
            entity->playerData->score = 0;
            entity->playerData->isCarrying = FALSE;
            // reset the player posotions
            entity->transform->pos = entity->playerData->startPosition;
            villager1->playerData->isCarried = FALSE;
        }
        _appData->gameplayData.godEatCount = 0;
        _appData->gameplayData.gameState = GAME_STATE_MAIN_MENU;
        Scene_Start(_appData);
    }
    // TODO
    if(_appData->gameplayData.gameState == GAME_STATE_PLAYING){
        
        PlayerController_UpdateAllPlayerMovements(&_appData->input, *_appData->gameplayData.playerEntities, PLAYER_COUNT);
        // Check if any players are "attatcking" if yes, then deal damage.
        PlayerController_DamageHandler(_appData);

    }

    AF_ECS* ecs = &_appData->ecs;
    // carry villages
    for(int i = 0; i < ecs->entitiesCount; ++i){
        AF_Entity* entity = &ecs->entities[i];
        AF_CPlayerData* playerData = entity->playerData;

        
        // TODO: move this out of this function
        //if((AF_Component_GetHas(playerData->enabled) == TRUE) && (AF_Component_GetEnabled(playerData->enabled) == TRUE)){
        if(AF_Component_GetHas(playerData->enabled) == TRUE && playerData->isCarrying == TRUE){
            // make villager match player transform
            Vec3 villagerCarryPos = {entity->transform->pos.x, entity->transform->pos.y+VILLAGER_CARRY_HEIGHT, entity->transform->pos.z};
            GetVillager()->transform->pos = villagerCarryPos;
            
            //debugf("entity carrying villager: x: %f y: %f x: %f \n", villagerCarryPos.x, villagerCarryPos.y, villagerCarryPos.z);
        }

        
    }

    // Update the god count by summing the players scores
    
    int updatedTotalScore = 0;
    assert(_appData->gameplayData.playerEntities != NULL && "Scene_Update: player entities component is null\n");
    for(int i = 0; i < PLAYER_COUNT; ++i){
        AF_Entity* playerEntity = _appData->gameplayData.playerEntities[i];
        assert(playerEntity!= NULL && "Scene_Update: player entity component is null\n");
        AF_CPlayerData* playerData = playerEntity->playerData;
        assert(playerData != NULL && "Scene_Update: playerdata component is null\n");
        updatedTotalScore += playerData->score;
    }
    _appData->gameplayData.godEatCount = updatedTotalScore;

    
    // lock the y position to stop falling through ground.
	for(int i = 0; i < PLAYER_COUNT; ++i){
        AF_C3DRigidbody* rigidbody = _appData->gameplayData.playerEntities[i]->rigidbody;
        if((AF_Component_GetHas(rigidbody->enabled) == TRUE) && (AF_Component_GetEnabled(rigidbody->enabled) == TRUE)){
            AF_CTransform3D* transform = _appData->gameplayData.playerEntities[i]->transform;
            Vec3* pos = &transform->pos;
            Vec3 lockedYPosition = {pos->x, 0.0f, pos->z};
            transform->pos = lockedYPosition;
            
        // Lock players to remaining inside the game level bounds.
        // Quick AABB check of the level bounds 
            Vec3 levelBounds = _appData->gameplayData.levelBounds;
            Vec3 adjustedPlayerPos = Vec3_MINUS(*pos, _appData->gameplayData.levelPos);
            if (adjustedPlayerPos.x < -levelBounds.x ||
                adjustedPlayerPos.x > levelBounds.x ||
                adjustedPlayerPos.z < -levelBounds.z ||
                adjustedPlayerPos.z > levelBounds.z){
                    debugf("PlayerPos: x: %f, y: %f, z: %f, Level Bounds: x: %f, y: %f, z: %f \n", pos->x, pos->y, pos->z, levelBounds.z, levelBounds.y, levelBounds.z);
            
                    // zero out the velocity so we don't travel further 
                    Vec3 zeroVelocity = {0,0,0};
                    Vec3 negativeVelocity = Vec3_NORMALIZE(Vec3_MULT_SCALAR(rigidbody->velocity, -1.0f));
                    // remove y and make the amount to back back very very small amount
                    Vec3 removeY = {0.5f,0.0f,0.5f};
                    negativeVelocity = Vec3_MULT(negativeVelocity, removeY);
                    Vec3 positionBack = Vec3_ADD(transform->pos, negativeVelocity);
                    rigidbody->velocity = zeroVelocity;
                    transform->pos = positionBack;
            }
        }
	}
}

void Scene_LateUpdate(AppData* _appData){


    
}

void Scene_Destroy(AF_ECS* _ecs){

}

// Setup the games entities
void Scene_SetupEntities(AppData* _appData){
    // TODO: store these entities into a special struct just to hold pointers to these elements specifically
    AF_ECS* _ecs = &_appData->ecs;
    GameplayData* gameplayData = &_appData->gameplayData;
    int zeroInverseMass = 0.0f;
	// initialise the ecs system
	// Create Camera
	camera = AF_ECS_CreateEntity(_ecs);
	
    // TODO: read this data from a csv or xml file for quick and efficient setup
    // also useful for mapping to a ui editor
    // Create God
    
	Vec3 godPos = {0, 0, 0};
	Vec3 godScale = {2,2,2};
    Vec3 godBoundingScale = {1,1,1};
    godEntity = Entity_Factory_CreatePrimative(_ecs, godPos, godScale, AF_MESH_TYPE_MESH, AABB);
    godEntity->mesh->meshID = MODEL_SNAKE;
    godEntity->mesh->material.color = WHITE_COLOR;
    godEntity->rigidbody->inverseMass = zeroInverseMass;
    //godEntity->rigidbody->isKinematic = TRUE;
    godEntity->collider->collision.callback = Scene_OnGodTrigger;
    godEntity->collider->boundingVolume = godBoundingScale;
    godEntity->collider->showDebug = TRUE;
    *godEntity->skeletalAnimation = AF_CSkeletalAnimation_ADD();

    // setup animations
	// ---------Create Player1------------------
	Vec3 player1Pos = {2.0f, 1.5f, 1.0f};
	Vec3 player1Scale = {1.0f,1.0f,1.0f};
    gameplayData->playerEntities[0] = Entity_Factory_CreatePrimative(_ecs, player1Pos, player1Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player1Entity = gameplayData->playerEntities[0];
    player1Entity->mesh->meshID = MODEL_RAT;//MODEL_SNAKE2;
    player1Entity->mesh->material.color = WHITE_COLOR;
    player1Entity->rigidbody->inverseMass = 1.0f;
	player1Entity->rigidbody->isKinematic = TRUE;
    *player1Entity->playerData = AF_CPlayerData_ADD();
    player1Entity->playerData->startPosition = player1Pos;
    player1Entity->playerData->movementSpeed = PLAYER_MOVEMENT_SPEED;
    *player1Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();

    // Create Player1 Hat
    // position needs to be thought of as local to the parent it will be inherited by
    Vec3 player1HatPos = {0.0f, 1.5f, 1.0f};//Vec3_ADD(player1Entity->transform->pos, hatPos);
    Vec3 player1HatScale = {0.5f, 0.5f, 0.5f};
    AF_Entity* player1Hat = AF_ECS_CreateEntity(_ecs);
	//move the position up a little
	player1Hat->transform->localPos = player1HatPos;
    player1Hat->transform->localScale = player1HatScale;
    player1Hat->parentTransform = player1Entity->transform;
	// add a rigidbody to our cube
	*player1Hat->mesh = AF_CMesh_ADD();
	player1Hat->mesh->meshType = AF_MESH_TYPE_MESH;
    player1Hat->mesh->material.color = PLAYER1_COLOR;

    //player1Hat->parentTransform = player1Entity->transform;
    player1Hat->mesh->meshID = MODEL_BOX;//MODEL_SNAKE2;
    player1Hat->mesh->material.color = PLAYER1_COLOR;



    // Get AI Level
    //AI_MOVEMENT_SPEED_MOD * ((2-core_get_aidifficulty())*5 + rand()%((3-core_get_aidifficulty())*3));
    float aiReactionSpeed = AI_MOVEMENT_SPEED_MOD * ((1+core_get_aidifficulty()) + rand()%((1+core_get_aidifficulty())));
    
    // Create Player2
	Vec3 player2Pos = {-2.0f, 1.5f, 1.0f};
	Vec3 player2Scale = {1,1,1};
    
    gameplayData->playerEntities[1] = Entity_Factory_CreatePrimative(_ecs, player2Pos, player2Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player2Entity = gameplayData->playerEntities[1];
    player2Entity->mesh->meshID = MODEL_SNAKE;
    player2Entity->mesh->material.color = PLAYER2_COLOR;
    player2Entity->rigidbody->inverseMass = 1.0f;
	player2Entity->rigidbody->isKinematic = TRUE;
    *player2Entity->playerData = AF_CPlayerData_ADD();
    player2Entity->playerData->startPosition = player2Pos;
    player2Entity->playerData->movementSpeed = aiReactionSpeed;
    *player2Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();
    
    

    // Create Player3
	Vec3 player3Pos = {-2.0f, 1.5f, -1.0f};
	Vec3 player3Scale = {.75f,.75f,.75f};
    
    gameplayData->playerEntities[2] = Entity_Factory_CreatePrimative(_ecs, player3Pos, player3Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player3Entity = gameplayData->playerEntities[2];
    player3Entity->mesh->meshID = MODEL_SNAKE;
    player3Entity->mesh->material.color = PLAYER3_COLOR;
	player3Entity->rigidbody->isKinematic = TRUE;
    player3Entity->rigidbody->inverseMass = 1.0f;
    *player3Entity->playerData = AF_CPlayerData_ADD();
    player3Entity->playerData->startPosition = player3Pos;
    player3Entity->playerData->movementSpeed = aiReactionSpeed;
    *player3Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();
    

    // Create Player4
	Vec3 player4Pos = {2.0f, 1.5f, -1.0f};
	Vec3 player4Scale = {1,1,1};
    
    gameplayData->playerEntities[3] = Entity_Factory_CreatePrimative(_ecs, player4Pos, player4Scale, AF_MESH_TYPE_MESH, AABB);
	AF_Entity* player4Entity = gameplayData->playerEntities[3];
    player4Entity->mesh->meshID = MODEL_SNAKE;
    player4Entity->mesh->material.color = PLAYER4_COLOR;
	player4Entity->rigidbody->isKinematic = TRUE;
    player4Entity->rigidbody->inverseMass = 1.0f;
    *player4Entity->playerData = AF_CPlayerData_ADD();
    player4Entity->playerData->startPosition = player4Pos;
    player4Entity->playerData->movementSpeed = aiReactionSpeed;
    *player4Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();


    // assign AI to other players based on the players choosen in the game jam menu
    // skip the first player as its always controllable
    for(int i = core_get_playercount(); i < PLAYER_COUNT; ++i){
        AF_Entity* aiPlayerEntity = _appData->gameplayData.playerEntities[i];
        *aiPlayerEntity->aiBehaviour = AF_CAI_Behaviour_ADD();
        AI_CreateFollow_Action(aiPlayerEntity, player1Entity,  Scene_AIStateMachine);
    }
    

	//=========ENVIRONMENT========
    Vec3 levelMapPos = {0, 0, 2};
	Vec3 levelMapScale = {2,1,1};
    levelMapEntity = Entity_Factory_CreatePrimative(_ecs, levelMapPos, levelMapScale, AF_MESH_TYPE_MESH, AABB);
    levelMapEntity->mesh->meshID = MODEL_MAP;
    // disable the map collider
    AF_Component_SetHas(levelMapEntity->collider->enabled, FALSE);
	levelMapEntity->rigidbody->inverseMass = zeroInverseMass;
    //levelMapEntity->rigidbody->isKinematic = TRUE;
    Vec3 mapBoundingVolume = {7,.1, 3.5};
    _appData->gameplayData.levelPos = levelMapPos;
    _appData->gameplayData.levelBounds = mapBoundingVolume;
    levelMapEntity->collider->boundingVolume = mapBoundingVolume;
    // ============Buckets=============
    // Bucket 1
    // World pos and scale for bucket
	Vec3 bucket1Pos = {-5.0f, .5f, 0.0f};
	Vec3 bucket1Scale = {1,1,1};
    bucket1 = Entity_Factory_CreatePrimative(_ecs, bucket1Pos, bucket1Scale,AF_MESH_TYPE_MESH, AABB);
    bucket1->mesh->meshID = MODEL_BOX;
    bucket1->rigidbody->inverseMass = zeroInverseMass;

    // TODO: add details to scene_onBucketTrigger callback
    bucket1->collider->collision.callback = Scene_OnBucket1Trigger;
    // Bucket 2
    // World pos and scale for bucket
	Vec3 bucket2Pos = {5.0f, .5f, 0.0f};
	Vec3 bucket2Scale = {1,1,1};
	bucket2 = Entity_Factory_CreatePrimative(_ecs, bucket2Pos, bucket2Scale,AF_MESH_TYPE_MESH, AABB);
    bucket2->mesh->meshID = MODEL_BOX;
    bucket2->rigidbody->inverseMass = zeroInverseMass;
     // TODO: add details to scene_onBucketTrigger callback
    bucket2->collider->collision.callback = Scene_OnBucket2Trigger;

    // Bucket 3
    // World pos and scale for bucket
	Vec3 bucket3Pos = {5.0f, .5f, 5.0f};
	Vec3 bucket3Scale = {1,1,1};
	bucket3 = Entity_Factory_CreatePrimative(_ecs, bucket3Pos, bucket3Scale,AF_MESH_TYPE_MESH, AABB);
    bucket3->mesh->meshID = MODEL_BOX;
    bucket3->rigidbody->inverseMass = zeroInverseMass;
     // TODO: add details to scene_onBucketTrigger callback
    bucket3->collider->collision.callback = Scene_OnBucket3Trigger;
    // Bucket 4
    // World pos and scale for bucket
	Vec3 bucket4Pos = {-5.0f, .5f, 5.0f};
	Vec3 bucket4Scale = {1,1,1};
	bucket4 = Entity_Factory_CreatePrimative(_ecs, bucket4Pos, bucket4Scale,AF_MESH_TYPE_MESH, AABB);
    bucket4->mesh->meshID = MODEL_BOX;
    bucket4->rigidbody->inverseMass = zeroInverseMass;
     // TODO: add details to scene_onBucketTrigger callback
    bucket4->collider->collision.callback = Scene_OnBucket4Trigger;

    /// Villages
	Vec3 villager1Pos = {-1000.0f, 0, 0};
	Vec3 villager1Scale = {1,1,1};
    villager1 = Entity_Factory_CreatePrimative(_ecs, villager1Pos, villager1Scale, AF_MESH_TYPE_MESH, AABB);
    villager1->mesh->meshID = MODEL_FOOD;
	villager1->rigidbody->inverseMass = zeroInverseMass;
	villager1->rigidbody->isKinematic = TRUE;
    villager1->collider->collision.callback = Scene_OnCollision;
    /*

   float wallHeight = 3;
    // Create Left Wall
	Vec3 leftWallPos = {-8, 0, 3};
	Vec3 leftWallScale = {1,wallHeight,10};
	leftWall = Entity_Factory_CreatePrimative(_ecs, leftWallPos, leftWallScale,AF_MESH_TYPE_MESH, AABB);
    leftWall->mesh->meshID = MODEL_BOX;
    leftWall->rigidbody->inverseMass = zeroInverseMass;

    // Create Right Wall
	Vec3 rightWallPos = {8, 0, 3};
	Vec3 rightWallScale = {1,wallHeight,10};
	rightWall = Entity_Factory_CreatePrimative(_ecs, rightWallPos, rightWallScale,AF_MESH_TYPE_MESH, AABB);
    rightWall->mesh->meshID = MODEL_BOX;
    rightWall->rigidbody->inverseMass = zeroInverseMass;

    // Create Back Wall
	Vec3 backWallPos = {0, 0, -1.5};
	Vec3 backWallScale = {18, wallHeight, 0};
	backWall = Entity_Factory_CreatePrimative(_ecs, backWallPos, backWallScale, AF_MESH_TYPE_MESH, AABB);
    backWall->mesh->meshID = MODEL_BOX;
    backWall->rigidbody->inverseMass = zeroInverseMass;

    // Create Front Wall
	Vec3 frontWallPos = {0, 0, 7};
	Vec3 frontWallScale = {8,wallHeight,1};
	frontWall = Entity_Factory_CreatePrimative(_ecs, frontWallPos, frontWallScale,AF_MESH_TYPE_MESH, AABB);
    frontWall->mesh->meshID = MODEL_BOX;
    frontWall->rigidbody->inverseMass = zeroInverseMass;
    */

	// Scale everything due to strange model sizes exported from blender
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        // scale everything
        _ecs->transforms[i].scale = Vec3_MULT_SCALAR(_ecs->transforms[i].scale, .0075f);//0.075f);
    }
}



// ======== SPAWNERS ==========
// TODO: add better control flow
void Scene_SpawnBucket(uint8_t* _currentBucket){
    int upper = 4;
    int lower = 0;
    int randomNum = (rand() % (upper + - lower) + lower);
    // don't let the random number go above 4, as we count from 0
    if(randomNum >=4){
        randomNum = 3;
    }

    if(godEntity == NULL || bucket1 == NULL || bucket2 == NULL || bucket3 == NULL || bucket4 == NULL)
    {   debugf("Game: SpawnBucket: god or bucket entity is null \n");
        return;
    }

    AF_Entity* bucket1 = GetBucket1();
    AF_Entity* bucket2 = GetBucket2();
    AF_Entity* bucket3 = GetBucket3();
    AF_Entity* bucket4 = GetBucket4();

    

    if(randomNum == 0){
        *_currentBucket = 0;
        Vec3 bucket1Pos = {bucket1->transform->pos.x, bucket1->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket1->transform->pos.z};
        villager1->transform->pos = bucket1Pos;

    }else if( randomNum == 1){
        *_currentBucket = 1;
        Vec3 bucket2Pos = {bucket2->transform->pos.x, bucket2->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket2->transform->pos.z};
        villager1->transform->pos = bucket2Pos;

    }else if( randomNum == 2){
        *_currentBucket = 2;
        Vec3 bucket3Pos = {bucket3->transform->pos.x, bucket3->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket3->transform->pos.z};
        villager1->transform->pos = bucket3Pos;

    }else if( randomNum == 3){
        *_currentBucket = 3;
        Vec3 bucket4Pos = {bucket4->transform->pos.x, bucket4->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket4->transform->pos.z};
        villager1->transform->pos = bucket4Pos;
    }
}

// ======== TRIGGERS =========

/*
Scene_OnTrigger
Default collision callback to be used by game entities
*/
void Scene_OnTrigger(AF_Collision* _collision){
    
}

/*
Scene_OnGodTrigger
Callback Behaviour triggered when the player dropps off a sacrafice to the gods
*/
void Scene_OnGodTrigger(AF_Collision* _collision){
	AF_Entity* entity1 =  _collision->entity1;
	BOOL hasPlayerData1 = AF_Component_GetHas(entity1->playerData->enabled);

    AF_Entity* entity2 =  _collision->entity2;
	BOOL hasPlayerData2 = AF_Component_GetHas(entity2->playerData->enabled);
    
    // figure out which collision is a playable character
    AF_Entity* collidedEntity;
    if(hasPlayerData1 == TRUE){
        collidedEntity = entity1;
    }else if (hasPlayerData2 == TRUE){
        collidedEntity = entity2;
    }else{
        return;
    }

    //== FALSE && hasPlayerData2 == FALSE){


    // if entity is carrying, eat and shift the villager into the distance
    if(collidedEntity->playerData->isCarrying == TRUE){
        debugf("trigger god eat entity: %lu \n", AF_ECS_GetID(collidedEntity->id_tag));
       
        //debugf("Scene_GodTrigger: eat count %i \n", godEatCount);
        // TODO: update godEatCount. observer pattern would be nice right now
        //godEatCount++;
        
        // update the player who collided with gods score.
        collidedEntity->playerData->score ++;
        

        collidedEntity->playerData->isCarrying = FALSE;
        collidedEntity->playerData->carryingEntity = 0;
        Vec3 poolLocation = {100, 0,0};
        villager1->transform->pos = poolLocation;
        villager1->playerData->isCarried = FALSE;
        // randomly call for a colour bucket
        Scene_SpawnBucket(&g_currentBucket);
        // play sound
        // clear the players from carrying
    }
    
	// Play sound
}




/*
Scene_BucketCollisionBehaviour
Perform gameplay logic if bucket has been collided with by a player character
*/
void Scene_BucketCollisionBehaviour(int _currentBucket, int _bucketID, AF_Collision* _collision, AF_Entity* _villager, AF_Entity* _godEntity){
    
     //debugf("Scene_BucketCollisionBehaviour \n ");
    // Don't react if this bucket isn't activated
    if(_currentBucket != _bucketID){
        return;
    }

    //AF_Entity* entity1 =  _collision->entity1;
	AF_Entity* entity2 =  _collision->entity2;

    // Second collision is the playable character
    // skip if collision object doesn't have player data
    AF_CPlayerData* playerData2 = entity2->playerData;
    if(AF_Component_GetHas(playerData2->enabled) == FALSE){
        return;
    }
    
	
   
        // attatch the villager to this player
        if(_villager->playerData->isCarried == FALSE){
            playerData2->carryingEntity = _villager->id_tag;
            _villager->mesh->material.textureID = _godEntity->mesh->material.textureID;
            debugf("carry villager \n");
            playerData2->isCarrying = TRUE;
            _villager->playerData->isCarried = TRUE;
        }
}

/*
Scene_OnBucketTrigger
Trigger callback assigned to buckets in the game world
*/
void Scene_OnBucket1Trigger(AF_Collision* _collision){
    int bucketID = 0;
    if(g_currentBucket != bucketID){
        return;
    }
    
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}

void Scene_OnBucket2Trigger(AF_Collision* _collision){
    int bucketID = 1;
    if(g_currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}

void Scene_OnBucket3Trigger(AF_Collision* _collision){
    int bucketID = 2;
    if(g_currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}

void Scene_OnBucket4Trigger(AF_Collision* _collision){
    int bucketID = 3;
    if(g_currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(g_currentBucket, bucketID, _collision, villager1, godEntity);
}




/*===============
Scene_OnCollision
Default collision call.
================*/
void Scene_OnCollision(AF_Collision* _collision){
	if(_collision == NULL){
		debugf("Game: Scene_OnCollision: passed null collision object\n");
		return;
	}
	if(_collision->entity1 == NULL){
		debugf("Game: Scene_OnCollision: entity 1 is null\n");
		return;
	}
	AF_Entity* entity = (AF_Entity*)_collision->entity1;
	if(entity == NULL){
		debugf("Game: Scene_OnCollision: entity is null\n");
		return;
	}
	AF_CCollider* collider = entity->collider;
	if(collider == NULL){
		debugf("Game: Scene_OnCollision: collider is null\n");
		return;
	}
	// do collision things
}


// ======== GETTERS & SETTERS ==========

AF_Entity* GetVillager(){
    return villager1;
}

AF_Entity* GetGodEntity(){
    return godEntity;
}

/*
AF_Entity* GetPlayerEntity(uint8_t _index){
    if(_index >= PLAYER_COUNT)
    {
        debugf("Scene: GetPlayerEntity: passed index out of range %i \n", _index);
        return NULL;
    }
    return playerEntities[_index];
}*/

void Scene_AIStateMachine(AF_AI_Action* _aiAction){
    // We need to 
    assert(_aiAction != NULL);

    AF_Entity* sourceEntity = (AF_Entity*)_aiAction->sourceEntity;
    AF_Entity* targetEntity = (AF_Entity*)_aiAction->targetEntity;
    assert(sourceEntity != NULL);
    assert(targetEntity != NULL);

    AF_CPlayerData* sourceEntityPlayerData = sourceEntity->playerData;
    if(sourceEntityPlayerData->isCarrying){
        // change the AI action to head towards god
        _aiAction->sourceEntity = sourceEntity;
        // TODO: don't like this. need better control flow
        _aiAction->targetEntity = godEntity;
    }else{
        // change the AI action to head towards food
        _aiAction->sourceEntity = sourceEntity;
        // TODO: don't like this. need better control flow
        _aiAction->targetEntity = villager1;
    }

    Scene_AIFollowEntity(_aiAction);
}


// AI behaviour tester function
// take in a void* that we know will be a FollowBehaviour struct
// this allows our original callback to be generically re-used
// But its hella risky. YOLO
void Scene_AIFollowEntity(AF_AI_Action* _aiAction){
    // We need to 
    assert(_aiAction != NULL);

    AF_Entity* sourceEntity = (AF_Entity*)_aiAction->sourceEntity;
    AF_Entity* targetEntity = (AF_Entity*)_aiAction->targetEntity;
    assert(sourceEntity != NULL);
    assert(targetEntity != NULL);
    // Move towards 
    Vec3 startPos = sourceEntity->transform->pos;
    Vec3 destination = targetEntity->transform->pos;
    
    // stap at distance from object
    float stopAtDistance = 0.5f;
    float distance = Vec3_DISTANCE(startPos, destination);

    // move towards the player until close enough
    if(distance > stopAtDistance){
        Vec3 movementDirection = Vec3_NORMALIZE(Vec3_MINUS(startPos, destination));
        Vec2 movementDirection2D = {-movementDirection.x, movementDirection.z};
        //player->transform->pos = Vec3_Lerp(startPos, destination, playerSpeed);
        // create a direction vector to use
        PlayerController_UpdatePlayerMovement(movementDirection2D, sourceEntity);
    }
    
    // data needed:
    //  - gameState
    //  - current player ID
    //  - the other players
    //  - AI difficulty
    //  - Speed
    //  - Villager Location
    //  - god location
    //  - AI player carrying
    //  - Current player carrying and their state
    //  - distance to other players
    // if game is playing
    // Head towards the available food
        // speed * AI difficult
    // if we have food, head towards god
        // speed * AI difficult
    // if the food is held by a human, head towards the human
        // speed * AI difficult
    // if we are close to any player that is holding the food
        // Attack * AI difficulty
}


AF_Entity* GetBucket1(){
    return bucket1;
}

AF_Entity* GetBucket2(){
    return bucket2;
}

AF_Entity* GetBucket3(){
    return bucket3;
}

AF_Entity* GetBucket4(){
    return bucket4;
}