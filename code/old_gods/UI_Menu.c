/*================
UI_MENU_C
Implmentation of functions and code relating to the UI menu in the game
================*/
#include "UI_Menu.h"
#include <stdio.h>
#include "Assets.h"
#include "AF_UI.h"
#include "EntityFactory.h"
#include "GameplayData.h"



#include "../../minigame.h"
#include "../../core.h"

// Key mappings for n64 controller to joypad_button struct, polled from libdragon
#define A_KEY 0			// A Button		
#define B_KEY 1			// B Button
#define START_KEY 2		// Start Button

// Gameplay Screen
// TODO: wrap this into a struct
// ======== Text ========
const char *titleText = "og64 0.5\n";
char godsCountLabelText[20] = "666";
char countdownTimerLabelText[20] = "6666";

// player counter char buffers
char playerCountCharBuff[640];
const char* characterSpace = "    ";

char mainMenuTitleCharBuffer[20] = "Sneks";
char mainMenuSubTitleCharBuffer[40] = " Press (A)";

// Pause Menu
char pausedMenuTitleCharBuffer[7] = "PAUSED";
char pausedMenuSubTitle1CharBuffer[11] = "resume (A)";
char pausedMenuSubTitle2CharBuffer[11] = "exit (B)";

// GameOverScreen
char gameOverTitleCharBuffer[40] = "WINNER";
char gameOverSubTitle[40] = "Player 1";

// Start Countdown char
char startCountdownCharBuffer[16] = "0";

// UI Size Defines
#define PADDING 32
#define MARGIN 32
#define PADDING_MARGIN 64
// Setup some helpful vars for ui layout
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 240
#define SCREEN_HALF_WIDTH 320
#define SCREEN_HALF_HEIGHT 120


// UI COLORS
uint8_t whiteColor[4] = {255, 255, 255, 255};
uint8_t yellowColor[4] = {254, 222, 42, 255};
uint8_t pink[4] = {223, 82, 200, 255};

// ======== Entities ========
// Timer
AF_Entity* countdownTimerLabelEntity = NULL;

// Player Counters
AF_Entity* playersCountUIEntity = NULL;

// Main Menu
AF_Entity* mainMenuTitleEntity = NULL;
AF_Entity* mainMenuSubTitleEntity = NULL;
AF_Entity* mainMenuTitleBackground = NULL;
AF_Entity* mainMenuSubTitleBackground = NULL;

// Pause Menu
AF_Entity* pauseMenuTitleEntity = NULL;
AF_Entity* pauseMenuTitleBackground = NULL;

// Resume
AF_Entity* pauseMenuSubTitle1Entity = NULL;
AF_Entity* pauseMenuSubTitle1Background = NULL;

// Exit
AF_Entity* pauseMenuSubTitle2Entity = NULL;
AF_Entity* pauseMenuSubTitle2Background = NULL;

// Player Score Cards
AF_Entity* player1ScoreBackground = NULL;
AF_Entity* player2ScoreBackground = NULL;
AF_Entity* player3ScoreBackground = NULL;
AF_Entity* player4ScoreBackground = NULL;

// GameOverScreen
AF_Entity* gameOverTitleEntity = NULL;
AF_Entity* gameOverSubTitleEntity = NULL;
AF_Entity* gameOverTitleBackground = NULL;
AF_Entity* gameOverSubTitleBackground = NULL;

// START COUNTDOWN
AF_Entity* startCountdownUIBackgroundEntity = NULL;
AF_Entity* startCountdownLabelEntity = NULL;

// GAME DELAYS
#define COUNTDOWN_DELAY     3.0f
#define GO_DELAY            1.0f
#define WIN_DELAY           5.0f
#define WIN_SHOW_DELAY      2.0f

// ==== GAMEPLAY VARS ====
BOOL isDeclaredWinner = FALSE;
BOOL isStartedPlaying = FALSE;
BOOL isMusicPlaying = FALSE;
float countDownTimer;


// ==== GAME SOUNDS ====
wav64_t music_2;
wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;
wav64_t sfx_startButton;

// ==== FONT ====
rdpq_font_t* font2Ptr = NULL;
rdpq_font_t* font3Ptr = NULL;
rdpq_font_t* font4Ptr = NULL;
rdpq_font_t* font5Ptr = NULL;
rdpq_font_t* font6Ptr = NULL;

// ==== FORWARDS DECLARED FUNCTIONS ====
void UI_Menu_RenderGameOverScreen(AppData* _appData);
void RefreshCountdownTimer(AF_ECS* _ecs);

// Render Menu
void UI_Menu_RenderMainMenu(AppData* _appData);
void UI_Menu_RenderPlayingUI(AppData* _appData);
void UI_Menu_RenderCountdown(AppData* _appData);
void UI_Menu_RenderPausedScreen(AppData* _appData);

// Set Menu states
void UI_Menu_MainMenuSetShowing(BOOL _state);
void UI_Menu_GameOverUISetShowing(BOOL _state);
void UI_Menu_PlayingSetState(BOOL _state);
void UI_Menu_CountdownState(BOOL _state);

