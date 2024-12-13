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

// TODO: move this
#define PLAYER_COUNT 4

// public accessible init function
void Scene_Awake(AppData* _appData);
void Scene_Start(AppData* _appData);
void Scene_Update(AppData* _appData);
void Scene_LateUpdate(AppData* _appData);
void Scene_Destroy(AF_ECS* _ecs);

#endif  // SCENE_H