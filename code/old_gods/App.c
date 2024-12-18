#include "App.h"
#include <stdio.h>
#include <libdragon.h>
#include <t3d/t3d.h>
#include "AF_Renderer.h"
#include "AF_Input.h"
#include "AF_Physics.h"
#include "AF_UI.h"
// UFNLoader for n64
#include "debug.h"
#include "Scene.h"
#include "UI_Menu.h"
#include "AI.h"


#include "../../minigame.h"

// forward declare
void PrintHeapStatus(const char* _message);

/* ===============
PlayerController_UpdatePlayerButtonPress
// Update player button press
================ */
void AppData_Init(AppData* _appData, uint16_t _windowWidth, uint16_t _windowHeight){
    _appData->windowWidth = _windowWidth;
    _appData->windowHeight = _windowHeight;
    _appData->gameTime = AF_Time_Init(0);
    _appData->input = AF_Input_ZERO();
    // initialise the gameplay data
    _appData->gameplayData = GameplayData_INIT();
    _appData->gameplayData.gameState = GAME_STATE_MAIN_MENU;
    AF_ECS_Init(&_appData->ecs);
    
}

/* ===============
App_Init
Init function
================ */
void App_Init(AppData* _appData){
    assert(_appData != NULL && "App_Init: argument is null");
    
    //debugf("App_Init\n");

    t3d_init((T3DInitParams){});

    //display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    display_init(RESOLUTION_640x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    
    // Init the ECS system
    //AF_ECS* ecs = &_appData->ecs;
    //assert(ecs != NULL && "App_Init: ecs is null");

    //AF_ECS_Init(ecs);
    //PrintAppDataSize(_appData);
    
    // Basic  setup for libdragon debugging
    // toggled to true or false at the entry point
    
    //console_init();
    if(_appData->gameplayData.isDebug == TRUE){
        //debug_initialize();
        //debug_init(DEBUG_FEATURE_LOG_USB);
        debugf("USB debugging Enabled!\n");
        debug_init_isviewer(); // this enables the ISViewer debug channel
        
        debug_init_usblog();
        console_set_debug(true);
        
        //rdpq_debug_start();
    }
    //PrintHeapStatus("N64 Console init: ");

    timer_init();

    // Tiny 3d stuff but
    //asset_init_compression(2);  // if we are loading assets that have level 2 compression we need this
    //dfs_init(DFS_DEFAULT_LOCATION);
    //PrintHeapStatus("dfs_init: ");
    
    // Now do system specific initialisation
    // Init Input
    AF_Input_Init();

    AF_Physics_Init(&_appData->ecs);
    // Init Rendering
   
    
    // 3D rendering
    Vec2 screenSize = {_appData->windowWidth, _appData->windowHeight};
    AF_Renderer_Init(&_appData->ecs, screenSize); 
    
    Scene_Awake(_appData);

    Scene_Start(_appData);
    
    // UI
    UI_Menu_Awake(_appData);

    UI_Menu_Start(_appData);
    
    // start renderer things that need to know about scene or game entities that have been setup
    AF_Renderer_LateStart(&_appData->ecs);
    
    
    // set framerate to target 60fp and call the app update function
    //new_timer(TIMER_TICKS(1000000 / 60), TF_CONTINUOUS, App_Update_Wrapper);

    // set the game state to player
   // _appData->gameplayData.gameState = GAME_STATE_PLAYING;
   
}


/* ===============
App_Update
Update function
================ */
void App_Update(AppData* _appData){
    assert(_appData != NULL && "App: App_Update: argument is null");
    
    //debugf("App_Update\n");
    //print to the screen
    // TODO: get input to retrun a struct of buttons pressed/held
    AF_Input_Update(&_appData->input);
    
    

    // update the game AI
    AI_Update(_appData);

    //AF_Physics_EarlyUpdate(&ecs);

    // TODO: pass input and ECS structs to the game to apply game logic
    //Game_Update(_appData);
    Scene_Update(_appData);
    AF_ECS* ecs = &_appData->ecs;
    AF_Time* time = &_appData->gameTime;
    
    // Physics
    AF_Physics_Update(ecs, time->timeSinceLastFrame);

    // late update for physics
    AF_Physics_LateUpdate(ecs);

    //Game_LateUpdate(_appData);
    Scene_LateUpdate(_appData);
    // TODO: Pass ECS entities to renderer to render them
    //AF_Renderer_Update(&ecs);
    //AF_Renderer_Debug();
    // render debug physics if enabled
    /*if(isDebug == TRUE){
        AF_Physics_LateRenderUpdate(&ecs);
    }*/
    //AF_Renderer_Finish(); 
    // update the tick
    time->currentTick++;
    //curTick++;
}


/* ===============
App_Render_Update
// Game Render Loop
// NOTE: this is indipendent from the other update functions which are operating on CPU Tick
// This render loop runs from a while loop in sandbox64.c
================ */
void App_Render_Update(AppData* _appData){

    
    assert(_appData != NULL && "App: App_Render_Update: argument is null");
    AF_Time* time = &_appData->gameTime;
    // Start Render loop
    AF_Renderer_Update(&_appData->ecs, time);
    //if(isDebug == TRUE){
        //AF_Physics_LateRenderUpdate(&ecs);
    //}
    // 
    
    // Render text
    UI_Menu_Update(_appData);
    AF_UI_Update(&_appData->ecs, &_appData->gameTime);

    AF_Renderer_Finish(); 
    
    time->currentFrame++;
}


/* ===============
App_Shutdown
Shutdown function
================ */
void App_Shutdown(AppData* _appData){
    assert(_appData != NULL && "App: App_Shutdown: argument is null");
    
    // UI Renderer
    AF_UI_Renderer_Shutdown(&_appData->ecs);

    // UI Menu
    UI_Menu_Shutdown(&_appData->ecs);

    // Scene
    Scene_Destroy(&_appData->ecs);

    // Physics
	AF_Physics_Shutdown();
    
    // Input
	AF_Input_Shutdown();

    // Timer Shutdown
    timer_close();

    // debug shutdown
    //PrintHeapStatus("Debug Shutdown TODO: ");

    // ECS
	//AF_ECS_Shutdown();
    //PrintHeapStatus("ECS Shutdown: ");

    // Renderer
	// TODO:this is out of order but matching game jam tmeplay shutdown order
    AF_Renderer_Shutdown(&_appData->ecs);

    t3d_destroy();
    // close the display used
    // TODO ths should go into renderer
    display_close();
}



/* ===============
PrintHeapStatus
// Print the N64 Heap status to the console
// Helpful for checking for memory leaks
================ */
void PrintHeapStatus(const char* _message){
    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);
    debugf("====\n%s: Mem: %d KiB\n====\n", _message, heap_stats.used/1024);
}