// Setup UI
void UI_Menu_SetupMainMenu(AppData* _appData);
void UI_Menu_SetupGameplayUI(AppData* _appData);
void UI_Menu_SetupPauseMenu(AppData* _appData);
void UI_Menu_SetupCountdownTimer(AppData* _appData);
void UI_Menu_SetupStartCountdownTimer(AppData* _appData);
void UI_Menu_SetupGameOverMenu(AppData* _appData);

void UI_Menu_SetupAudio();

void UI_Menu_Awake(AppData* _appData){

}




//======= START ========
/* ================
UI_Menu_Start
General functiont to get the setups started
 ================ */
void UI_Menu_Start(AppData* _appData){
    // Setup gameplay vars
    isDeclaredWinner = FALSE;
    isStartedPlaying = FALSE;

     // Setup the audio and countdown timer
    countDownTimer = COUNTDOWN_DELAY;

    // Setup Font Ptrs
    font2Ptr = (rdpq_font_t*)AF_LoadFont(FONT2_ID, fontPath2, pink);
    font3Ptr = (rdpq_font_t*)AF_LoadFont(FONT3_ID, fontPath3, whiteColor);
    font4Ptr = (rdpq_font_t*)AF_LoadFont(FONT4_ID, fontPath4, pink); 
    font5Ptr = (rdpq_font_t*)AF_LoadFont(FONT5_ID, fontPath5, whiteColor);
    font6Ptr = (rdpq_font_t*)AF_LoadFont(FONT6_ID, fontPath4, whiteColor);

    // COUNTDOWN TIMER
    UI_Menu_SetupCountdownTimer(_appData);

    // ======== MAIN MENU
    UI_Menu_SetupMainMenu(_appData);
    
    // ======== Player Score ========
    UI_Menu_SetupGameplayUI(_appData);

    // ======== GAME OVER =========
    UI_Menu_SetupGameOverMenu(_appData);
    
    // ======== Start COUNTDOWN UI ==========
    UI_Menu_SetupStartCountdownTimer(_appData);
    
    
    // ======== PAUSE MENU ======== 
    UI_Menu_SetupPauseMenu(_appData);

    // ======== AUDIO ======== 
    UI_Menu_SetupAudio();
}

//======= UPDATE ========
/* ================
UI_Menu_Update
UI state machine to switch between different UI menu systems
 ================ */
void UI_Menu_Update(AppData* _appData){
    switch (_appData->gameplayData.gameState)
    {
        case GAME_STATE_MAIN_MENU:
            UI_Menu_RenderMainMenu(_appData);
            //UI_Menu_RenderGameOverScreen(_appData);
        break;

        case GAME_STATE_COUNTDOWN:
            UI_Menu_RenderCountdown(_appData);
        break;

        case GAME_STATE_PLAYING:
        // TODO
            UI_Menu_RenderPlayingUI(_appData);
        break;

        case GAME_STATE_PAUSED:
            UI_Menu_RenderPausedScreen(_appData);
        break;

        case GAME_STATE_GAME_OVER_LOSE:
        // TODO
            UI_Menu_RenderGameOverScreen(_appData);
        break;

        case GAME_STATE_GAME_OVER_WIN:
            UI_Menu_RenderGameOverScreen(_appData);
        break;

        case GAME_STATE_GAME_RESTART:
            countDownTimer = 0;
            isDeclaredWinner = FALSE;
        break;

        case GAME_STATE_GAME_END:
            minigame_end();
        break;
    }
}


//======= SHUTDOWN ========
/* ================
UI_Menu_Shutdown
Shutdown sequence to free memory and clean up
 ================ */
void UI_Menu_Shutdown(AF_ECS* _ecs){
    // ===== Destroy Font ===== 
    debugf("UI Renderer Shutdown: Unregistering fonts \n");
    rdpq_text_unregister_font(FONT2_ID);
    rdpq_text_unregister_font(FONT3_ID);
    rdpq_text_unregister_font(FONT4_ID);
    rdpq_text_unregister_font(FONT5_ID);
    rdpq_text_unregister_font(FONT6_ID);

    
    // ===== free the font ===== 
    rdpq_font_free(font2Ptr);
    rdpq_font_free(font3Ptr);
    rdpq_font_free(font4Ptr);
    rdpq_font_free(font5Ptr);
    rdpq_font_free(font6Ptr);

    // Free the loaded sprites in memory
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        AF_Entity* entity = &_ecs->entities[i];
        BOOL hasSprite = AF_Component_GetHas(entity->sprite->enabled);
        if(hasSprite){
            AF_CSprite* sprite = entity->sprite;
            sprite_t* spriteData = (sprite_t*)sprite->spriteData;
            if(spriteData != NULL){
                debugf("UI Renderer Shutdown: freeing sprites %i \n", i);
                sprite_free(spriteData);
            }
        }
    }

    // ===== Destroy Audio ===== 
    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);
    mixer_ch_stop(0);
    wav64_close(&music_2);
    wav64_close(&sfx_startButton);
}

