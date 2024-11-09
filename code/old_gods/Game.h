/*================
Game is used to run all gameplay logic e.g. take in inputs and then process them for specific game functionality.
Game also will take in ECS Entities and then apply gameplay to those.
*/

#ifndef GAME_H
#define GAME_H
#include "ECS/Entities/AF_ECS.h"
#include "AF_Input.h"
#include "AF_Time.h"
#include "AF_Collision.h"
#include "GameplayData.h"
#include "App.h"

void Game_Awake(AF_ECS* _ecs);
void Game_Start(AF_ECS* _ecs);
void Game_Update(AppData* _appData);
void Game_LateUpdate(AF_ECS* _ecs);
void Game_OnCollision(AF_Collision* _collision);
void Game_Shutdown(void);

#endif
