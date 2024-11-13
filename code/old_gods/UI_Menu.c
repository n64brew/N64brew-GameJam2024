#include "UI_Menu.h"
#include <stdio.h>
#include "Assets.h"
#include "AF_UI.h"
#include "EntityFactory.h"

// Key mappings for n64 controller to joypad_button struct, polled from libdragon
#define A_KEY 0			// A Button		
#define B_KEY 1			// B Button
#define START_KEY 2		// Start Button

// Gameplay Screen
// TODO: wrap this into a struct
// ======== Text ========
const char *titleText = "og64 0.1\n";
char godsCountLabelText[20] = "666";
char countdownTimerLabelText[20] = "6666";

// player counter char buffers
char playerCountCharBuff[320] = "0                 0                  0                  0";

char mainMenuTitleCharBuffer[20] = "OLD GODS";
char mainMenuSubTitleCharBuffer[40] = "Press Start to begin";

// GameOverScreen
char gameOverTitleCharBuffer[20] = "Game Over";
char gameOverSubTitleLoseCharBuffer[80] = "You have failed to contain the the god\nPress Start to restart";
char gameOverSubTitleWinCharBuffer[80] = "You have succeded to contain the the god\nPress Start to restart";

// ======== Entities ========
AF_Entity* gameTitleEntity = NULL;
// Timer
AF_Entity* godEatCountLabelEntity = NULL;
AF_Entity* countdownTimerLabelEntity = NULL;

// Player Counters
AF_Entity* playersCountUIEntity = NULL;

// Main Menu
AF_Entity* mainMenuTitleEntity = NULL;
AF_Entity* mainMenuSubTitleEntity = NULL;

// GameOverScreen
AF_Entity* gameOverTitleEntity = NULL;
AF_Entity* gameOverSubTitleEntity = NULL;


// Forward decalred functions
//void Game_UpdatePlayerScoreText();


void UI_Menu_RenderGameOverScreen(AppData* _appData);
void RefreshCountdownTimer(AF_ECS* _ecs);
// set ui states
void UI_Menu_RenderMainMenu(AppData* _appData);
void UI_Menu_RenderPlayingUI(AppData* _appData);
void UI_Menu_MainMenuSetShowing(BOOL _state);
void UI_Menu_GameOverUISetShowing(BOOL _state);
void UI_Menu_PlayingSetState(BOOL _state);

void UI_Menu_Awake(AppData* _appData){

}


//======= SETUP ========

