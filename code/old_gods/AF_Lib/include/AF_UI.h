/*
===============================================================================
AF_UI_H

Implimentation of helper functions for UI rendering
Used for text rendering
TODO: sprite and other UI component rendering
===============================================================================
*/
#include "ECS/Entities/AF_ECS.h"
#include "ECS/Components/AF_CText.h"
#include "AF_Time.h"
void AF_UI_Init(AF_ECS* _ecs);
void AF_UI_Update(AF_ECS* _ecs, AF_Time* _time);
void AF_UI_RendererText_Update(AF_CText* _text);
void AF_UI_RendererSprite_Update(AF_CSprite* _sprite, AF_Time* _time);
void AF_UI_RendererSprite_Play(AF_CSprite* _sprite, BOOL _isLooping);
void AF_UI_Renderer_Finish(void);
void AF_UI_Renderer_Shutdown(AF_ECS* _ecs);
void* AF_LoadFont(int _id, const char* _path, uint8_t _color[4]);