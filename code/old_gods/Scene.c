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
#include "AF_Renderer.h"
#include "AI.h"

// SCENE config vars
#define SCENE_MODEL_SCALE_FACTOR .0075f

// ECS system
AF_Entity* camera = NULL;

// God
AF_Entity* levelMapEntity = NULL;
AF_Entity* levelEntity = NULL;
AF_Entity* godSnekEntity = NULL;
AF_Entity* mapSeaFoamEntity = NULL;

// Environment Trimmings
//AF_Entity* alterEntity = NULL;
//AF_Entity* graveEntity = NULL;
//AF_Entity* smokeEntity = NULL;
AF_Entity* ratSpawnEntity = NULL;

// Island Spawn points
AF_Entity* bucket1 = NULL;
AF_Entity* bucket2 = NULL;
AF_Entity* bucket3 = NULL;
AF_Entity* bucket4 = NULL;

// Rat
//AF_Entity* rat1 = NULL;

// Shark
AF_Entity* sharkEntity = NULL;
AF_Entity* sharkTrail = NULL;

// Shark Hunters - 1 for each player ;)
AF_Entity* sharkHunterEntities[PLAYER_COUNT] = {NULL, NULL, NULL, NULL};
AF_Entity* sharkHunterTrails[PLAYER_COUNT] = {NULL, NULL, NULL, NULL};
AF_Entity* sharkHomeEntity = NULL;

// player trails
// TODO: pop this into an array
AF_Entity* player1Trail = NULL;
AF_Entity* player2Trail = NULL;
AF_Entity* player3Trail = NULL;
AF_Entity* player4Trail = NULL;

// player attackWaves
AF_Entity* playerWaves[PLAYER_COUNT] = {NULL, NULL, NULL, NULL};
AF_Entity* playerEatenWave = NULL;

// Gameplay Vars
uint8_t currentBucket = 0;
AF_Entity* g_ratPtr = NULL;

#define VILLAGER_CARRY_HEIGHT 1
#define ATTACK_RADIUS       5.0f
#define ATTACK_FORCE_STRENGTH 10.0f
#define PLAYER_MOVEMENT_SPEED 1.0f
#define AI_MOVEMENT_SPEED_MOD 0.075f
#define AI_TARGET_DISTANCE 1.0f
#define RAT_MOVEMENT_SPEED 0.5f

// FACTION Colours
AF_Color WHITE_COLOR = {255, 255, 255, 255};
AF_Color SAND_COLOR = {255, 245, 177};
AF_Color PLAYER1_COLOR = {233, 82, 129, 255};
AF_Color PLAYER2_COLOR = {200, 233, 82, 255};
AF_Color PLAYER3_COLOR = {82, 200, 223, 255};
AF_Color PLAYER4_COLOR = {255, 233, 0, 255};


// ======== AUDIO ========
wav64_t feedGodSoundFX;
wav64_t pickupSoundFX;



// Forward declare functions found in this implementation file
// ==== AI =====
void Scene_AIStateMachine(AF_AI_Action* _aiAction);
void RatAIBehaviourUpdate(AppData* _appData);
void SharkAIBehaviourUpdate(AppData* _appData);
void Scene_AIFollowEntity(AF_AI_Action* _aiAction);

// ==== Triggers / Collisions 
void OnRatCollision(AF_Collision* _collision);
void OnSharkCollision(AF_Collision* _collision);

// ==== FX Updates ====
void Scene_UpdateWaterTrails(AppData* appData);
void Scene_UpdatePlayerAttackWaves(AppData* _appData);
void Scene_UpdateEffects(AppData* _appData);
void Scene_UpdateSharkEffects(AppData* _appData);

// ==== SPAWNING ====
void RespawnPlayer(AF_Entity* _entity);
void RespawnRat();

// ==== Setup ====
void Scene_SetupEntities(AppData* _appData);
void Scene_SetupSharks(AppData* _appData);
void Scene_SetupLevelObjects(AppData* _appData);
void Scene_Setup_SnekGod(AppData* _appData);
void Scene_Setup_LevelMap(AppData* _appData);
void Scene_Setup_Players(AppData* _appData);
void Scene_Setup_RatSpawnPoints(AppData* _appData);
void Scene_Setup_Sharks(AppData* _appData);
void Scene_Setup_Rats(AppData* _appData);
void Scene_Setup_Audio(AppData* _appData);

// ==== GAMEPLAY ====
void TogglePrimativeComponents(AF_Entity* _entity, BOOL _state);
void PlayerController_DamageHandler(AppData* _appData);
void MonitorPlayerHealth(AppData* _appData);

// Buckets
void Scene_SpawnBucket(uint8_t* _currentBucket);
AF_Entity* GetBucket1();
AF_Entity* GetBucket2();
AF_Entity* GetBucket3();
AF_Entity* GetBucket4();

// Triggers
void Scene_OnTrigger(AF_Collision* _collision);
void Scene_OnGodTrigger(AF_Collision* _collision);
void Scene_OnBucket1Trigger(AF_Collision* _collision);
void Scene_OnBucket2Trigger(AF_Collision* _collision);
void Scene_OnBucket3Trigger(AF_Collision* _collision);
void Scene_OnBucket4Trigger(AF_Collision* _collision);

void Scene_OnCollision(AF_Collision* _collision);
void Scene_BucketCollisionBehaviour(int _currentBucket, int _bucketID, AF_Collision* _collision);


// ================ SCENE GENERIC FUNCTIONS =================

/* ================
Scene_Awake
Inititial setup of entities
 ================ */
void Scene_Awake(AppData* _appData){
    Scene_SetupEntities(_appData);
}

/* ================
Scene_Start
Second step in init of scene
 ================ */
void Scene_Start(AppData* _appData){
    // TODO: get rid of global state, current bucket
    Scene_SpawnBucket(&currentBucket);
}


/* ================
Scene_Update
Scene gameplay update loop
// TODO: clean this up
 ================ */
void Scene_Update(AppData* _appData){
    AF_Entity* rat = _appData->gameplayData.rat;
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
            rat->playerData->isCarried = FALSE;
        }
        _appData->gameplayData.godEatCount = 0;
        _appData->gameplayData.gameState = GAME_STATE_MAIN_MENU;
        Scene_Start(_appData);
    }

    // TODO
    if(_appData->gameplayData.gameState == GAME_STATE_PLAYING){
        PlayerController_UpdateAllPlayerMovements(_appData);
        

        // Check if any players are "attatcking" if yes, then deal damage.
        PlayerController_DamageHandler(_appData);
        RatAIBehaviourUpdate(_appData);
        SharkAIBehaviourUpdate(_appData);
    }

    Scene_UpdateWaterTrails(_appData);
    Scene_UpdatePlayerAttackWaves(_appData);
    Scene_UpdateEffects(_appData);
    Scene_UpdateSharkEffects(_appData);

    AF_ECS* ecs = &_appData->ecs;
    // carry villages
    for(int i = 0; i < ecs->entitiesCount; ++i){
        AF_Entity* entity = &ecs->entities[i];
        AF_CPlayerData* playerData = entity->playerData;

        // TODO: move this out of this function
        //if((AF_Component_GetHas(playerData->enabled) == TRUE) && (AF_Component_GetEnabled(playerData->enabled) == TRUE)){
        // TODO let parent transform take care of this.
        if(AF_Component_GetHas(playerData->enabled) == TRUE && playerData->isCarrying == TRUE){
            // make villager match player transform
            Vec3 villagerCarryPos = {entity->transform->pos.x, entity->transform->pos.y+VILLAGER_CARRY_HEIGHT, entity->transform->pos.z};
            AF_Entity* rat = _appData->gameplayData.rat;
            rat->transform->pos = villagerCarryPos;
            
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
            AF_Entity* hunterShark = sharkHunterEntities[i];
            if (adjustedPlayerPos.x < -levelBounds.x ||
                adjustedPlayerPos.x > levelBounds.x ||
                adjustedPlayerPos.z < -levelBounds.z ||
                adjustedPlayerPos.z > levelBounds.z){
                
                // send shark to eat player
                hunterShark->aiBehaviour->enabled = AF_Component_SetEnabled(hunterShark->aiBehaviour->enabled, TRUE);
                hunterShark->playerData->isAttacking = TRUE;
                // Shark will target and move towards the player
                // on collision will handle the rest
                  
            }else{
                // disable the hunter shark ai behaviour
                //hunterShark->aiBehaviour->enabled = AF_Component_SetEnabled(hunterShark->aiBehaviour->enabled, FALSE);
                hunterShark->playerData->isAttacking = FALSE;
                // TODO: need a way to tell shark to target a distant location
            }
        }
	}

    MonitorPlayerHealth(_appData);
   
}

/* ================
Scene_LateUpdate
Do late update things.
 ================ */
void Scene_LateUpdate(AppData* _appData){
    // empty
}


/* ================
Scene_Destroy
Cleanup variables and memory
// TODO: check if anything else needs cleanup
 ================ */