void UI_Menu_Start(AppData* _appData){
    
    // TODO: get rid of magic numbers
    // Create test Text
    //int FONT1_ID = 1;
    //int FONT2_ID = 2;
    float whiteColor[4];
    whiteColor[0] = 255;
    whiteColor[1] = 255;
    whiteColor[2] = 255;
    whiteColor[3] = 255;

    AF_LoadFont(FONT1_ID, fontPath, whiteColor);
    AF_LoadFont(FONT2_ID, fontPath2, whiteColor); // title font
	gameTitleEntity = AF_ECS_CreateEntity(&_appData->ecs);
	*gameTitleEntity->text = AF_CText_ADD();

	gameTitleEntity->text->text = titleText;
	gameTitleEntity->text->fontID = 1;
	gameTitleEntity->text->fontPath = fontPath2;

	// Text Color
	//gameTitleEntity->text->textColor = whiteColor;
    gameTitleEntity->text->fontID = FONT1_ID;

	// Title Text position
    int box_width = 262;
    int box_height = 0;
    int x0 = 10;
	int y0 = 20;

	Vec2 textScreenPos = {x0, y0};
	Vec2 textBounds = {box_width, box_height};
    gameTitleEntity->text->screenPos = textScreenPos;
	gameTitleEntity->text->textBounds = textBounds;

    // ======God Eat Label Text position
    godEatCountLabelEntity = AF_ECS_CreateEntity(&_appData->ecs);
	*godEatCountLabelEntity->text = AF_CText_ADD();
	godEatCountLabelEntity->text->text = godsCountLabelText;
	godEatCountLabelEntity->text->fontID = FONT2_ID;
	godEatCountLabelEntity->text->fontPath = fontPath2;

	// Text Color
    godEatCountLabelEntity->text->fontID = FONT2_ID;

    int godLabelBox_width = 262;
    int godLabelBox_height = 0;
    int godLabelBoxPosX = 250;
	int godLabelBoxPosY = 20;

	Vec2 godLabelTextScreenPos = {godLabelBoxPosX, godLabelBoxPosY};
	Vec2 godLabelTextBounds = {godLabelBox_width, godLabelBox_height};
    godEatCountLabelEntity->text->screenPos = godLabelTextScreenPos;
	godEatCountLabelEntity->text->textBounds = godLabelTextBounds;

    // ======Count down timer Text position
    countdownTimerLabelEntity = AF_ECS_CreateEntity(&_appData->ecs);
	*countdownTimerLabelEntity->text = AF_CText_ADD();

	countdownTimerLabelEntity->text->text = countdownTimerLabelText;
	countdownTimerLabelEntity->text->fontID = 2;
	countdownTimerLabelEntity->text->fontPath = fontPath2;

	// Text Color
	//countdownTimerLabelEntity->text->textColor = whiteColor;
    countdownTimerLabelEntity->text->fontID = FONT1_ID;

    int countdownTimerBox_width = 262;
    int countdownTimerBox_height = 0;//150;
    int countdownTimerBoxPosX = 150;//(320-box_width);///2;
	int countdownTimerBoxPosY = 20;//(240-box_height);///2; 

	Vec2 countdownTimerLabelTextScreenPos = {countdownTimerBoxPosX, countdownTimerBoxPosY};
	Vec2 countdownTimerLabelTextBounds = {countdownTimerBox_width, countdownTimerBox_height};
    countdownTimerLabelEntity->text->screenPos = countdownTimerLabelTextScreenPos;
	countdownTimerLabelEntity->text->textBounds = countdownTimerLabelTextBounds;
 // Create Player 1 card
    Vec2 playe1CountLabelPos = {20, 180};
    Vec2 playe1CountLabelSize = {320, 50};
    playersCountUIEntity = Entity_Factory_CreateUILabel(&_appData->ecs, playerCountCharBuff, FONT2_ID, fontPath2, whiteColor, playe1CountLabelPos, playe1CountLabelSize);

    // game over
    Vec2 gameOverTitlePos = {120, 100};
    Vec2 gameOverTitleSize = {320, 50};
    Vec2 gameOverSubTitlePos = {20, 140};
    Vec2 gameOverSubTitleSize = {320, 50};
    gameOverTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, gameOverTitleCharBuffer, FONT2_ID, fontPath2, whiteColor, gameOverTitlePos, gameOverTitleSize);
    gameOverSubTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, gameOverSubTitleLoseCharBuffer, FONT2_ID, fontPath2, whiteColor, gameOverSubTitlePos, gameOverSubTitleSize);
    // disable at the start
	gameOverTitleEntity->text->isShowing = FALSE;
    gameOverSubTitleEntity->text->isShowing = FALSE;

    // Create Main Menu
    Vec2 mainMenuTitlePos = {120, 100};
    Vec2 mainMenuTitleSize = {320, 50};
    Vec2 mainMenuSubTitlePos = {80, 140};
    Vec2 mainMenuSubTitleSize = {320, 50};
    mainMenuTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, mainMenuTitleCharBuffer, FONT2_ID, fontPath2, whiteColor, mainMenuTitlePos, mainMenuTitleSize);

    mainMenuSubTitleEntity = Entity_Factory_CreateUILabel(&_appData->ecs, mainMenuSubTitleCharBuffer, FONT2_ID, fontPath2, whiteColor, mainMenuSubTitlePos, mainMenuSubTitleSize);
    // disable at the start
	mainMenuTitleEntity->text->isShowing = FALSE;
    mainMenuSubTitleEntity->text->isShowing = FALSE;

    
}

void UI_Menu_Update(AppData* _appData){
    switch (_appData->gameplayData.gameState)
    {
        case GAME_STATE_MAIN_MENU:
            // this will decide how to render depending on game state
            // TODO
            UI_Menu_RenderMainMenu(_appData);
        break;

        case GAME_STATE_PLAYING:
        // TODO
            UI_Menu_RenderPlayingUI(_appData);
        break;

        case GAME_STATE_GAME_OVER_LOSE:
        // TODO
            UI_Menu_RenderGameOverScreen(_appData);
        break;

        case GAME_STATE_GAME_OVER_WIN:
            UI_Menu_RenderGameOverScreen(_appData);
        break;
    }
    
}

void UI_Menu_Shutdown(AF_ECS* _ecs){
    
}