//======= SETUP HELPERS ========
/* ================
UI_Menu_SetupStartCountdownTimer
Setup the start countdown timer ui elements
Title, sub title, (sprite & text)
 ================ */
void UI_Menu_SetupStartCountdownTimer(AppData* _appData){
    // Setup Start Countdown
    Vec2 countdownSpriteSize = {256,256};
    Vec2 countdownBoxSize = {SCREEN_WIDTH, 0};
    Vec2 startCountDownSpriteScale = {0.4f, 0.2f};
    Vec2 startCountdownTitleBackgroundPos = {SCREEN_HALF_WIDTH - (countdownSpriteSize.x * 0.2f), SCREEN_HALF_HEIGHT - 100};
    Vec2 startCountdownTitleLabelPos = {startCountdownTitleBackgroundPos.x + 45,startCountdownTitleBackgroundPos.y + 35};
    startCountdownLabelEntity = Entity_Factory_CreateUILabel(&_appData->ecs, startCountdownCharBuffer, FONT4_ID, fontPath4, whiteColor, startCountdownTitleLabelPos, countdownBoxSize);
    startCountdownUIBackgroundEntity = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_0],startCountdownTitleBackgroundPos, startCountDownSpriteScale,countdownSpriteSize,pink,0,countdownBoxSize);
    
}

/* ================
UI_Menu_SetupGameOverMenu
Setup the game over menu ui elements
Title, sub title, (sprite & text)
 ================ */
void UI_Menu_SetupGameOverMenu(AppData* _appData){
    Vec2 titlePos = {SCREEN_HALF_WIDTH - (SCREEN_HALF_WIDTH*.5f), PADDING_MARGIN};
    // game over text
    // title background elements
    Vec2 titleSpriteScale = {1.5f, 0.25f};
    Vec2 gameOverTitleSpriteSize = {256,256};
    Vec2 gameOverTitleSpriteScale = {.75f, 0.25f};
    Vec2 gameOverSpriteScale = {1.25f, 0.25f};
    Vec2 titleSpriteSize = {256, 256};

    Vec2 gameOverTitleBoxSize = {SCREEN_WIDTH, 0};
    Vec2 gameOverSubTitleBoxSize = {SCREEN_WIDTH, 0};


    Vec2 gameOverTitleBackgroundPos = {SCREEN_HALF_WIDTH- ((titleSpriteSize.x*titleSpriteScale.x)* 0.25f), titlePos.y - ((titleSpriteSize.y*titleSpriteScale.y)* 0.55f)};
    Vec2 gameOverSubTitleBackgroundPos = {gameOverTitleBackgroundPos.x -32, SCREEN_HALF_HEIGHT + (titleSpriteSize.y*titleSpriteScale.y)* 0.5f};//gameOverSubTitlePos.y - 32};
    
    Vec2 gameOverTitleTextPos = {SCREEN_HALF_WIDTH- ((titleSpriteSize.x*titleSpriteScale.x)* 0.1f ), titlePos.y +4};//- ((titleSpriteSize.y*titleSpriteScale.y)* 0.75f)};
    Vec2 gameOverSubTitleTextPos = {SCREEN_HALF_WIDTH- (((titleSpriteSize.x*titleSpriteScale.x)* 0.15f) ), gameOverSubTitleBackgroundPos.y + 44};//subTitlePos.y};
    
    //gameOverTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, gameOverTitleCharBuffer, FONT3_ID, fontPath3, whiteColor, gameOverTitleTextPos, mainMenuTitleSize);
    // TODO: setting the colour here does nothing
    //gameOverTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, gameOverTitleCharBuffer, FONT3_ID, fontPath3, whiteColor, gameOverTitleTextPos, gameOverTitleBoxSize);
    gameOverTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, gameOverTitleCharBuffer, FONT6_ID, fontPath4, whiteColor, gameOverTitleTextPos, gameOverTitleBoxSize);
    gameOverSubTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, gameOverSubTitle, FONT4_ID, fontPath4, whiteColor, gameOverSubTitleTextPos, gameOverSubTitleBoxSize);

    gameOverTitleEntity->text->isShowing = FALSE;
    gameOverSubTitleEntity->text->isShowing = FALSE;

    gameOverTitleBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_1],gameOverTitleBackgroundPos, gameOverTitleSpriteScale,gameOverTitleSpriteSize,whiteColor,0,gameOverTitleBoxSize);
    gameOverSubTitleBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_0],gameOverSubTitleBackgroundPos, gameOverSpriteScale,gameOverTitleSpriteSize,pink,0,gameOverSubTitleBoxSize);
    gameOverTitleBackground->sprite->spriteRotation = 0.1f;
    gameOverSubTitleBackground->sprite->spriteRotation = -.1f;
}


/* ================
UI_Menu_SetupMainMenu
Setup the main menu ui elements
Title, sub title, (sprite & text)
 ================ */