void Scene_Destroy(AF_ECS* _ecs){
    debugf("Scene: Destroy\n");
    wav64_close(&feedGodSoundFX);
    wav64_close(&pickupSoundFX);
}

// ================ SCENE PLAYER CONTROLLER FUNCTIONS =================

/* ================
PlayerController_DamageHandler
TODO: move to player controller 
Check for player in range of other players and if attack.
Then do attacking things like force push other players in the opposite direction
 ================ */
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
        }
    }
}

// ================ SCENE VISUAL EFFECTS =================

/* ================
Scene_UpdateSharkEffects
Update the visual effects e.g. shark trails
 ================ */
void Scene_UpdateSharkEffects(AppData* _appData){
    
    // turn the shark trails on or off depending if they are visible
    for(int i = 0; i < PLAYER_COUNT; ++i){
        Vec3 levelBounds = _appData->gameplayData.levelBounds;
            Vec3 pos = sharkHunterEntities[i]->transform->pos;
            Vec3 adjustedSharkPos = Vec3_MINUS(pos, _appData->gameplayData.levelPos);
            //AF_Entity* hunterShark = sharkHunterEntities[i];
            AF_CMesh* sharkMesh = sharkHunterEntities[i]->mesh;
            AF_CMesh* sharkTrail = sharkHunterTrails[i]->mesh;

            if(_appData->gameplayData.gameState != GAME_STATE_PLAYING)
            {
                sharkTrail->enabled = AF_Component_SetEnabled(sharkTrail->enabled, FALSE);
                continue;
            }
            // if inside the bounds, turn on the visibilty of the shark and trails, otherwise turn off.
            if (adjustedSharkPos.x < -levelBounds.x - 5 ||
                adjustedSharkPos.x > levelBounds.x+ 5  ||
                adjustedSharkPos.z < -levelBounds.z- 5  ||
                adjustedSharkPos.z > levelBounds.z+ 5 ){
                
                sharkTrail->enabled = AF_Component_SetEnabled(sharkTrail->enabled, FALSE);
                sharkMesh->enabled = AF_Component_SetEnabled(sharkMesh->enabled, FALSE);
            
            }else{
                sharkMesh->enabled = AF_Component_SetEnabled(sharkMesh->enabled, TRUE);
                sharkTrail->enabled = AF_Component_SetEnabled(sharkTrail->enabled, TRUE);
            }
    }
}

/* ================
Scene_UpdateEffects
Update the visual effects e.g. player eaten waves, spawning the pentagon ect.
 ================ */
void Scene_UpdateEffects(AppData* _appData){
    // TODO: get these working
    /*
    // update the smoke
    AF_CMesh* smokeMesh = smokeEntity->mesh;
    if(smokeMesh->material.color.a - 10.0f <= 0){
        smokeMesh->material.color.a = 0.0f;
    }else{
        smokeMesh->material.color.a -= 10.0f;
    }

    // update the spawn pentogram
    AF_CMesh* spawnPentagramMesh = ratSpawnEntity->mesh;
    if(spawnPentagramMesh->material.color.a - 10.0f <= 0){
        spawnPentagramMesh->material.color.a = 0.0f;
    }else{
        spawnPentagramMesh->material.color.a -= 10.0f;
    }*/

   // TODO: disable this object once it can't be seen
    // update the eaten attack wave
    // if we are not playing, don't animate. helps with poor performance when menu is shown
    AF_CMesh* eatenWaveMesh = playerEatenWave->mesh;
    AF_CMesh* mapSeaFoamEntityMesh = mapSeaFoamEntity->mesh;
    if(_appData->gameplayData.gameState != GAME_STATE_PLAYING)
    {
        eatenWaveMesh->enabled = AF_Component_SetEnabled(eatenWaveMesh->enabled, FALSE);
        mapSeaFoamEntityMesh->enabled = AF_Component_SetEnabled(mapSeaFoamEntityMesh->enabled, FALSE);
        return;
    }else{
        eatenWaveMesh->enabled = AF_Component_SetEnabled(eatenWaveMesh->enabled, TRUE);
        mapSeaFoamEntityMesh->enabled = AF_Component_SetEnabled(mapSeaFoamEntityMesh->enabled, TRUE);
    }
    
    if(eatenWaveMesh->material.color.a - 10.0f <= 0){
        eatenWaveMesh->material.color.a = 0.0f;
        // disable the renderer when we are finished
        eatenWaveMesh->enabled = AF_Component_SetEnabled(eatenWaveMesh->enabled, FALSE);
    }else{
        eatenWaveMesh->enabled = AF_Component_SetEnabled(eatenWaveMesh->enabled, TRUE);
        eatenWaveMesh->material.color.a -= 10.0f;
        
    }

}

/* ================
Scene_UpdatePlayerAttackWaves
Update the visual effects e.g. player attack waves
 ================ */
void Scene_UpdatePlayerAttackWaves(AppData* _appData){
    // TODO: disable this object once it can't be seen
    for(int i = 0; i < PLAYER_COUNT; ++i){
        AF_CMesh* mesh = playerWaves[i]->mesh;
        // disable whilst not playing to improve performance
        if(_appData->gameplayData.gameState != GAME_STATE_PLAYING)
            {
                mesh->enabled = AF_Component_SetEnabled(mesh->enabled, FALSE);
                continue;
            }

        if(_appData->gameplayData.playerEntities[i]->playerData->isAttacking == TRUE){
            mesh->material.color.a = 255;
        }else{
            // it will solwly fade away
             // don't overflow
            if(mesh->material.color.a - 10.0f <= 0){
                // disable the renderer when we are finished
                mesh->enabled = AF_Component_SetEnabled(mesh->enabled, FALSE);
                mesh->material.color.a = 0.0f;
            }else{
                mesh->enabled = AF_Component_SetEnabled(mesh->enabled, TRUE);
                mesh->material.color.a -= 10.0f;
            }
            
            //playerWaves[i]->mesh->enabled = AF_Component_SetEnabled(playerWaves[i]->mesh->enabled, FALSE);
        }
    }
}

/* ================
Scene_UpdateWaterTrails
Update the visual effects e.g. player water trails as it moves
Trails appear based on the player velocity
 ================ */
void Scene_UpdateWaterTrails(AppData* _appData){
    // Player 1
    AF_C3DRigidbody* player1Rigidbody = _appData->gameplayData.playerEntities[0]->rigidbody;
    AF_CMesh* player1TrailMesh = player1Trail->mesh; 
    float foamTransparency1 = Vec3_MAGNITUDE(player1Rigidbody->velocity) / 25;
    // normalised 0-1 will mult against color range of alpha value
    player1TrailMesh->material.color.a = foamTransparency1 * 255;
    if(player1TrailMesh->material.color.a == 0.1f || _appData->gameplayData.gameState != GAME_STATE_PLAYING){
        // disable the renderer when we are finished
        player1TrailMesh->enabled = AF_Component_SetEnabled(player1TrailMesh->enabled, FALSE);
    }else{
        player1TrailMesh->enabled = AF_Component_SetEnabled(player1TrailMesh->enabled, TRUE);
    }

    // Player 2
    AF_C3DRigidbody* player2Rigidbody = _appData->gameplayData.playerEntities[1]->rigidbody;
    AF_CMesh* player2TrailMesh = player2Trail->mesh; 
    float foamTransparency2 = Vec3_MAGNITUDE(player2Rigidbody->velocity) / 25;
    // normalised 0-1 will mult against color range of alpha value
    player2TrailMesh->material.color.a = foamTransparency2 * 255;
    if(player2TrailMesh->material.color.a == 0.1f || _appData->gameplayData.gameState != GAME_STATE_PLAYING){
        // disable the renderer when we are finished
        player2TrailMesh->enabled = AF_Component_SetEnabled(player2TrailMesh->enabled, FALSE);
    }else{
        player2TrailMesh->enabled = AF_Component_SetEnabled(player2TrailMesh->enabled, TRUE);
    }

    // Player 3
    AF_C3DRigidbody* player3Rigidbody = _appData->gameplayData.playerEntities[2]->rigidbody;
    AF_CMesh* player3TrailMesh = player3Trail->mesh; 
    float foamTransparency3 = Vec3_MAGNITUDE(player3Rigidbody->velocity) / 25;
    // normalised 0-1 will mult against color range of alpha value
    player3TrailMesh->material.color.a = foamTransparency3 * 255;
    if(player3TrailMesh->material.color.a == 0.1f || _appData->gameplayData.gameState != GAME_STATE_PLAYING){
        // disable the renderer when we are finished
        player3TrailMesh->enabled = AF_Component_SetEnabled(player3TrailMesh->enabled, FALSE);
    }else{
        player3TrailMesh->enabled = AF_Component_SetEnabled(player3TrailMesh->enabled, TRUE);
    }

    // Player 4
    AF_C3DRigidbody* player4Rigidbody = _appData->gameplayData.playerEntities[3]->rigidbody;
    AF_CMesh* player4TrailMesh = player4Trail->mesh; 
    float foamTransparency4 = Vec3_MAGNITUDE(player4Rigidbody->velocity) / 25;
    // normalised 0-1 will mult against color range of alpha value
    player4TrailMesh->material.color.a = foamTransparency4 * 255;
    if(player4TrailMesh->material.color.a == 0.1f || _appData->gameplayData.gameState != GAME_STATE_PLAYING){
        // disable the renderer when we are finished
        player4TrailMesh->enabled = AF_Component_SetEnabled(player4TrailMesh->enabled, FALSE);
    }else{
        player4TrailMesh->enabled = AF_Component_SetEnabled(player4TrailMesh->enabled, TRUE);
    }
}


