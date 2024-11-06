/*================
App to kick of the rest of the game and game functions
App will handle communication between the main entry point, update, input, rendering, sound, physics etc.
==================*/
#ifndef APP_H
#define APP_H
#include <stdint.h>
#include "AF_Input.h"
#include "ECS/Entities/AF_ECS.h"
#include "AF_Time.h"
typedef struct {
    uint16_t windowWidth;
    uint16_t windowHeight;
    
}AppData;
void App_Init(uint16_t _windowWidth,uint16_t _windowHeight, AF_Time* _time);
void App_Update(AF_Input* _input, AF_ECS* _ecs, AF_Time* _time);
void App_Update_Wrapper(int _ovfl);
void App_Render_Update(AF_Time* _time);
void App_Shutdown(void);
#endif