void UI_Menu_SetupMainMenu(AppData* _appData){
    Vec2 titlePos = {SCREEN_HALF_WIDTH - (SCREEN_HALF_WIDTH*.5f), PADDING_MARGIN};

    // Create Main Menu
    Vec2 mainMenuTitleSize = {SCREEN_WIDTH, 0};
    Vec2 mainMenuSubTitleSize = {SCREEN_WIDTH, 0};
    
    // title background elements
    Vec2 titleSpriteSize = {256, 256};
    Vec2 titleSheetSpriteSize = {256, 256};
    
    Vec2 titleSpriteScale = {1.5f, 0.25f};
    
    Vec2 subTitleSpriteSize = {256, 256};
    Vec2 subTitleSheetSpriteSize = {256, 256};
    Vec2 subTitleSpriteScale = {1.25, 0.25f};


    Vec2 mainMenuTitleBackgroundPos = {SCREEN_HALF_WIDTH- ((titleSpriteSize.x*titleSpriteScale.x)* 0.5f), titlePos.y - ((titleSpriteSize.y*titleSpriteScale.y)* 0.75f)};
    Vec2 mainMenuSubTitleBackgroundPos = {mainMenuTitleBackgroundPos.x+48, SCREEN_HALF_HEIGHT + (titleSpriteSize.y*titleSpriteScale.y)* 0.5f};//gameOverSubTitlePos.y - 32};
    

    Vec2 mainMenuTitleTextPos = {SCREEN_HALF_WIDTH- ((titleSpriteSize.x*titleSpriteScale.x)* 0.2f ), titlePos.y +4};//- ((titleSpriteSize.y*titleSpriteScale.y)* 0.75f)};
    Vec2 mainMenuSubTitleTextPos = {SCREEN_HALF_WIDTH- ((titleSpriteSize.x*titleSpriteScale.x)* 0.25f ), mainMenuSubTitleBackgroundPos.y + 42};//subTitlePos.y};
    
    
    mainMenuTitleBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_1],mainMenuTitleBackgroundPos, titleSpriteScale,titleSpriteSize,pink,0,titleSheetSpriteSize);
    mainMenuSubTitleBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_0],mainMenuSubTitleBackgroundPos, subTitleSpriteScale,subTitleSpriteSize,whiteColor,0,subTitleSheetSpriteSize);
    AF_CSprite* mainMenuTitleBackgroundSprite = mainMenuTitleBackground->sprite;
    mainMenuTitleBackgroundSprite->filtering = TRUE;
    mainMenuSubTitleBackground->sprite->filtering = TRUE;
    mainMenuTitleBackgroundSprite->spriteRotation = 0.1f;
    mainMenuSubTitleBackground->sprite->spriteRotation = -.1f;
    
    mainMenuTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, mainMenuTitleCharBuffer, FONT3_ID, fontPath3, whiteColor, mainMenuTitleTextPos, mainMenuTitleSize);
    mainMenuSubTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, mainMenuSubTitleCharBuffer, FONT4_ID, fontPath4, whiteColor, mainMenuSubTitleTextPos, mainMenuSubTitleSize);

}

/* ================
UI_Menu_SetupGameplayUI
Setup the gameplay ui elements
player cards, timer
 ================ */
void UI_Menu_SetupGameplayUI(AppData* _appData){
    Vec2 playerScoreBackgroundSize = {64,64};
    Vec2 playerScoreBackgroundSizeScale = {1.0f, 0.6f};
    float scorePadding = (SCREEN_WIDTH - (PADDING*2)) * .15f;

    float scoreBasePosX = PADDING_MARGIN*2 + 24;
    float scoreBasePosY = SCREEN_HEIGHT - (PADDING + 15);


    // TODO: clean this up
    Vec2 player1ScoreBackgroundSpritePos = {scoreBasePosX, scoreBasePosY};
    // TODO: fix the magic numbers, have to do this as font size is making things a bit hard
    Vec2 player2ScoreBackgroundSpritePos = {scoreBasePosX + (scorePadding) + 5, scoreBasePosY};
    Vec2 player3coreBackgroundSpritePos = {scoreBasePosX + (2 * scorePadding) + 12,scoreBasePosY};
    Vec2 player4ScoreBackgroundSpritePos = {scoreBasePosX + (3 * scorePadding) + 20, scoreBasePosY};

    Vec2 scoreTextPos = {player1ScoreBackgroundSpritePos.x + 8, player1ScoreBackgroundSpritePos.y - 15};

    // Create Player 1 card
    Vec2 playe1CountLabelSize = {SCREEN_WIDTH, PADDING_MARGIN};
    playersCountUIEntity = Entity_Factory_CreateUILabel(&_appData->ecs, playerCountCharBuff, FONT5_ID, fontPath5, yellowColor, scoreTextPos, playe1CountLabelSize);
    
    player1ScoreBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_2],player1ScoreBackgroundSpritePos, playerScoreBackgroundSizeScale,playerScoreBackgroundSize,whiteColor,0,playerScoreBackgroundSizeScale);
    player2ScoreBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_3],player2ScoreBackgroundSpritePos, playerScoreBackgroundSizeScale,playerScoreBackgroundSize,whiteColor,0,playerScoreBackgroundSizeScale);
    player3ScoreBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_4],player3coreBackgroundSpritePos, playerScoreBackgroundSizeScale,playerScoreBackgroundSize,whiteColor,0,playerScoreBackgroundSizeScale);
    player4ScoreBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_5],player4ScoreBackgroundSpritePos, playerScoreBackgroundSizeScale,playerScoreBackgroundSize,whiteColor,0,playerScoreBackgroundSizeScale);
    
}

