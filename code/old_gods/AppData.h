/*================
Simple struck to contain app data
==================*/
#ifndef APP_DATA_H
#define APP_DATA_H
#include "ECS/Entities/AF_ECS.h"
#include "AF_Time.h"
#include "AF_Input.h"
#include "GameplayData.h"

typedef struct AppData {
    uint16_t windowWidth;
    uint16_t windowHeight;
    AF_Time gameTime;
    AF_Input input;
    GameplayData gameplayData;
    AF_ECS ecs;
}AppData;

#endif