// ================ SCENE SETUP FUNCTIONS =================
/* ================
Scene_SetupEntities
General Setup function, that will call other setup functions
 ================ */
void Scene_SetupEntities(AppData* _appData){
    // TODO: store these entities into a special struct just to hold pointers to these elements specifically
    AF_ECS* _ecs = &_appData->ecs;
	// initialise the ecs system
	// Create Camera
	camera = AF_ECS_CreateEntity(_ecs);
	
    // TODO: read this data from a csv or xml file for quick and efficient setup
    // also useful for mapping to a ui editor

    // ==== LEVEL MAP ====
	Scene_Setup_LevelMap(_appData);

    // ==== SNEK GOD ====
    Scene_Setup_SnekGod(_appData);

    // ======== RATS ========
    Scene_Setup_Rats(_appData);
    
    // ======== PLAYERS ========
    Scene_Setup_Players(_appData);
    
	//========= SPAWN SPOTS ========
    Scene_Setup_RatSpawnPoints(_appData);
    
    // ==== SHARKS ====
    Scene_SetupSharks(_appData);
    
    // ==== MISC LEVEL ENTITIES ====
    Scene_SetupLevelObjects(_appData);
    
    // ======== AUDIO ========
    Scene_Setup_Audio(_appData);

    // TODO: figure out how to avoid this
	// Scale everything due to strange model sizes exported from blender
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        // scale everything
        _ecs->transforms[i].scale = Vec3_MULT_SCALAR(_ecs->transforms[i].scale, SCENE_MODEL_SCALE_FACTOR);
    }
}

/* ================
Scene_Setup_Audio
General Setup function, to setup audio effects
 ================ */
void Scene_Setup_Audio(AppData* _appData){
    wav64_open(&feedGodSoundFX,feedGodSoundFXPath);
    wav64_open(&pickupSoundFX, pickupSoundFXPath);
}

/* ================
Scene_Setup_SnekGod
General Setup function, to setup the snek god
 ================ */
void Scene_Setup_SnekGod(AppData* _appData){
    // Create rising god snek
    Vec3 godSnekPos = {0,-1,-3};
    Vec3 godSnekScale = {2,2,2};
    Vec3 godBoundingScale = {1,1,1};
    godSnekEntity = Entity_Factory_CreatePrimative(&_appData->ecs, godSnekPos, godSnekScale, AF_MESH_TYPE_MESH, AABB);
    godSnekEntity->mesh->meshID = MODEL_SNAKE;
    godSnekEntity->mesh->material.color = WHITE_COLOR;
    godSnekEntity->rigidbody->inverseMass = 0.0f;
    levelEntity->rigidbody->isKinematic = TRUE;
    godSnekEntity->collider->collision.callback = Scene_OnGodTrigger;
    godSnekEntity->collider->boundingVolume = godBoundingScale;
    godSnekEntity->collider->showDebug = TRUE;
    *godSnekEntity->skeletalAnimation = AF_CSkeletalAnimation_ADD();
}

/* ================
Scene_Setup_LevelMap
General Setup function, to setup the level map
 ================ */
void Scene_Setup_LevelMap(AppData* _appData){
    Vec3 levelPos = {0, .1, 0};
	Vec3 levelScale = {2,2,2};
    Vec3 godBoundingScale = {1,1,1};


    // Level Map
    Vec3 mapBoundingVolume = {15,0, 7.5};
    Vec3 levelMapPos = {0, 0, 0};
    
    
	Vec3 levelMapScale = {30.0f,15.0f,15.0f};
    levelMapEntity = Entity_Factory_CreatePrimative(&_appData->ecs, levelMapPos, levelMapScale, AF_MESH_TYPE_MESH, AABB);
    levelMapEntity->mesh->meshID = MODEL_MAP;
    levelMapEntity->mesh->material.color = WHITE_COLOR;
    // disable the map collider
    AF_Component_SetHas(levelMapEntity->collider->enabled, FALSE);
	levelMapEntity->rigidbody->inverseMass = 0.0f;
    //levelMapEntity->rigidbody->isKinematic = TRUE;
    levelMapEntity->collider->boundingVolume = mapBoundingVolume;
    
    /**/
    _appData->gameplayData.levelPos = levelMapPos;
    _appData->gameplayData.levelBounds = mapBoundingVolume;


    levelEntity = Entity_Factory_CreatePrimative(&_appData->ecs, levelPos, levelScale, AF_MESH_TYPE_MESH, AABB);
    levelEntity->mesh->enabled = AF_Component_SetHas(levelEntity->mesh->enabled, FALSE);
    levelEntity->mesh->enabled = AF_Component_SetEnabled(levelEntity->mesh->enabled, FALSE);
    //levelEntity->mesh->meshID = MODEL_SNAKE;
    //levelEntity->mesh->material.color = WHITE_COLOR;
    levelEntity->rigidbody->inverseMass = 0.0f;
    levelEntity->rigidbody->isKinematic = TRUE;
    levelEntity->collider->collision.callback = Scene_OnGodTrigger;
    levelEntity->collider->boundingVolume = godBoundingScale;
    levelEntity->collider->showDebug = TRUE;

    Vec3 godSeaFoamPos = {0.0f, 0.0f, 0.0f};
    Vec3 godSeaFoamScale = {30.0f, 1.0f, 15.0f};
    mapSeaFoamEntity = AF_ECS_CreateEntity(&_appData->ecs);
	//move the position up a little
    mapSeaFoamEntity->transform->pos = godSeaFoamPos;
    mapSeaFoamEntity->transform->scale = godSeaFoamScale;

	// add a rigidbody to our cube
	*mapSeaFoamEntity->mesh = AF_CMesh_ADD();
	mapSeaFoamEntity->mesh->meshType = AF_MESH_TYPE_MESH;
    mapSeaFoamEntity->mesh->material.color = WHITE_COLOR;
    mapSeaFoamEntity->mesh->material.color.a = 100;
    mapSeaFoamEntity->mesh->meshID = MODEL_FOAM;
    *mapSeaFoamEntity->animation = AF_CAnimation_ADD();
    mapSeaFoamEntity->animation->animationSpeed = 0.1f;
}

/* ================
Scene_Setup_Players
General Setup function, to setup the controllable players
 ================ */