/* ================
UI_Menu_SetupCountdownTimer
Setup the gameplay ui elements
player cards, timer
 ================ */
void UI_Menu_SetupCountdownTimer(AppData* _appData){
    int countdownTimerBox_width = SCREEN_WIDTH + PADDING_MARGIN;
    int countdownTimerBox_height = 0;
    int countdownTimerBoxPosX = SCREEN_HALF_WIDTH - MARGIN;

	Vec2 countdownTimerLabelTextScreenPos = {countdownTimerBoxPosX, PADDING};
	Vec2 countdownTimerLabelTextBounds = {countdownTimerBox_width, countdownTimerBox_height};

    countdownTimerLabelEntity = Entity_Factory_CreateUILabel(&_appData->ecs, countdownTimerLabelText, FONT5_ID, fontPath5, whiteColor, countdownTimerLabelTextScreenPos, countdownTimerLabelTextBounds);
}

/* ================
UI_Menu_SetupPauseMenu
Setup the pause menu ui elements
player cards, timer
 ================ */
void UI_Menu_SetupPauseMenu(AppData* _appData){

    // ==== PAUSED Menu ====
    // title background elements
    Vec2 pauseTitleSpriteSize = {256, 256};
    Vec2 pauseTitleSheetSpriteSize = {256, 256};
    Vec2 pauseTitleSpriteScale = {2.0f, 0.25f};

    // Text Pos
    Vec2 pauseMenuTitlePos = {SCREEN_HALF_WIDTH - (SCREEN_HALF_WIDTH*.25f) , PADDING + MARGIN};
    // sprite pos
    Vec2 pauseMenutitleSpritePos = {SCREEN_HALF_WIDTH- ((pauseTitleSpriteSize.x*pauseTitleSpriteScale.x)* 0.5f), pauseMenuTitlePos.y - ((pauseTitleSpriteSize.y*pauseTitleSpriteScale.y)* 0.75f)};
    // Size
    Vec2 pauseMenuTitleSize = {SCREEN_WIDTH, 0};

    // Label entity
    pauseMenuTitleBackground = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_1],pauseMenutitleSpritePos, pauseTitleSpriteScale, pauseTitleSpriteSize, pink, 0, pauseTitleSheetSpriteSize);
    // text Entity
    pauseMenuTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, pausedMenuTitleCharBuffer, FONT3_ID, fontPath3, whiteColor, pauseMenuTitlePos, pauseMenuTitleSize);

     // sprite settings
    pauseMenuTitleBackground->sprite->filtering = TRUE;
    pauseMenuTitleBackground->sprite->spriteRotation = 0.1f;

    // ==== Sub title 1 - Resume ====
    Vec2 pauseSubTitle1SpriteSize = {256, 256};
    Vec2 pauseSubTitle1SheetSpriteSize = {256, 256};
    Vec2 pauseSubTitle1SpriteScale = {1, 0.25f};
    Vec2 pauseMenuSubTitle1Size = pauseMenuTitleSize;
    Vec2 pauseMenuSubTitle1Pos = {SCREEN_HALF_WIDTH- ((pauseSubTitle1SpriteSize.x*pauseSubTitle1SpriteScale.x)), SCREEN_HALF_HEIGHT};
    Vec2 pauseMenuSubTitle1TextPos = {SCREEN_HALF_WIDTH - (116*2),  SCREEN_HALF_HEIGHT + 32+12};

    // Label Entity
    pauseMenuSubTitle1Entity = Entity_Factory_CreateUILabel(&_appData->ecs, pausedMenuSubTitle1CharBuffer, FONT4_ID, fontPath4, whiteColor, pauseMenuSubTitle1TextPos, pauseMenuSubTitle1Size);
    // Sprite entity
    pauseMenuSubTitle1Background = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_0], pauseMenuSubTitle1Pos, pauseSubTitle1SpriteScale, pauseSubTitle1SpriteSize, whiteColor, 0, pauseSubTitle1SheetSpriteSize);
    
    // turn on sprite filtering
    pauseMenuSubTitle1Background->sprite->filtering = TRUE;

    // Rotate the spirtes slightly
    pauseMenuSubTitle1Background->sprite->spriteRotation = -.1f;

    // ==== Sub title 2- Resume ====
    Vec2 pauseSubTitle2SpriteSize = {256, 256};
    Vec2 pauseSubTitle2SheetSpriteSize = {256, 256};
    Vec2 pauseSubTitle2SpriteScale = {1, 0.25f};
    //Vec2 pauseMenuSubTitle2Size = pauseMenuTitleSize;
    Vec2 pauseMenuSubTitle2Pos = {SCREEN_HALF_WIDTH + ((pauseSubTitle2SpriteSize.x*pauseSubTitle2SpriteScale.x)* 0.25f), SCREEN_HALF_HEIGHT};
    Vec2 pauseMenuSubTitle2TextPos = {SCREEN_HALF_WIDTH + 116,  SCREEN_HALF_HEIGHT + 32+12};

    // Label Entity
    pauseMenuSubTitle2Entity = Entity_Factory_CreateUILabel(&_appData->ecs, pausedMenuSubTitle2CharBuffer, FONT4_ID, fontPath4, whiteColor, pauseMenuSubTitle2TextPos, pauseMenuSubTitle1Size);
    // Sprite entity
    pauseMenuSubTitle2Background = Entity_Factory_CreateSprite(&_appData->ecs, texture_path[TEXTURE_ID_0], pauseMenuSubTitle2Pos, pauseSubTitle2SpriteScale, pauseSubTitle2SpriteSize, whiteColor, 0, pauseSubTitle2SheetSpriteSize);
    
    // turn on sprite filtering
    pauseMenuSubTitle2Background->sprite->filtering = TRUE;

    // Rotate the spirtes slightly
    pauseMenuSubTitle2Background->sprite->spriteRotation = -.1f;
}


