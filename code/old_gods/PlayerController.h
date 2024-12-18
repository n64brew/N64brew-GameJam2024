/*
===============================================================================
PLAYERCONTROLLER_H definitions

Definition for the PlayerController 
and helper functions
===============================================================================
*/
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H
#include "App.h"

// Function definitions
void PlayerController_UpdateAllPlayerMovements(AppData* _appData);
void PlayerController_UpdatePlayerMovement(Vec2 _stick, AF_Entity* _entity);
void PlayerController_UpdatePlayerButtonPress(uint8_t _playerIndex, AF_Input* _input, AF_Entity* _entity);
void PlayerController_Attack(AF_Entity* _entity);

#endif