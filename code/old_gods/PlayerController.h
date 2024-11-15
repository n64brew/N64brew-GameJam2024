/*
===============================================================================
PLAYERCONTROLLER_H definitions

Definition for the PlayerController 
and helper functions
===============================================================================
*/
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "AF_Input.h"
#include "ECS/Entities/AF_Entity.h"

void PlayerController_UpdateAllPlayerMovements(AF_Input* _input, AF_Entity* _entities, uint8_t _entityCount);
void PlayerController_UpdatePlayerMovement(Vec2 _stick, AF_Entity* _entity);
void PlayerController_UpdatePlayerButtonPress(uint8_t _playerIndex, AF_Input* _input, AF_Entity* _entity);

void PlayerController_Attack(AF_Entity* _entity);


#endif