// ============== SET STATE ================
/* ================
UI_Menu_MainMenuSetShowing
Set state for main menu
 ================ */
void UI_Menu_MainMenuSetShowing(BOOL _state){
    // Main Menu
    mainMenuTitleEntity->text->isShowing = _state;
    mainMenuSubTitleEntity->text->isShowing = _state;
    mainMenuTitleBackground->text->isShowing = _state;
    // background panels
    mainMenuTitleBackground->sprite->enabled= AF_Component_SetEnabled(mainMenuTitleBackground->sprite->enabled, _state);
    mainMenuSubTitleBackground->sprite->enabled =AF_Component_SetEnabled(mainMenuSubTitleBackground->sprite->enabled, _state);
}

/* ================
UI_Menu_PauseMenuSetShowing
Set state for pause menu
 ================ */
void UI_Menu_PauseMenuSetShowing(BOOL _state){
    // Text
    pauseMenuTitleEntity->text->isShowing = _state;
    pauseMenuSubTitle1Entity->text->isShowing = _state;
    pauseMenuSubTitle2Entity->text->isShowing = _state;

    // sprite
    pauseMenuTitleBackground->text->isShowing = _state;
    pauseMenuSubTitle1Background->text->isShowing = _state;
    pauseMenuSubTitle2Background->text->isShowing = _state;

    // background panels
    pauseMenuTitleBackground->sprite->enabled= AF_Component_SetEnabled(pauseMenuTitleBackground->sprite->enabled, _state);
    pauseMenuSubTitle1Background->sprite->enabled =AF_Component_SetEnabled(pauseMenuSubTitle1Background->sprite->enabled, _state);
    pauseMenuSubTitle2Background->sprite->enabled =AF_Component_SetEnabled(pauseMenuSubTitle2Background->sprite->enabled, _state);
}

/* ================
UI_Menu_GameOverUISetShowing
Set State for Game Over UI
 ================ */
void UI_Menu_GameOverUISetShowing(BOOL _state){
    // Game Over
    
    gameOverTitleEntity->text->isShowing = _state;
     
    gameOverSubTitleEntity->text->isShowing = _state;
   
    gameOverTitleBackground->sprite->enabled= AF_Component_SetEnabled(gameOverTitleBackground->sprite->enabled, _state);
    gameOverSubTitleBackground->sprite->enabled =AF_Component_SetEnabled(gameOverSubTitleBackground->sprite->enabled, _state);
    /**/
    
}

/* ================
UI_Menu_PlayingSetState
Set State for Player UI
 ================ */
void UI_Menu_PlayingSetState(BOOL _state){
    playersCountUIEntity->text->isShowing = _state;
    countdownTimerLabelEntity->text->isShowing = _state;

    //toggle the score backgrounds
    player1ScoreBackground->sprite->enabled = AF_Component_SetEnabled(player1ScoreBackground->sprite->enabled, _state);
    player2ScoreBackground->sprite->enabled = AF_Component_SetEnabled(player2ScoreBackground->sprite->enabled, _state);
    player3ScoreBackground->sprite->enabled = AF_Component_SetEnabled(player3ScoreBackground->sprite->enabled, _state);
    player4ScoreBackground->sprite->enabled = AF_Component_SetEnabled(player4ScoreBackground->sprite->enabled, _state);   
}

/* ================
UI_Menu_CountdownState
Set State for Count down timer UI
 ================ */
void UI_Menu_CountdownState(BOOL _state){
    // Player Counts hid
    startCountdownUIBackgroundEntity->text->isShowing = _state;
    startCountdownLabelEntity->text->isShowing = _state;
    startCountdownUIBackgroundEntity->sprite->enabled = AF_Component_SetEnabled(startCountdownUIBackgroundEntity->sprite->enabled, _state);
    startCountdownLabelEntity->sprite->enabled = AF_Component_SetEnabled(startCountdownLabelEntity->sprite->enabled, _state); 
}

