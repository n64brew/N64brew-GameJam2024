#include "Game.h"
#include <libdragon.h>

#include <inttypes.h>
#include "ECS/Entities/AF_ECS.h"
#include "AF_Input.h"
#include "AF_Debug.h"
#include "AF_Util.h"
//#include "AF_Ray.h"
#include "AF_Physics.h"
#include "AF_Audio.h"
#include "AF_UI.h"
#include "ECS/Components/AF_CMesh.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>
#include "PlayerController.h"
#include "AF_Renderer.h"
#include "EntityFactory.h"
#include "Assets.h"
#include "Scene.h"
#include "UI_Menu.h"

// Sprite
//Animation frame size defines
#define ANIM_FRAME_W 120
#define ANIM_FRAME_H 80

//Animation frame timing defines
#define ANIM_FRAME_DELAY 3
#define ANIM_FRAME_MAX 6

/*================
Game_Awake
Standard start function to kick off init functions, called before start
================*/
void Game_Awake(AF_ECS* _ecs){
	assert(_ecs != NULL &&"Game_Awake: passed null ecs reference \n");
	debugf("Game Awake\n");
	
}

/*================
Game_Start
Standard start function to kick off init functions
================*/
void Game_Start(AF_ECS* _ecs){
	assert(_ecs != NULL && "Game_Start: passed null ecs reference \n");
    debugf("Game_Start\n");

    //PlayMusic();
}

/*================
Game_Update
used to call input and other functions that should occur every fixed tick.
================*/
void Game_Update(AppData* _appData)
{
	assert(_appData != NULL && "Game_Update: passed null AppData reference \n");
	//AF_ECS* ecs = &_appData->ecs;
	//AF_Input* input = &_appData->input;
	//GameplayData* gameplayData = &_appData->gameplayData;
	//AF_Time* time = &_appData->gameTime;

	//if(ecs == NULL){
	//	debugf("Game_Update: passed null ecs reference \n");
	//	return;Scene
	//}
	// game update

	// update cube position based on button press
	// get input
	// if stick pressed left, right, up, down then adjust velocity
	// add velocity to cube component

    // if gamestate is playing, then accept player inputs
    
    
}

/*================
Game_LateUpdate
used to run in-between render start and render end. 
Good for rendering debug data.
================*/
void Game_LateUpdate(AppData* _appData){
	assert(_appData != NULL && "Game_LateUpdate: passed null ecs reference \n");
	
}

/*================
Game_Shutdown
Standard shutdown function. Called last by the window manager before exiting
Use to free memory and clean up
================*/
void Game_Shutdown(void){
	debugf("Game_Shutdown");

}

/*
void OnScoreUpdate(GameEventType event, int playerId, int score, GameplayData* _gameplayData) {
    if (event == EVENT_SCORE_UPDATED) {
        _gameplayData->playerScores[playerId] += score;
        printf("Player %d score updated to %d\n", playerId, _gameplayData->playerScores[playerId]);
    }
}

// Register this callback
void RegisterScoreSystem(GameplayData* _gameplayData) {
    GameState_RegisterObserver(_gameplayData, OnScoreUpdate);
}*/