void Scene_Setup_Players(AppData* _appData){
    // Player shared Vars
    float foamAnimationSpeed = -0.5f;
	// ---------Create Player1------------------
	Vec3 player1Pos = {2.0f, 0.0f, 1.0f};
	Vec3 player1Scale = {1.0f,1.0f,1.0f};
    _appData->gameplayData.playerEntities[0] = Entity_Factory_CreatePrimative(&_appData->ecs, player1Pos, player1Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player1Entity = _appData->gameplayData.playerEntities[0];
    player1Entity->mesh->meshID = MODEL_SNAKE;
    player1Entity->mesh->material.color = PLAYER1_COLOR;
    player1Entity->rigidbody->inverseMass = 0.0f;
	player1Entity->rigidbody->isKinematic = TRUE;
    *player1Entity->playerData = AF_CPlayerData_ADD();
    player1Entity->playerData->faction = PLAYER;
    player1Entity->playerData->startPosition = player1Pos;
    // For some reason player 1 is faster. not sure so halving the speed
    player1Entity->playerData->movementSpeed = PLAYER_MOVEMENT_SPEED * .25f;
    *player1Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();

    
    // Create Player1 Foam Trail
    // position needs to be thought of as local to the parent it will be inherited by
    Vec3 player1TrailPos = {0.0f, -.01f, 0.0f};
    Vec3 player1TrailScale = {1.5f, 1.0f, 1.5f};
    player1Trail = AF_ECS_CreateEntity(&_appData->ecs);
	//move the position up a little
	player1Trail->transform->localPos = player1TrailPos;
    player1Trail->transform->localScale = player1TrailScale;
    player1Trail->parentTransform = player1Entity->transform;
	// add a rigidbody to our cube
	*player1Trail->mesh = AF_CMesh_ADD();
	player1Trail->mesh->meshType = AF_MESH_TYPE_MESH;
    player1Trail->mesh->material.color = WHITE_COLOR;
    player1Trail->mesh->material.color.a = 0;
    player1Trail->mesh->meshID = MODEL_TRAIL;

    // Animation
    *player1Trail->animation = AF_CAnimation_ADD();
    player1Trail->animation->animationSpeed = foamAnimationSpeed;


    // Get AI Level
    //AI_MOVEMENT_SPEED_MOD * ((2-core_get_aidifficulty())*5 + rand()%((3-core_get_aidifficulty())*3));
    float aiReactionSpeed = AI_MOVEMENT_SPEED_MOD * ((1+core_get_aidifficulty()) + rand()%((1+core_get_aidifficulty())));
    
    // Create Player2
	Vec3 player2Pos = {-2.0f, 0.0f, 1.0f};
	Vec3 player2Scale = {1,1,1};
    
    _appData->gameplayData.playerEntities[1] = Entity_Factory_CreatePrimative(&_appData->ecs, player2Pos, player2Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player2Entity = _appData->gameplayData.playerEntities[1];
    player2Entity->mesh->meshID = MODEL_SNAKE;
    player2Entity->mesh->material.color = PLAYER2_COLOR;
    player2Entity->rigidbody->inverseMass =  0.0f;
	player2Entity->rigidbody->isKinematic = TRUE;
    *player2Entity->playerData = AF_CPlayerData_ADD();
    player2Entity->playerData->faction = PLAYER;
    player2Entity->playerData->movementSpeed = PLAYER_MOVEMENT_SPEED;
    player2Entity->playerData->startPosition = player2Pos;
    player2Entity->playerData->movementSpeed = aiReactionSpeed;
    *player2Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();

    // Create Player2 Foam Trail
    // position needs to be thought of as local to the parent it will be inherited by
    Vec3 player2TrailPos = {0.0f, -.01f, 0.0f};
    Vec3 player2TrailScale = {1.5f, 1.0f, 1.5f};
    player2Trail = AF_ECS_CreateEntity(&_appData->ecs);
	//move the position up a little
	player2Trail->transform->localPos = player2TrailPos;
    player2Trail->transform->localScale = player2TrailScale;
    player2Trail->parentTransform = player2Entity->transform;
	// add a rigidbody to our cube
	*player2Trail->mesh = AF_CMesh_ADD();
	player2Trail->mesh->meshType = AF_MESH_TYPE_MESH;
    player2Trail->mesh->material.color = WHITE_COLOR;
    player2Trail->mesh->material.color.a = 0;
    player2Trail->mesh->meshID = MODEL_TRAIL;

    // Animation
    *player2Trail->animation = AF_CAnimation_ADD();
    player2Trail->animation->animationSpeed = foamAnimationSpeed;
    
    // Create Player3
	Vec3 player3Pos = {-2.0f, 0.0f, -1.0f};
	Vec3 player3Scale = {.75f,.75f,.75f};
    
    _appData->gameplayData.playerEntities[2] = Entity_Factory_CreatePrimative(&_appData->ecs, player3Pos, player3Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* player3Entity = _appData->gameplayData.playerEntities[2];
    player3Entity->mesh->meshID = MODEL_SNAKE;
    player3Entity->mesh->material.color = PLAYER3_COLOR;
	player3Entity->rigidbody->isKinematic = TRUE;
    player3Entity->rigidbody->inverseMass =  0.0f;
    *player3Entity->playerData = AF_CPlayerData_ADD();
    player3Entity->playerData->movementSpeed = PLAYER_MOVEMENT_SPEED;
    player3Entity->playerData->faction = PLAYER;
    player3Entity->playerData->startPosition = player3Pos;
    player3Entity->playerData->movementSpeed = aiReactionSpeed;
    *player3Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();

    // Create Player3 Foam Trail
    // position needs to be thought of as local to the parent it will be inherited by
    Vec3 player3TrailPos = {0.0f, -.01f, 0.0f};
    Vec3 player3TrailScale = {1.5f, 1.0f, 1.5f};
    player3Trail = AF_ECS_CreateEntity(&_appData->ecs);
	//move the position up a little
	player3Trail->transform->localPos = player3TrailPos;
    player3Trail->transform->localScale = player3TrailScale;
    player3Trail->parentTransform = player3Entity->transform;
	// add a rigidbody to our cube
	*player3Trail->mesh = AF_CMesh_ADD();
	player3Trail->mesh->meshType = AF_MESH_TYPE_MESH;
    player3Trail->mesh->material.color = WHITE_COLOR;
    player3Trail->mesh->material.color.a = 0;
    player3Trail->mesh->meshID = MODEL_TRAIL;

    // animation
    *player3Trail->animation = AF_CAnimation_ADD();
    player3Trail->animation->animationSpeed = foamAnimationSpeed;


    // Create Player4
	Vec3 player4Pos = {2.0f, 0.0f, -1.0f};
	Vec3 player4Scale = {1,1,1};
    
    _appData->gameplayData.playerEntities[3] = Entity_Factory_CreatePrimative(&_appData->ecs, player4Pos, player4Scale, AF_MESH_TYPE_MESH, AABB);
	AF_Entity* player4Entity = _appData->gameplayData.playerEntities[3];
    player4Entity->mesh->meshID = MODEL_SNAKE;
    player4Entity->mesh->material.color = PLAYER4_COLOR;
	player4Entity->rigidbody->isKinematic = TRUE;
    player4Entity->rigidbody->inverseMass =  0.0f;
    *player4Entity->playerData = AF_CPlayerData_ADD();
    player4Entity->playerData->movementSpeed = PLAYER_MOVEMENT_SPEED;
    player4Entity->playerData->faction = PLAYER;
    player4Entity->playerData->startPosition = player4Pos;
    player4Entity->playerData->movementSpeed = aiReactionSpeed;
    *player4Entity->skeletalAnimation = AF_CSkeletalAnimation_ADD();

    // Create Player4 Foam Trail
    // position needs to be thought of as local to the parent it will be inherited by
    Vec3 player4TrailPos = {0.0f, -.01f, 0.0f};
    Vec3 player4TrailScale = {1.5f, 1.0f, 1.5f};
    player4Trail = AF_ECS_CreateEntity(&_appData->ecs);
	//move the position up a little
	player4Trail->transform->localPos = player4TrailPos;
    player4Trail->transform->localScale = player4TrailScale;
    player4Trail->parentTransform = player4Entity->transform;
	// add a rigidbody to our cube
	*player4Trail->mesh = AF_CMesh_ADD();
	player4Trail->mesh->meshType = AF_MESH_TYPE_MESH;
    player4Trail->mesh->material.color = WHITE_COLOR;
    player4Trail->mesh->material.color.a = 0;
    player4Trail->mesh->meshID = MODEL_TRAIL;

    // Animation
    *player4Trail->animation = AF_CAnimation_ADD();
    player4Trail->animation->animationSpeed = foamAnimationSpeed;


    // assign AI to other players based on the players choosen in the game jam menu
    // skip the first player as its always controllable
    for(int i = core_get_playercount(); i < PLAYER_COUNT; ++i){
        AF_Entity* aiPlayerEntity = _appData->gameplayData.playerEntities[i];
        *aiPlayerEntity->aiBehaviour = AF_CAI_Behaviour_ADD();
        //AI_CreateFollow_Action(aiPlayerEntity, player1Entity,  Scene_AIStateMachine);
        //BOOL hasAI = AF_Component_GetHas(aiPlayerEntity->aiBehaviour->enabled );
        //BOOL isEnabled = AF_Component_GetEnabled(aiPlayerEntity->aiBehaviour->enabled);
        //debugf("player hasAI: %i isEnabled: %i\n",hasAI, isEnabled);

        // First behaviour is to go for the villager
        AF_Entity* rat = _appData->gameplayData.rat;
        AI_CreateFollow_Action(aiPlayerEntity, rat,  Scene_AIStateMachine);
        // second behaviour is go for the god 
        AI_CreateFollow_Action(aiPlayerEntity, levelEntity,  Scene_AIStateMachine);
    }

    
    // Create Attack Wave effect
    for(int i = 0; i < PLAYER_COUNT; ++i){
        // attack wave
        Vec3 playerWavePos = {0.0f, -.01f, 0.0f};
        Vec3 playerWaveScale = {2.5f, 1.0f, 2.5f};
        playerWaves[i] = AF_ECS_CreateEntity(&_appData->ecs);
        //move the position up a little
        playerWaves[i]->transform->localPos = playerWavePos;
        playerWaves[i]->transform->localScale = playerWaveScale;
        playerWaves[i]->parentTransform = _appData->gameplayData.playerEntities[i]->transform;
        // add a rigidbody to our cube
        *playerWaves[i]->mesh = AF_CMesh_ADD();
        playerWaves[i]->mesh->meshType = AF_MESH_TYPE_MESH;
        playerWaves[i]->mesh->material.color = WHITE_COLOR;
        playerWaves[i]->mesh->material.color.a = 0;
        playerWaves[i]->mesh->meshID = MODEL_ATTACK_WAVE;
        //playerWaves[i]->mesh->enabled = AF_Component_SetEnabled(playerWaves[i]->mesh->enabled, FALSE);

        // Animation
        *playerWaves[i]->animation = AF_CAnimation_ADD();
        playerWaves[i]->animation->animationSpeed = foamAnimationSpeed;
        // disable at the start. Will get re-enabled on attack
        
    }
}

/* ================
Scene_Setup_RatSpawnPoints
General Setup function, to setup the rat spawn points
 ================ */
void Scene_Setup_RatSpawnPoints(AppData* _appData){
    // ============Buckets=============
    uint8_t offsetX = 5;
    uint8_t offsetZ = 4;
    Vec3 bucketScale = {5,0.1f,5};
    float bucketY = 0.01f;
    Vec3 mapBoundingVolume = {15,0, 7.5};

    // Bucket 1
    // World pos and scale for bucket
	Vec3 bucket1Pos = {-mapBoundingVolume.x + offsetX, bucketY, -mapBoundingVolume.z + offsetZ};
	//Vec3 bucket1Scale = {1,1,1};
    bucket1 = Entity_Factory_CreatePrimative(&_appData->ecs, bucket1Pos, bucketScale,AF_MESH_TYPE_MESH, AABB);
    // disable the mesh rendering
    bucket1->mesh->enabled = AF_Component_SetHas(bucket1->mesh->enabled, FALSE);
    bucket1->mesh->enabled = AF_Component_SetEnabled(bucket1->mesh->enabled, FALSE);
    //bucket1->mesh->meshID = MODEL_CYLINDER;
    //bucket1->mesh->material.color = WHITE_COLOR;
    bucket1->rigidbody->inverseMass = 0.0f;

    // TODO: add details to scene_onBucketTrigger callback
    bucket1->collider->collision.callback = Scene_OnBucket1Trigger;
    // Bucket 2
    // World pos and scale for bucket
	Vec3 bucket2Pos =  {mapBoundingVolume.x - offsetX, bucketY, -mapBoundingVolume.z + offsetZ};
	//Vec3 bucket2Scale = {1,1,1};
	bucket2 = Entity_Factory_CreatePrimative(&_appData->ecs, bucket2Pos, bucketScale,AF_MESH_TYPE_MESH, AABB);
    bucket2->mesh->enabled = AF_Component_SetHas(bucket2->mesh->enabled, FALSE);
    bucket2->mesh->enabled = AF_Component_SetEnabled(bucket2->mesh->enabled, FALSE);
    //bucket2->mesh->meshID = MODEL_CYLINDER;
    bucket2->rigidbody->inverseMass = 0.0f;
     // TODO: add details to scene_onBucketTrigger callback
    bucket2->collider->collision.callback = Scene_OnBucket2Trigger;

    // Bucket 3
    // World pos and scale for bucket
	Vec3 bucket3Pos =  {-mapBoundingVolume.x + offsetX, bucketY, mapBoundingVolume.z - (offsetZ*.5f)};
	//Vec3 bucket3Scale = {1,1,1};
	bucket3 = Entity_Factory_CreatePrimative(&_appData->ecs, bucket3Pos, bucketScale,AF_MESH_TYPE_MESH, AABB);
    bucket3->mesh->enabled = AF_Component_SetHas(bucket3->mesh->enabled, FALSE);
    bucket3->mesh->enabled = AF_Component_SetEnabled(bucket3->mesh->enabled, FALSE);
    //bucket3->mesh->meshID = MODEL_CYLINDER;
    bucket3->rigidbody->inverseMass = 0.0f;
     // TODO: add details to scene_onBucketTrigger callback
    bucket3->collider->collision.callback = Scene_OnBucket3Trigger;
    // Bucket 4
    // World pos and scale for bucket
	Vec3 bucket4Pos =  {mapBoundingVolume.x - offsetX, bucketY, mapBoundingVolume.z - (offsetZ*.5f)};
	//Vec3 bucket4Scale = {1,1,1};
	bucket4 = Entity_Factory_CreatePrimative(&_appData->ecs, bucket4Pos, bucketScale,AF_MESH_TYPE_MESH, AABB);
    bucket4->mesh->enabled = AF_Component_SetHas(bucket4->mesh->enabled, FALSE);
    bucket4->mesh->enabled = AF_Component_SetEnabled(bucket4->mesh->enabled, FALSE);
    //bucket4->mesh->meshID = MODEL_CYLINDER;
    bucket4->rigidbody->inverseMass = 0.0f;
     // TODO: add details to scene_onBucketTrigger callback
    bucket4->collider->collision.callback = Scene_OnBucket4Trigger;
}

/* ================
Scene_Setup_Sharks
General Setup function, to setup the sharks
 ================ */
void Scene_Setup_Sharks(AppData* _appData){

}

/* ================
Scene_Setup_Rats
General Setup function, to setup the rat
 ================ */
void Scene_Setup_Rats(AppData* _appData){
    // Rats
	Vec3 rat1Pos = {-1000.0f, 0, 0};
	Vec3 rat1Scale = {0.5f,0.5f,0.5f};
    _appData->gameplayData.rat = Entity_Factory_CreatePrimative(&_appData->ecs, rat1Pos, rat1Scale, AF_MESH_TYPE_MESH, AABB);
    AF_Entity* rat = _appData->gameplayData.rat;
    // save a global ptr copy 
    // TODO: i don't like this, fix it later
    g_ratPtr = rat;
    rat->mesh->meshID = MODEL_RAT;
	rat->rigidbody->inverseMass = 0.0f;
	rat->rigidbody->isKinematic = TRUE;
    rat->collider->collision.callback = Scene_OnCollision;
    debugf("Rat init: %p \n", rat);

    // TODO: re-introduce the rats into the population
    /*
    Vec3 ratSpawnPos = {2, 0,0};
    Vec3 ratScale = {1,1,1};
    for(int i = 0; i < ENEMY_POOL_COUNT; ++i){
        
        _appData->gameplayData.enemyEntities[i] = Entity_Factory_CreatePrimative(_ecs, ratSpawnPos, ratScale, AF_MESH_TYPE_MESH, AABB);
        AF_Entity* rat = _appData->gameplayData.enemyEntities[i];
        *rat->playerData = AF_CPlayerData_ADD();
        rat->playerData->isAlive = FALSE;
        rat->playerData->movementSpeed = RAT_MOVEMENT_SPEED;
        rat->mesh->meshID = MODEL_BOX;
        rat->rigidbody->inverseMass = zeroInverseMass;
        rat->rigidbody->isKinematic = TRUE;
        // disable the collision, mesh and physics for now
        rat->collider->collision.callback = OnRatCollision;
        //TogglePrimativeComponents(rat, TRUE);
    }
    */
}


/* ================
Scene_SetupLevelObjects
Setup additional level objects such as alter, smoke and pentogram
 ================ */
void Scene_SetupLevelObjects(AppData* _appData){
    float animationSpeed = -0.5f;

 /*
    // Place the alter
    Vec3 alterSpawnPos = {0,0.1f,0.1f};
    Vec3 alterScale = { 30,1,15};
    alterEntity = AF_ECS_CreateEntity(&_appData->ecs);
	alterEntity->transform->pos = alterSpawnPos;
    alterEntity->transform->scale = alterScale;
	// add a rigidbody to our cube
	*alterEntity->mesh = AF_CMesh_ADD();
	alterEntity->mesh->meshType = AF_MESH_TYPE_MESH;
    alterEntity->mesh->material.color = WHITE_COLOR;
    alterEntity->mesh->material.color.a = 255;
    alterEntity->mesh->meshID = MODEL_ALTER; 
    alterEntity->rigidbody->isKinematic = TRUE;
    alterEntity->rigidbody->inverseMass = 0.0f;
    
    
    // TODO: get these back in
    
    Vec3 graveSpawnPos = {0,0.5f,.1f};
    Vec3 graveScale = { 30,1,15};
    graveEntity = AF_ECS_CreateEntity(&_appData->ecs);
    graveEntity = AF_ECS_CreateEntity(&_appData->ecs);
	graveEntity->transform->pos = graveSpawnPos;
    graveEntity->transform->scale = graveScale;
	// add a rigidbody to our cube
	*graveEntity->mesh = AF_CMesh_ADD();
	graveEntity->mesh->meshType = AF_MESH_TYPE_MESH;
    graveEntity->mesh->material.color = WHITE_COLOR;
    graveEntity->mesh->material.color.a = 255;
    graveEntity->mesh->meshID = MODEL_GRAVE; 
    

    // place the re-usable smoke
    Vec3 smokeSpawnPos = {7,0,0};
    Vec3 smokeScale = { 3,3,3};
    smokeEntity = AF_ECS_CreateEntity(&_appData->ecs);
	smokeEntity->transform->pos = smokeSpawnPos;
    smokeEntity->transform->scale = smokeScale;
	// add a rigidbody to our cube
	*smokeEntity->mesh = AF_CMesh_ADD();
	smokeEntity->mesh->meshType = AF_MESH_TYPE_MESH;
    smokeEntity->mesh->material.color = WHITE_COLOR;
    smokeEntity->mesh->material.color.a = 255;
    smokeEntity->mesh->meshID = MODEL_SMOKE; 
    smokeEntity->rigidbody->isKinematic = TRUE;
    smokeEntity->rigidbody->inverseMass = 0.0f;
    */

    // place the re-usable snake eaten
    Vec3 playerEatenWavesSpawnPos = {40,0,0};
    Vec3 playerEatenWavesalterScale = { 1,1,1};
    playerEatenWave = AF_ECS_CreateEntity(&_appData->ecs);
	playerEatenWave->transform->pos = playerEatenWavesSpawnPos;
    playerEatenWave->transform->scale = playerEatenWavesalterScale;
	// add a rigidbody to our cube
	*playerEatenWave->mesh = AF_CMesh_ADD();
	playerEatenWave->mesh->meshType = AF_MESH_TYPE_MESH;
    playerEatenWave->mesh->material.color = WHITE_COLOR;
    playerEatenWave->mesh->material.color.a = 150;
    playerEatenWave->mesh->meshID = MODEL_ATTACK_WAVE; 
    playerEatenWave->rigidbody->isKinematic = TRUE;
    playerEatenWave->rigidbody->inverseMass = 0.0f;
    
    // Animation
    *playerEatenWave->animation = AF_CAnimation_ADD();
    playerEatenWave->animation->animationSpeed = animationSpeed;

    // Place the alter
    Vec3 ratPentogramSpawnPos = {-5,0.1f,0.1f};
    Vec3 ratPentogramScale = { 15,1,7.5};
    ratSpawnEntity = AF_ECS_CreateEntity(&_appData->ecs);
	ratSpawnEntity->transform->pos = ratPentogramSpawnPos;
    ratSpawnEntity->transform->scale = ratPentogramScale;
	// add a rigidbody to our cube
	*ratSpawnEntity->mesh = AF_CMesh_ADD();
	ratSpawnEntity->mesh->meshType = AF_MESH_TYPE_MESH;
    ratSpawnEntity->mesh->material.color = WHITE_COLOR;
    ratSpawnEntity->mesh->material.color.a = 150;
    ratSpawnEntity->mesh->meshID = MODEL_ALTER; 
    ratSpawnEntity->rigidbody->isKinematic = TRUE;
    ratSpawnEntity->rigidbody->inverseMass = 0.0f;
    
    
}

/* ================
Scene_SetupSharks
Setup sharks
 ================ */
void Scene_SetupSharks(AppData* _appData){
    // ==== Patroling Shark ====
    float sharkFoamAnimationSpeed = -0.7f;
    Vec3 sharkSpawnPos = {20, 0, 0};
    
    Vec3 sharkScale = {1,1,1};
    sharkEntity = Entity_Factory_CreatePrimative(&_appData->ecs, sharkSpawnPos, sharkScale, AF_MESH_TYPE_MESH, AABB);
    sharkEntity->mesh->meshID = MODEL_SHARK;
    sharkEntity->rigidbody->inverseMass = 0.0f;
    sharkEntity->rigidbody->isKinematic = TRUE;
    sharkEntity->collider->collision.callback = OnSharkCollision;

    // player data
    *sharkEntity->playerData = AF_CPlayerData_ADD();
    
    sharkEntity->playerData->movementSpeed = 10.0f;
    sharkEntity->playerData->faction = ENEMY2;
    // Create Player1 Foam Trail
    // position needs to be thought of as local to the parent it will be inherited by
    Vec3 sharkTrailPos = {0.0f, -.01f, 0.0f};
    Vec3 sharkTrailScale = {1.5f, 1.0f, 1.5f};
    sharkTrail = AF_ECS_CreateEntity(&_appData->ecs);
	//move the position up a little
	sharkTrail->transform->localPos = sharkTrailPos;
    sharkTrail->transform->localScale = sharkTrailScale;
    sharkTrail->parentTransform = sharkEntity->transform;
	// add a rigidbody to our cube
	*sharkTrail->mesh = AF_CMesh_ADD();
	sharkTrail->mesh->meshType = AF_MESH_TYPE_MESH;
    sharkTrail->mesh->material.color = WHITE_COLOR;
    sharkTrail->mesh->material.color.a = 255;
    sharkTrail->mesh->meshID = MODEL_TRAIL; 

    // Animation
    *sharkTrail->animation = AF_CAnimation_ADD();
    sharkTrail->animation->animationSpeed = sharkFoamAnimationSpeed;

    Vec3 sharkHunterSpawnPos = {-30, 0, 0};
    // create the shark home
    sharkHomeEntity = AF_ECS_CreateEntity(&_appData->ecs);
    sharkHomeEntity->transform->pos = sharkHunterSpawnPos;
    sharkHomeEntity->rigidbody->inverseMass = 0.0f;
    sharkHomeEntity->rigidbody->isKinematic = TRUE;
    // ==== Hunter Shark ====
    // 1 for each player
    for(int i = 0; i < PLAYER_COUNT; ++i){
        
        Vec3 sharkHunterScale = {1,1,1};

        sharkHunterEntities[i] = Entity_Factory_CreatePrimative(&_appData->ecs, sharkHunterSpawnPos, sharkHunterScale, AF_MESH_TYPE_MESH, AABB);
        AF_Entity* sharkHunterEntity = sharkHunterEntities[i];
        sharkHunterEntity->mesh->meshID = MODEL_SHARK;
        sharkHunterEntity->rigidbody->inverseMass = 0.0f;
        sharkHunterEntity->rigidbody->isKinematic = TRUE;
        sharkHunterEntity->collider->collision.callback = OnSharkCollision;

        // player data
        *sharkHunterEntity->playerData = AF_CPlayerData_ADD();
        sharkHunterEntity->playerData->movementSpeed = AI_MOVEMENT_SPEED_MOD * ((1+core_get_aidifficulty()) + rand()%((1+core_get_aidifficulty())))*2;
        sharkHunterEntity->playerData->faction = ENEMY2;

        // AI
        *sharkHunterEntity->aiBehaviour = AF_CAI_Behaviour_ADD();
        

        //BOOL hasAI = AF_Component_GetHas(sharkHunterEntity->aiBehaviour->enabled);
        //BOOL isEnabled = AF_Component_GetEnabled(sharkHunterEntity->aiBehaviour->enabled);
        //debugf("shark hasAI: %i isEnabled: %i\n",hasAI, isEnabled);
        // match the target with the player number
        // add a follow action 
        AI_CreateFollow_Action(sharkHunterEntity, _appData->gameplayData.playerEntities[i],  Scene_AIStateMachine);

        // Create the go home action. This function increments to a new action slot if called again.
        AI_CreateFollow_Action(sharkHunterEntity, sharkHomeEntity,  Scene_AIStateMachine);

        // disable the component for now, but it will be enabled when the player goes out of bounds.
        sharkHunterEntity->aiBehaviour->enabled = AF_Component_SetEnabled(sharkHunterEntity->aiBehaviour->enabled, FALSE);
        //BOOL hasAIBehaviour = AF_Component_GetHas(sharkHunterEntity->aiBehaviour->enabled);
        //BOOL aiIsEnabled = AF_Component_GetEnabled(sharkHunterEntity->aiBehaviour->enabled);
        //debugf("entity: %i hasAI: %i isEnabled: %i\n",i, hasAIBehaviour, aiIsEnabled);
        // Create Player1 Foam Trail
        // position needs to be thought of as local to the parent it will be inherited by
        //Vec3 sharkHunterTrailPos = {0.0f, -.01f, 0.0f};
        //Vec3 sharkHunterTrailScale = {1.5f, 1.0f, 1.5f};
        sharkHunterTrails[i] = AF_ECS_CreateEntity(&_appData->ecs);
        AF_Entity* sharkHunterTrail = sharkHunterTrails[i];
        //move the position up a little
        sharkHunterTrail->transform->localPos = sharkTrailPos;
        sharkHunterTrail->transform->localScale = sharkTrailScale;
        sharkHunterTrail->parentTransform = sharkHunterEntity->transform;
        // add a rigidbody to our cube
        *sharkHunterTrail->mesh = AF_CMesh_ADD();
        sharkHunterTrail->mesh->meshType = AF_MESH_TYPE_MESH;
        sharkHunterTrail->mesh->material.color = WHITE_COLOR;
        sharkHunterTrail->mesh->material.color.a = 255;
        sharkHunterTrail->mesh->meshID = MODEL_TRAIL; 

        // Animation
        *sharkHunterTrail->animation = AF_CAnimation_ADD();
        sharkHunterTrail->animation->animationSpeed = sharkFoamAnimationSpeed;
    }

     
}


// ======== SPAWNERS ==========
// TODO: add better control flow
/* ================
Scene_SpawnBucket
setup the rat spawn entities
 ================ */
void Scene_SpawnBucket(uint8_t* _currentBucket){
    int upper = 4;
    int lower = 0;
    int randomNum = (rand() % (upper + - lower) + lower);
    // don't let the random number go above 4, as we count from 0
    if(randomNum >=4){
        randomNum = 3;
    }

    if(levelEntity == NULL || bucket1 == NULL || bucket2 == NULL || bucket3 == NULL || bucket4 == NULL)
    {   
        debugf("Game: SpawnBucket: god or bucket entity is null \n");
        return;
    }

    AF_Entity* bucket1 = GetBucket1();
    AF_Entity* bucket2 = GetBucket2();
    AF_Entity* bucket3 = GetBucket3();
    AF_Entity* bucket4 = GetBucket4();

    
    AF_Entity* rat = g_ratPtr;//_appData->gameplayData.rat;
    assert(rat != NULL);
    if(randomNum == 0){
        *_currentBucket = 0;
        Vec3 bucket1Pos = {bucket1->transform->pos.x, bucket1->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket1->transform->pos.z};
        
        rat->transform->pos = bucket1Pos;


    }else if( randomNum == 1){
        *_currentBucket = 1;
        Vec3 bucket2Pos = {bucket2->transform->pos.x, bucket2->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket2->transform->pos.z};
        rat->transform->pos = bucket2Pos;

    }else if( randomNum == 2){
        *_currentBucket = 2;
        Vec3 bucket3Pos = {bucket3->transform->pos.x, bucket3->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket3->transform->pos.z};
        rat->transform->pos = bucket3Pos;

    }else if( randomNum == 3){
        *_currentBucket = 3;
        Vec3 bucket4Pos = {bucket4->transform->pos.x, bucket4->transform->pos.y + VILLAGER_CARRY_HEIGHT, bucket4->transform->pos.z};
        rat->transform->pos = bucket4Pos;

    }

    // move the puff of spawn smoke
    //smokeEntity->transform->pos = rat1->transform->pos;
    // make the smoke appear. it will disapate over time.
    //smokeEntity->mesh->material.color.a = 255;

    // Move the pentogram to the rat spawn pos
    ratSpawnEntity->transform->pos = rat->transform->pos;
    // make the smoke appear. it will disapate over time.
    ratSpawnEntity->mesh->material.color.a = 255;
}

// ======== TRIGGERS =========

/* ================
Scene_OnTrigger
Default collision callback to be used by game entities
 ================ */
void Scene_OnTrigger(AF_Collision* _collision){
    
}


/* ================
Scene_OnGodTrigger
Callback Behaviour triggered when the player dropps off a sacrafice to the gods
 ================ */
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

    // if entity is carrying, eat and shift the villager into the distance
    if(collidedEntity->playerData->isCarrying == TRUE){
       
        //debugf("Scene_GodTrigger: eat count %i \n", godEatCount);
        // TODO: update godEatCount. observer pattern would be nice right now
        // Animate God eating
        AF_CSkeletalAnimation*  animation = godSnekEntity->skeletalAnimation;
        // if this entity has animations, then call play animation
        BOOL hasMeshComponent = AF_Component_GetHas(animation->enabled);
        BOOL isEnabled = AF_Component_GetEnabled(animation->enabled);
        if(hasMeshComponent == TRUE && isEnabled == TRUE){
            
            AF_Renderer_PlayAnimation(animation);
        }
        
        // update the player who collided with gods score.
        collidedEntity->playerData->score ++;
        collidedEntity->playerData->isCarrying = FALSE;
        collidedEntity->playerData->carryingEntity = 0;
        
        // TODO Move this:
        RespawnRat();
        // play sound
        wav64_play(&feedGodSoundFX, 16);
        // clear the players from carrying
    }
}

/* ================
RespawnRat
Respawn the rat
 ================ */
void RespawnRat(){
    Vec3 poolLocation = {100, 0,0};
    AF_Entity* rat = g_ratPtr;//_appData->gameplayData.rat;
    rat->transform->pos = poolLocation;
    rat->playerData->isCarried = FALSE;
    // randomly call for a colour bucket
    Scene_SpawnBucket(&currentBucket);
}


/* ================
Scene_BucketCollisionBehaviour
Perform gameplay logic if bucket has been collided with by a player character
 ================ */
void Scene_BucketCollisionBehaviour(int _currentBucket, int _bucketID, AF_Collision* _collision){
    
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
    
	// only players can carry rats
    if(playerData2->faction != PLAYER){
        return;
    }


    // Move this
    // attatch the rat to this player
    
    if(g_ratPtr->playerData->isCarried == FALSE){
        playerData2->carryingEntity = g_ratPtr->id_tag;
        //_rat->mesh->material.textureID = _godEntity->mesh->material.textureID;
        //debugf("carry villager \n");
        playerData2->isCarrying = TRUE;
        g_ratPtr->playerData->isCarried = TRUE;

        // play sound
        wav64_play(&pickupSoundFX, 16);
    }
}

// TODO: Make these more re-usable
/* ================
Scene_OnBucketTrigger
Trigger callback assigned to buckets in the game world
 ================ */
void Scene_OnBucket1Trigger(AF_Collision* _collision){
    int bucketID = 0;
    if(currentBucket != bucketID){
        return;
    }
    
    Scene_BucketCollisionBehaviour(currentBucket, bucketID, _collision);
}

/* ================
Scene_OnBucket2Trigger
Trigger callback assigned to buckets in the game world
 ================ */
void Scene_OnBucket2Trigger(AF_Collision* _collision){
    int bucketID = 1;
    if(currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(currentBucket, bucketID, _collision);
}

/* ================
Scene_OnBucket2Trigger
Trigger callback assigned to buckets in the game world
 ================ */
void Scene_OnBucket3Trigger(AF_Collision* _collision){
    int bucketID = 2;
    if(currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(currentBucket, bucketID, _collision);
}

/* ================
Scene_OnBucket3Trigger
Trigger callback assigned to buckets in the game world
 ================ */
void Scene_OnBucket4Trigger(AF_Collision* _collision){
    int bucketID = 3;
    if(currentBucket != bucketID){
        return;
    }
    Scene_BucketCollisionBehaviour(currentBucket, bucketID, _collision);
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


// ==== Collision Behaviours ====

/*===============
OnRatCollision
Collision call made by the rat
================*/
void OnRatCollision(AF_Collision* _collision){
    if(_collision == NULL){
        return;
    }

    AF_Entity* rat =  _collision->entity1;
	AF_Entity* player =  _collision->entity2;

    // Second collision is the playable character
    // skip if collision object doesn't have player data
    AF_CPlayerData* playerData = player->playerData;
    AF_CPlayerData* ratData = rat->playerData;
    if(AF_Component_GetHas(playerData->enabled) == FALSE){
        return;
    }

    if(playerData->faction == PLAYER){
        
        // if player is attacking
        if(playerData->isAttacking == TRUE){
            //debugf("OnRatCollision: hit player attacking player \n");
            // Rat is carried by colliding player
            playerData->isCarrying = TRUE;
            playerData->carryingEntity = AF_ECS_GetID(rat->id_tag);
            ratData->isCarried = TRUE;
            //ratData->isAlive = FALSE;
        }
    }
    //debugf("OnRatCollision: with a player\n");
    // is rat dead?
        // attatch to collided player
    // if rat is alive
        // puff of smoke
        // rat flips onto its back
        // rat is dead
}

/*===============
OnSharkCollision
Collision call made by the shark
================*/
void OnSharkCollision(AF_Collision* _collision){
    if(_collision == NULL){
        return;
    }

    //AF_Entity* shark =  _collision->entity1;
	AF_Entity* player =  _collision->entity2;

    // Second collision is the playable character
    // skip if collision object doesn't have player data
    AF_CPlayerData* playerData = player->playerData;
    //AF_CPlayerData* sharkData = shark->playerData;
    if(AF_Component_GetHas(playerData->enabled) == FALSE){
        return;
    }

    // disable the renderer when we are finished
    // if the collider entity is of player faction
    if(playerData->faction == PLAYER){
        playerData->isAlive = FALSE;
        //debugf("OnSharkCollision: hit player yum yum \n");
        // player yum yum audio
        wav64_play(&pickupSoundFX, 16);
    }

    
}


// ==== AI BEHAVIOURS ====
// AI state machine
// TODO move this into its own system/header
/*===============
Scene_AIStateMachine
AI state machine to switch between different behaviours in the list
================*/
void Scene_AIStateMachine(AF_AI_Action* _aiAction){
    // We need to 
    assert(_aiAction != NULL);

    AF_Entity* sourceEntity = (AF_Entity*)_aiAction->sourceEntity;
    AF_Entity* targetEntity = (AF_Entity*)_aiAction->targetEntity;
    //debugf("scene ai state machine:  entity:%p \n", targetEntity);
        
    assert(sourceEntity != NULL);
    assert(targetEntity != NULL);

    AF_CPlayerData* sourceEntityPlayerData = sourceEntity->playerData;
    AF_CAI_Behaviour* sourceAIBehaviour = sourceEntity->aiBehaviour;

    switch(sourceEntity->playerData->faction){
        // Player Behaviour
        case PLAYER:
            if(sourceEntityPlayerData->isCarrying){
                _aiAction = &sourceAIBehaviour->actionsArray[1];
            }else{
                _aiAction = &sourceAIBehaviour->actionsArray[0];
            }
            _aiAction->sourceEntity = sourceEntity;
        break;

        // Enemy/Shark behaviour
        case ENEMY1:
            // TODO:
            
        break;

        case ENEMY2:
            // if we are in an attacking state. go after the target we have had set
            
            if(sourceEntityPlayerData->isAttacking){
                // first action is setup to follow the player
                _aiAction = &sourceAIBehaviour->actionsArray[0];
            }else{
                // Second action is setup to follow the go home point
                _aiAction = &sourceAIBehaviour->actionsArray[1];
            }
            _aiAction->sourceEntity = sourceEntity;
        break;

        case ENEMY3:
            // TODO:
        break;

        case ENEMY4:
            // TODO:

        break;

        case DEFAULT:

        break;
    }
    
    Scene_AIFollowEntity(_aiAction);
}


/*===============
Scene_AIFollowEntity
// AI behaviour tester function
// take in a void* that we know will be a FollowBehaviour struct
// this allows our original callback to be generically re-used
// But its hella risky. YOLO
=============== */
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
    //debugf("source entity:%p  \n", _aiAction->sourceEntity);
    //debugf("target entity:%p x:%f y:%f z:%f \n", _aiAction->targetEntity, destination.x, destination.y, destination.z);
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

/*===============
RatAIBehaviourUpdate
Rat AI behaviour
================*/
void RatAIBehaviourUpdate(AppData* _appData){
    Vec3 spawnBounds = {7, 1, 3.5};
    int upperX = spawnBounds.x;
    int lowerX = -spawnBounds.x;
    int upperZ = spawnBounds.z;
    int lowerZ = -spawnBounds.z;
    
    for(int i = 0; i < ENEMY_POOL_COUNT; ++i){
        AF_Entity* ratEntity = _appData->gameplayData.enemyEntities[i];
        assert(ratEntity != NULL);
        AF_CPlayerData* ratPlayerData = ratEntity->playerData;
        // check if rat is alive)
        if(ratPlayerData->isAlive == FALSE){
            // spawn in random spot within bounds
            int randomX = (rand() % (upperX + - lowerX) + lowerX);
            int randomZ = (rand() % (upperZ + - lowerZ) + lowerZ);
            Vec3 randomPos = {randomX, ratEntity->transform->pos.y, randomZ};
            ratEntity->transform->pos = randomPos;
            ratPlayerData->isAlive = TRUE;
            TogglePrimativeComponents(ratEntity, TRUE);
            
        }else{
            //debugf("Move Rat\n");
            AF_CPlayerData* ratPlayerData = ratEntity->playerData;
            Vec3* ratPosition = &ratEntity->transform->pos;
            Vec3* ratDestination = &ratPlayerData->targetDestination;

            // Are we clost to our target
            float distanceToDest = Vec3_DISTANCE(*ratPosition, *ratDestination);
            if(distanceToDest < AI_TARGET_DISTANCE){
                // time to find a new destination point
                // spawn in random spot within bounds
                int randomX = (rand() % (upperX + - lowerX) + lowerX);
                int randomZ = (rand() % (upperZ + - lowerZ) + lowerZ);
                Vec3 randomDest = {randomX, ratEntity->transform->pos.y, randomZ};
                *ratDestination = randomDest;
            }
            
            AF_C3DRigidbody* ratRigidbody = ratEntity->rigidbody;
            Vec3 initialForce = Vec3_NORMALIZE(Vec3_MINUS(*ratDestination, *ratPosition));
            Vec3 directionForce = {ratPlayerData->movementSpeed * initialForce.x, 0, ratPlayerData->movementSpeed * initialForce.z};
            //debugf("Rat target   : x: %f y: %f z: %f\nRat direction: x: %f y: %f z: %f\n", ratDestination->x, ratDestination->y, ratDestination->z, directionForce.x, directionForce.y, directionForce.z);
            AF_Physics_ApplyLinearImpulse(ratRigidbody, Vec3_MULT_SCALAR(directionForce, ratPlayerData->movementSpeed));
            // Move rat towards destination
            // Is time time change direct reached?
            // is distance to destination reached?
            // pick new point to head towards.
        }
        // if not alive, is it time to spawn
        // TogglePrimativeComponents(rats[i], FALSE);
        // move rats around if they are alive

    }
}

/*===============
SharkAIBehaviourUpdate
Shark AI Behaviour
================*/
void SharkAIBehaviourUpdate(AppData* _appData){
    // move in a circular pattern around the centre
    // facing in the direction of movement.
    AF_CTransform3D* sharkTransform = sharkEntity->transform;
    AF_C3DRigidbody* sharkRigidbody = sharkEntity->rigidbody;

    Vec3 centre = {0,0,0};
    float radiusX = 4.0f; // Horizontal radius
    float radiusZ = 1.0f; // vertical radius

    
    AF_CPlayerData* sharkPlayerData = sharkEntity->playerData;

    // Current position relative to the center
    Vec3 vecToCentre = Vec3_MINUS(centre, sharkTransform->pos);
    
    // Project position onto the ellipse
    Vec3 ellipsePos;
    ellipsePos.x = vecToCentre.x / (radiusX * radiusX);
    ellipsePos.y = 0;
    ellipsePos.z = vecToCentre.z / (radiusZ * radiusZ);

    // Normalise the projected vector to find direction along the ellipse
    Vec3 ellipseNorm = Vec3_NORMALIZE(ellipsePos);

    // Calculate tangential velocity (perpendicular to radial direction)
    Vec3 tangent;
    tangent.x = -ellipseNorm.z * radiusZ; // Perpendicular component
    tangent.y = 0;
    tangent.z = ellipseNorm.x * radiusX; // Perpendicular component

    
    Vec3 tangentialVelocity = Vec3_NORMALIZE(tangent);
    // Scale tangential velocity to the desired speed
    tangentialVelocity = Vec3_MULT_SCALAR(tangentialVelocity, sharkPlayerData->movementSpeed);

    //AF_Physics_ApplyLinearImpulse(sharkRigidbody, tangentialVelocity);
    sharkRigidbody->velocity = tangentialVelocity;

    // rotate towards direction
	float newAngle = atan2f(-tangentialVelocity.x, tangentialVelocity.z);
	sharkTransform->rot.y = AF_Math_Lerp_Angle(sharkTransform->rot.y, newAngle, 0.25f);

}

/*===============
MonitorPlayerHealth
// Monitor Player Health
// if players die, then respawn in a position.
================*/
void MonitorPlayerHealth(AppData* _appData){
    for(int i = 0; i < PLAYER_COUNT; ++i){
        AF_Entity* playerEntity = _appData->gameplayData.playerEntities[i];
        AF_CPlayerData* playerData = playerEntity->playerData;

        if(playerData->isAlive == FALSE){
            // if the player dies whilst carrying. ensure it is respawned
            if(playerData->isCarrying == TRUE){
                
                RespawnRat();
            }
            // start a timer and restart
            // move the blood red wave to the players death position
            playerEatenWave->transform->pos = playerEntity->transform->pos;
            // make it appear again. it will disapate over time.
            playerEatenWave->mesh->material.color.a = 255;
            RespawnPlayer(playerEntity);
        }
    }
}

/*===============
RespawnPlayer
Respawn the player
================*/
void RespawnPlayer(AF_Entity* _entity){
    // zero velocities
    AF_C3DRigidbody* rigidbody = _entity->rigidbody;
    Vec3 zeroVel = {0,0,0};
    rigidbody->velocity = zeroVel;


    // make alive
    AF_CPlayerData* playerData = _entity->playerData;
    playerData->isAlive = TRUE;
    playerData->isCarrying = FALSE;
    playerData->carryingEntity = 0;

    // position back to starting spot
    AF_CTransform3D* transform = _entity->transform;
    transform->pos = playerData->startPosition;
}

// ===== HELPERS ====
/*===============
TogglePrimativeComponents
toggle the components mesh, rigidybody, and collider
================*/
void TogglePrimativeComponents(AF_Entity* _entity, BOOL _state){
    _entity->rigidbody->enabled = AF_Component_SetEnabled(_entity->rigidbody->enabled, _state);
    _entity->mesh->enabled = AF_Component_SetEnabled(_entity->mesh->enabled, _state);
    _entity->collider->enabled = AF_Component_SetEnabled(_entity->collider->enabled, _state);
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

