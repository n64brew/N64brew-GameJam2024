#include "App.h"
#include <stdio.h>
#include <libdragon.h>
#include "AF_Renderer.h"
#include "Game.h"
#include "AF_Input.h"
#include "AF_Physics.h"
#include "AF_UI.h"
// UFNLoader for n64
#include "debug.h"
#include "Scene.h"
#include "UI_Menu.h"
#include "AI.h"



//AF_Time* timerPtr;

//#define FRAME_BUFFER_SIZE 320*240*2
//display_context_t disp;
//char buffer[FRAME_BUFFER_SIZE];// = {0};

//const BOOL isDebug = TRUE;
//float deltaTime;
//int32_t curTick = 0;
//int32_t curFrame = 0;


//float frameMeasure;
//float updateMeasure;



// forward declare
void App_Update_Wrapper(int _ovfl);
//float App_Measure(void (*func)(int, ...), int num_args, ...);

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

void App_Init(AppData* _appData){
    assert(_appData != NULL && "App_Init: argument is null");
    debug_initialize();
    debug_printf("USB UFNLoader Enabled!\n");

    debugf("App_Init\n");

    
    // Init the ECS system
    AF_ECS* ecs = &_appData->ecs;
    assert(ecs != NULL && "App_Init: ecs is null");

    AF_ECS_Init(ecs);

    PrintAppDataSize(_appData);

    // Basic  setup for libdragon
    debug_init_isviewer(); // this enables the ISViewer debug channel
    console_init();
    debug_init_usblog();
    console_set_debug(true);
    timer_init();

    // Tiny 3d stuff but
    asset_init_compression(2);  // if we are loading assets that have level 2 compression we need this
    dfs_init(DFS_DEFAULT_LOCATION);
    
    // Now do system specific initialisation
    // Init Input
    AF_Input_Init();
    AF_Physics_Init(ecs);
    // Init Rendering
    
    // 3D rendering
    Vec2 screenSize = {_appData->windowWidth, _appData->windowHeight};
    AF_Renderer_Init(ecs, screenSize); 
    
    Game_Awake(ecs);
    Scene_Awake(_appData);

    Game_Start(ecs);
    Scene_Start(_appData);

    // UI
    UI_Menu_Awake(_appData);
    UI_Menu_Start(_appData);

    // start renderer things that need to know about scene or game entities that have been setup
    AF_Renderer_LateStart(ecs);
    
    // set framerate to target 60fp and call the app update function
    //new_timer(TIMER_TICKS(1000000 / 60), TF_CONTINUOUS, App_Update_Wrapper);

    // set the game state to player
   // _appData->gameplayData.gameState = GAME_STATE_PLAYING;
}

/*
void App_Update_Wrapper(int _ovfl){
    App_Update(&input, &ecs, timerPtr);
    //App_Measure(App_Update,&input, &ecs);
    //debugf("App_Update_Mesure(%f)\n", updateMeasure);
}*/


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
    Game_Update(_appData);
    AF_ECS* ecs = &_appData->ecs;
    AF_Time* time = &_appData->gameTime;
    // Physics
    AF_Physics_Update(ecs, time->timeSinceLastFrame);

    // late update for physics
    AF_Physics_LateUpdate(ecs);

    Game_LateUpdate(_appData);

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

// Game Render Loop
// NOTE: this is indipendent from the other update functions which are operating on CPU Tick
// This render loop runs from a while loop in sandbox64.c
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


void App_Shutdown(AppData* _appData){
    assert(_appData != NULL && "App: App_Shutdown: argument is null");
    if(_appData){}
	debugf("App_Shutdown\n");
    Game_Shutdown();
	AF_Renderer_Shutdown(&_appData->ecs);
    AF_UI_Renderer_Shutdown();
    UI_Menu_Shutdown(&_appData->ecs);
	AF_Physics_Shutdown();
	AF_Input_Shutdown();	
	//AF_ECS_Shutdown();

}

// TODO: Get this working
/*
float App_Measure(void (*func)(int, ...), int num_args, ...)
{
    uint64_t diff = 0;
    va_list args;
    rspq_wait();
    disable_interrupts();
    uint32_t t0 = get_ticks();
    // Initialize the argument list
    va_start(args, num_args);
        
    // Call the function with the variable arguments
    func(num_args, args);
        
    va_end(args);
    uint32_t t1 = get_ticks();
    enable_interrupts();
    diff += t1-t0;
    
    return TIMER_MICROS(diff) / 16.0f;
}*/