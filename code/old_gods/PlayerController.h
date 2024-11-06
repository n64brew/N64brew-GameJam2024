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

    void UpdatePlayerMovement(Vec2 _stick, AF_Entity* _ecs);

#endif