// ================ RENDER UI ================ 
/* ================
UI_Menu_RenderMainMenu
Set State for Count down timer UI
 ================ */
void UI_Menu_RenderMainMenu(AppData* _appData){
    UI_Menu_MainMenuSetShowing(TRUE);
    UI_Menu_GameOverUISetShowing(FALSE);
    UI_Menu_PlayingSetState(FALSE);
    UI_Menu_CountdownState(FALSE);
    UI_Menu_PauseMenuSetShowing(FALSE);

    // TODO: tidy this up
    if(isMusicPlaying == FALSE){
        // TODO: make this read from assets
        wav64_open(&music_2, "rom:/old_gods/sandy_seaside.wav64");
        // set sound to loop
        wav64_set_loop(&music_2, true);
        wav64_play(&music_2, 0);
        isMusicPlaying = TRUE;
    }
    

    // detect start button pressed
    if(_appData->input.keys[A_KEY]->pressed == TRUE){
        wav64_play(&sfx_startButton, 31);
        GameplayData* gameplayData = &_appData->gameplayData;
        // gods count reset
        gameplayData->godEatCount = 0;
        // countdown Time
        gameplayData->countdownTimer = COUNT_DOWN_TIME;
        gameplayData->gameState = GAME_STATE_COUNTDOWN;
    }
}


/* ================
UI_Menu_RenderPlayingUI
Game_UpdatePlayerScoreText
Update the UI score elements
================ */
void UI_Menu_RenderPlayingUI(AppData* _appData){
    // TODO: dont run these commands every frame
    UI_Menu_MainMenuSetShowing(FALSE);
    UI_Menu_GameOverUISetShowing(FALSE);
    UI_Menu_PlayingSetState(TRUE);
    UI_Menu_CountdownState(FALSE);
    UI_Menu_PauseMenuSetShowing(FALSE);

    GameplayData* gameplayData = &_appData->gameplayData;
    sprintf(playerCountCharBuff, " %i%s %i%s%i%s %i", 
        (int)gameplayData->playerEntities[0]->playerData->score, 
        characterSpace, 
        (int)gameplayData->playerEntities[1]->playerData->score, 
        characterSpace,
         (int)gameplayData->playerEntities[2]->playerData->score, 
         characterSpace, 
         (int)gameplayData->playerEntities[3]->playerData->score);
    
    playersCountUIEntity->text->text = playerCountCharBuff;
    playersCountUIEntity->text->isDirty = TRUE;

    AF_Time* time = &_appData->gameTime;

    // Update countdown timer
    gameplayData->countdownTimer -= time->timeSinceLastFrame;
    sprintf(countdownTimerLabelText, "%i", (int)gameplayData->countdownTimer);
    countdownTimerLabelEntity->text->text = countdownTimerLabelText;
    countdownTimerLabelEntity->text->isDirty = TRUE;

    if(gameplayData->countdownTimer <= 0){
        gameplayData->gameState = GAME_STATE_GAME_OVER_LOSE;
        gameplayData->countdownTimer = COUNT_DOWN_TIME;
    }

    if(gameplayData->godEatCount == GODS_EAT_COUNT){
        gameplayData->gameState = GAME_STATE_GAME_OVER_WIN;
        gameplayData->countdownTimer = COUNT_DOWN_TIME;
    }

    // detect start button pressed by any player
    for(int i = 0; i < PLAYER_COUNT; ++i){
        if(_appData->input.keys[i][START_KEY].pressed == TRUE){
                debugf("Pause game\n");
                wav64_play(&sfx_startButton, 31);
                GameplayData* gameplayData = &_appData->gameplayData;
                gameplayData->gameState = GAME_STATE_PAUSED;
                // TODO: hack to force the key off so the resume works ocrrectly.
        }
    }
}

/* ================
UI_Menu_RenderGameOverScreen
Render Game Over screen UI
 ================ */
