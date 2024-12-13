/*================
UI_MENU_H
Contains functions and code relating to the scene including all the entities that will be setup for the game
================*/
#include "ECS/Entities/AF_ECS.h"
#include "AF_Input.h"
#include "AF_Time.h"
#include "AppData.h"

// Function Definitions
void UI_Menu_Start(AppData* _appData);
void UI_Menu_Awake(AppData* _appData);
void UI_Menu_Update(AppData* _appData);
void UI_Menu_Shutdown(AF_ECS* _ecs);
