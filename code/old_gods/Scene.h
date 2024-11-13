/*================
SCENE_H
Contains functions and code relating to the scene including all the entities that will be setup for the game
================*/
#include "ECS/Entities/AF_ECS.h"
#include "AF_Input.h"
#include "GameplayData.h"
#include "App.h"

#ifndef SCENE_H
#define SCENE_H

#define PLAYER_COUNT 4

// public accessible init function
void Scene_Awake(AppData* _appData);
void Scene_Start(AppData* _appData);
void Scene_Update(AppData* _appData);
void Scene_Destroy(AF_ECS* _ecs);

// Setup the games entities
void Scene_SetupEntities(AppData* _appData);


AF_Entity* GetVillagerEntity();
AF_Entity* GetGodEntity();

// Getters and Setters
AF_Entity* GetVillager();
AF_Entity* GetGodEntity();


AF_Entity* GetPlayerEntity(uint8_t _index);

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
void Scene_BucketCollisionBehaviour(int _currentBucket, int _bucketID, AF_Collision* _collision, AF_Entity* _villager, AF_Entity* _godEntity);

// Factory functions
AF_Entity* CreateGod(AF_ECS* _ecs, Vec3 _pos, Vec2 _scale, uint8_t _textureID);
AF_Entity* CreatePlayer(AF_ECS* _ecs, uint8_t _playerID, Vec3 _pos, Vec2 _scale, uint8_t _textureID);
AF_Entity* CreateBucket(AF_ECS* _ecs, uint8_t _bucketID, Vec3 _pos, Vec2 _scale, uint8_t _textureID);
AF_Entity* CreateVillager(AF_ECS* _ecs, uint8_t _villagerID, Vec3 _pos, Vec2 _scale, uint8_t _textureID);




#endif  // SCENE_H