void UI_Menu_RenderGameOverScreen(AppData* _appData ){
    UI_Menu_MainMenuSetShowing(FALSE);
    UI_Menu_PlayingSetState(FALSE);
    UI_Menu_CountdownState(FALSE);
    UI_Menu_PauseMenuSetShowing(FALSE);
    
    
    GameplayData* gameplayData = &_appData->gameplayData;
    int highestScore = 0;
    int playerWithHighestScore = 0;
    for(int i = 0; i < PLAYER_COUNT; ++i){
       int playerScore = _appData->gameplayData.playerEntities[i]->playerData->score;
       if(playerScore > highestScore){
        highestScore = playerScore;
        playerWithHighestScore = i;
       }
    }
    
    // count is from 0, but for UX we add 1
    sprintf(gameOverSubTitle, "Player %i ",playerWithHighestScore+1);
     if(gameplayData->gameState == GAME_STATE_GAME_OVER_WIN){
        gameOverSubTitleEntity->text->text = gameOverSubTitle;
     }else if(gameplayData->gameState == GAME_STATE_GAME_OVER_LOSE){
        gameOverSubTitleEntity->text->text = gameOverSubTitle;
     }

     // position winning player in centre
     AF_Entity* winningPlayer = _appData->gameplayData.playerEntities[playerWithHighestScore];
     Vec3 winnerSpawnSpot = {0,0,1.1};
     winningPlayer->transform->pos = winnerSpawnSpot;

    
    
     UI_Menu_GameOverUISetShowing(TRUE);
    
    // Game Jam CORE MINI GAME end game stuff
    // only call this once
    // TODO: tidy this up
    if(isDeclaredWinner == FALSE){
        mixer_ch_stop(0);
        wav64_close(&music_2);
        wav64_play(&sfx_winner, 31);
        //xm64player_stop(&music);
        isDeclaredWinner = TRUE;
        isMusicPlaying = FALSE;
    }
   
    
    for(int i = 0; i < PLAYER_COUNT; ++i){
        // detect start button pressed to restart the game
        if(_appData->input.keys[i][A_KEY].pressed == TRUE){
            gameplayData->gameState = GAME_STATE_GAME_END;
            core_set_winner(playerWithHighestScore);
            minigame_end(); 
        }

        // Let the game jam template handle the game ending
        if(_appData->input.keys[i][START_KEY].pressed == TRUE){
            //debugf("End minigame\n");
            gameplayData->gameState = GAME_STATE_GAME_END;
            core_set_winner(playerWithHighestScore);
            minigame_end(); 
        }
    }
}


/* ================
UI_Menu_RenderCountdown
Render in game count down clock
 ================ */
void UI_Menu_RenderCountdown(AppData* _appData){
    UI_Menu_MainMenuSetShowing(FALSE);
    UI_Menu_GameOverUISetShowing(FALSE);
    UI_Menu_PlayingSetState(FALSE);
    UI_Menu_CountdownState(TRUE);

    // this will loop 3 times then progress
    if (countDownTimer > -GO_DELAY)
    {
        float prevCountDown = countDownTimer;
        countDownTimer -= _appData->gameTime.timeSinceLastFrame;
        if ((int)prevCountDown != (int)countDownTimer && countDownTimer >= 0){
            wav64_play(&sfx_countdown, 31);
            // update the char buffer that will be the onscreen text. ensure there
            sprintf(startCountdownCharBuffer, "%i", ((int)countDownTimer)+1);
            startCountdownLabelEntity->text->text = startCountdownCharBuffer;
            startCountdownLabelEntity->text->isDirty = TRUE;
        }
        return;
    }
    // this will only play once
    wav64_play(&sfx_start, 31);
    isStartedPlaying = TRUE;
   
    
    startCountdownLabelEntity->text->isDirty = TRUE;
    // TODO: add a final slight delay to allow the words GO!!! to be read
     _appData->gameplayData.gameState = GAME_STATE_PLAYING;
}

/* ================
UI_Menu_RenderPausedScreen
Render paused screen
 ================ */
void UI_Menu_RenderPausedScreen(AppData* _appData){
    UI_Menu_PauseMenuSetShowing(TRUE);
    UI_Menu_MainMenuSetShowing(FALSE);
    UI_Menu_GameOverUISetShowing(FALSE);
    UI_Menu_PlayingSetState(FALSE);
    UI_Menu_CountdownState(FALSE);

        // detect start button pressed by any player
        
        for(int i = 0; i < PLAYER_COUNT; ++i){
            if(_appData->input.keys[i][A_KEY].pressed == TRUE){
                    debugf("Resume game\n");
                    wav64_play(&sfx_startButton, 31);
                    GameplayData* gameplayData = &_appData->gameplayData;
                    gameplayData->gameState = GAME_STATE_PLAYING;
            }

            if(_appData->input.keys[i][B_KEY].pressed == TRUE){
                    debugf("Exit game\n");
                    wav64_play(&sfx_startButton, 31);
                    GameplayData* gameplayData = &_appData->gameplayData;
                    gameplayData->gameState = GAME_STATE_GAME_END;
            }
        }
}


// ================ AUDIO SETUP ================ 
/* ================
UI_Menu_RenderCountdown
Render in game count down clock
 ================ */
void UI_Menu_SetupAudio(){
  wav64_open(&sfx_start, AUDIO_START_FX); //"rom:/core/Start.wav64");
  wav64_open(&sfx_countdown, AUDIO_COUNTDOWN_FX); //"rom:/core/Countdown.wav64");
  wav64_open(&sfx_stop, AUDIO_STOP_FX); //"rom:/core/Stop.wav64");
  wav64_open(&sfx_winner, AUDIO_WINNER_FX); //"rom:/core/Winner.wav64");
  wav64_open(&sfx_startButton, AUDIO_BUTTON_PRESS_FX); //"rom:/old_gods/Item2A.wav64");
  
  mixer_ch_set_vol(31, 0.5f, 0.5f);
}