// ===================== ===================== ===================== =====================


void RefreshUIEntity(AF_Entity* _entity, const char* _text){
    // countdown timer
    // buffer as wide as the screen
    // TODO, check buffer length is matched with _text
    char charBuff[320];
    sprintf(charBuff, "%s", _text);
    // TODO include component guards around this
    _entity->text->text = charBuff;
    // update the text
    _entity->text->isDirty = TRUE;
}



void UI_Menu_MainMenuSetShowing(BOOL _state){
    // Main Menu
        mainMenuTitleEntity->text->isShowing = _state;
        mainMenuSubTitleEntity->text->isShowing = _state;
}

void UI_Menu_GameOverUISetShowing(BOOL _state){
     // Game Over
        gameOverTitleEntity->text->isShowing = _state;
        gameOverSubTitleEntity->text->isShowing = _state;
        gameOverTitleEntity->text->isShowing = _state;
        gameOverSubTitleEntity->text->isShowing = _state;
}

void UI_Menu_PlayingSetState(BOOL _state){
    // Player Counts hid
        playersCountUIEntity->text->isShowing = _state;
        godEatCountLabelEntity->text->isShowing = _state;
        countdownTimerLabelEntity->text->isShowing = _state;
}
void UI_Menu_RenderMainMenu(AppData* _appData){
        UI_Menu_MainMenuSetShowing(TRUE);
        UI_Menu_GameOverUISetShowing(FALSE);
        UI_Menu_PlayingSetState(FALSE);

        // detect start button pressed
        if(_appData->input.keys[A_KEY].pressed == TRUE){
            GameplayData* gameplayData = &_appData->gameplayData;
            // gods count reset
            gameplayData->godEatCount = 0;
            // countdown Time
            gameplayData->countdownTimer = COUNT_DOWN_TIME;

            gameplayData->gameState = GAME_STATE_PLAYING;
            
            // reset the player score
            for(int i = 0; i < PLAYER_COUNT; ++i){
                _appData->gameplayData.playerEntities[i]->playerData->score = 0;
            }
        }
}


/*
Game_UpdatePlayerScoreText
Update the UI score elements
*/
void UI_Menu_RenderPlayingUI(AppData* _appData){
    UI_Menu_MainMenuSetShowing(FALSE);
    UI_Menu_GameOverUISetShowing(FALSE);
    UI_Menu_PlayingSetState(TRUE);

    GameplayData* gameplayData = &_appData->gameplayData; 
    sprintf(godsCountLabelText, "%i", gameplayData->godEatCount);
    godEatCountLabelEntity->text->text = godsCountLabelText;
    // our UI text rendering needs to be told an element is dirty so it will rebuild the text paragraph (for performance)
    godEatCountLabelEntity->text->isDirty = TRUE;
    
    // TODO, figure out how to reference the player entities
    sprintf(playerCountCharBuff, "%i                 %i                  %i                  %i", (int)gameplayData->playerEntities[0]->playerData->score, (int)gameplayData->playerEntities[1]->playerData->score, (int)gameplayData->playerEntities[2]->playerData->score, (int)gameplayData->playerEntities[3]->playerData->score);
    //sprintf(playerCountCharBuff, "%i                 %i                  %i                  %i", 123, 12, 123, 123);
    //debugf("playerScore %s \n", playerCountCharBuff);
    
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
}

void UI_Menu_RenderGameOverScreen(AppData* _appData ){

    UI_Menu_MainMenuSetShowing(FALSE);
    UI_Menu_PlayingSetState(FALSE);
    

    //gameOverTitleEntity->text->isShowing = TRUE;
    //gameOverSubTitleEntity->text->isShowing = TRUE;

    GameplayData* gameplayData = &_appData->gameplayData;
     if(gameplayData->gameState == GAME_STATE_GAME_OVER_WIN){
        gameOverSubTitleEntity->text->text = gameOverSubTitleWinCharBuffer;
     }else if(gameplayData->gameState == GAME_STATE_GAME_OVER_LOSE){
        gameOverSubTitleEntity->text->text = gameOverSubTitleLoseCharBuffer;
     }

     UI_Menu_GameOverUISetShowing(TRUE);

    // detect start button pressed
    if(_appData->input.keys[A_KEY].pressed == TRUE){
        gameplayData->gameState = GAME_STATE_MAIN_MENU;
    }

}