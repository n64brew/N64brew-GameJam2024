#include <libdragon.h>
#include "AF_UI.h"
#include "ECS/Components/AF_CText.h"
#include "AF_Vec2.h"
#include "AF_Util.h"


void AF_LoadFont(int _id, const char* _path, uint8_t _color[4]){
    rdpq_font_t *fnt1 = rdpq_font_load(_path);
        rdpq_font_style(fnt1, 0, &(rdpq_fontstyle_t){
            .color = RGBA32(_color[0],_color[1], _color[2], 0xFF),//text->textColor[0],text->textColor[1], text->textColor[2], text->textColor[3]), //0xED, 0xAE, 0x49, 0xFF),
        });
       
        rdpq_text_register_font(_id, fnt1);
}


/*
====================
AF_UI_INIT
Init for UI components like text
====================
*/
void AF_UI_Init(AF_ECS* _ecs){

    if(_ecs == NULL){
		debugf("Renderer: Renderer_Init has null ecs referenced passed in \n");
		return;
	} 

    
	debugf("InitTextRendering\n");
    /*
    for(int i = 0; i < _ecs->entitiesCount; i++){
        // find components that are text components and initialise them

        // Setup Text
        // Register the font
        AF_CText* text = &_ecs->texts[i];

        int hasFlag = AF_Component_GetHas(text->enabled);
        int enabledFlag = AF_Component_GetEnabled(text->enabled);
        // skip components that don't have the text component
        if(AF_Component_GetHas(enabledFlag) == 0){
            continue;
        }

        // don't proceed if no path or null
        if(text->fontPath == NULL || AF_STRING_IS_EMPTY(text->text) == TRUE){
            continue;
        }
        debugf("AF_UI_INIT: loading font %i: %s enabled: %d has: %d \n",text->fontID, text->fontPath, hasFlag, enabledFlag);
        // if font already is loaded, don't load it again

        rdpq_font_t *fnt1 = rdpq_font_load(text->fontPath);
        rdpq_font_style(fnt1, 0, &(rdpq_fontstyle_t){
            .color = RGBA32(text->textColor[0],text->textColor[1], text->textColor[2], 0xFF),//text->textColor[0],text->textColor[1], text->textColor[2], text->textColor[3]), //0xED, 0xAE, 0x49, 0xFF),
        });
       
        rdpq_text_register_font(text->fontID, fnt1);

        // Setup sprite
        
    }
    */
    
}


void AF_UI_Update(AF_ECS* _ecs, AF_Time* _time){
    for(int i = 0; i < _ecs->entitiesCount; ++i){
        AF_CSprite* sprite = &_ecs->sprites[i];

       // render spirte first then text
        AF_UI_RendererSprite_Update(sprite, _time);
       
    }

    for(int i = 0; i < _ecs->entitiesCount; ++i){
        AF_CText* text = &_ecs->texts[i];

       // render spirte first then text
       AF_UI_RendererText_Update(text);
    }
     
}

/*
====================
AF_UI_PLAYANIMATION_UPDATE
Allow an animation to start playing
====================
*/
void AF_UI_RendererSprite_Play(AF_CSprite* _sprite, BOOL _isLooping){
    _sprite->isPlaying = TRUE;
    _sprite->loop = _isLooping;
    // reset the frame
    _sprite->currentFrame = 0;
}


/*
====================
AF_UI_RENDERERSPrITE_UPDATE
Render all UI elements like text
====================
*/
void AF_UI_RendererSprite_Update(AF_CSprite* _sprite, AF_Time* _time){
    // Find components that are text components
    // skip components that don't have the text component
    //int hasFlag = AF_Component_GetHas(_sprite->enabled);
    BOOL enabledFlag = AF_Component_GetEnabled(_sprite->enabled);
    BOOL hasFlag = AF_Component_GetHas(_sprite->enabled);
    // skip components that don't have the text component
    //if(AF_Component_GetHas(enabledFlag) == 0){
    //    return;
    //}
    if(enabledFlag != TRUE || hasFlag != TRUE){
        return;
    }
    // don't proceed if no path or null
    if(_sprite->spritePath == NULL){ 
        return;
    }

    if(AF_STRING_IS_EMPTY(_sprite->spritePath) == TRUE){
        // draw default background
        rdpq_set_mode_fill(RGBA32(_sprite->spriteColor[0], _sprite->spriteColor[1], _sprite->spriteColor[2], 0xFF));
        rdpq_fill_rectangle(_sprite->spritePos.x, _sprite->spritePos.y, _sprite->spriteSize.x, _sprite->spriteSize.y);
    }

    // don't progress if we don't have sprite data
    if(_sprite->spriteData == NULL){
        return;
    }
    //Set render mode to the standard render mode
    rdpq_set_mode_standard();
    rdpq_mode_alphacompare(1); // colorkey (draw pixel with alpha >= 1)

    // Progress the sprite frame
    //debugf("time: %f\n", _time->currentTime);
    
    if (_sprite->isPlaying == TRUE && _time->currentTime >= _sprite->nextFrameTime) {
        // Check if it's the last frame in the animation
        if (_sprite->currentFrame == _sprite->animationFrames - 1) {
            if (_sprite->loop == TRUE) {
                // Loop to the first frame
                _sprite->currentFrame = 0;
                _sprite->nextFrameTime = _time->currentTime + _sprite->animationSpeed;
            } else {
                // Reset back to the start but keep the current nextFrameTime
                _sprite->currentFrame = 0;
                // tell sprite to stop playing
                _sprite->isPlaying = FALSE;
            }
        } else {
            // Advance to the next frame
            _sprite->currentFrame++;
            _sprite->nextFrameTime = _time->currentTime + _sprite->animationSpeed;
        }
    }

    
    //int horizontalFrame = 0;
    //Draw knight sprite
    rdpq_sprite_blit((sprite_t*)_sprite->spriteData, _sprite->spritePos.x, _sprite->spritePos.y, &(rdpq_blitparms_t){
            .s0 = 0,//_sprite->currentFrame * _sprite->spriteSheetSize.x,  //< Source sub-rect top-left X coordinate
            .t0 = 0,//_sprite->currentFrame * _sprite->spriteSheetSize.y,                                        ///< Source sub-rect top-left Y coordinate
            .width = _sprite->spriteSize.x,            ///< Source sub-rect width. If 0, the width of the surface is used
            //Set sprite center to bottom-center
            .height = _sprite->spriteSize.y,           ///< Source sub-rect height. If 0, the height of the surface is used
            .flip_x = _sprite->flipX,                       ///< Flip horizontally. If true, the source sub-rect is treated as horizontally flipped (so flipping is performed before all other transformations)
            .flip_y = _sprite->flipY,                       ///< Flip vertically. If true, the source sub-rect is treated as vertically flipped (so flipping is performed before all other transformations)
            .cx = 0.5f,                                      ///< Transformation center (aka "hotspot") X coordinate, relative to (s0, t0). Used for all transformations
            .cy = 0.5f,                                      ///< Transformation center (aka "hotspot") X coordinate, relative to (s0, t0). Used for all transformations
            .scale_x = _sprite->spriteScale.x,                     ///< Horizontal scale factor to apply to the surface. If 0, no scaling is performed (the same as 1.0f). If negative, horizontal flipping is applied
            .scale_y = _sprite->spriteScale.y,                     ///< Vertical scale factor to apply to the surface. If 0, no scaling is performed (the same as 1.0f). If negative, vertical flipping is applied
            .theta = _sprite->spriteRotation,                        ///< Rotation angle in radians
            .filtering = _sprite->filtering,                             ///< True if texture filtering is enabled (activates workaround for filtering artifacts when splitting textures in chunks)
            //.nx = 1,                                        ///< Texture horizontal repeat count. If 0, no repetition is performed (the same as 1)
            //.ny = 1                                         ///< Texture vertical repeat count. If 0, no repetition is performed (the same as 1)
        });

}

/*
====================
AF_UI_RENDERER_TEXT_UPDATE
Render all UI elements like text
====================
*/
void AF_UI_RendererText_Update(AF_CText* _text){
    
    // Find components that are text components
    // skip components that don't have the text component
    //int hasFlag = AF_Component_GetHas(_text->enabled);
    int enabledFlag = AF_Component_GetEnabled(_text->enabled);
    // skip components that don't have the text component
    if(AF_Component_GetHas(enabledFlag) == 0){
        return;
    }

    if(_text->isShowing == FALSE){
        return;
    }

    // don't proceed if no path or null
    if(_text->fontPath == NULL || AF_STRING_IS_EMPTY(_text->text) == TRUE){
        return;
    }

    
    // if text needs updating, rebuild, otherwise skip
    if(_text->isDirty == TRUE){
        int nbytes = strlen(_text->text);
        rdpq_paragraph_free((rdpq_paragraph_t*)_text->textData);
        rdpq_paragraph_t* par = rdpq_paragraph_build(&(rdpq_textparms_t){
            // .line_spacing = -3,
            .align = ALIGN_LEFT,
            .valign = VALIGN_CENTER,
            .width = _text->textBounds.x,
            .height = _text->textBounds.y,
            .wrap = WRAP_WORD,
        }, _text->fontID, _text->text, &nbytes);
        _text->textData = (void*) par;
        _text->isDirty = FALSE;
    }

    
    if(_text->textData == NULL){
        return;
    }
    rdpq_paragraph_render((rdpq_paragraph_t*)_text->textData, _text->screenPos.x, _text->screenPos.y);
    

    //rdpq_paragraph_render(par, _text->screenPos.x, _text->screenPos.y);
    //rdpq_paragraph_free(par);

}



/*
====================
AF_UI_RENDERER_FINISH
Final render pass
====================
*/
void AF_UI_Renderer_Finish(void){

}

/*
====================
AF_UI_RENDERER_SHUTDOWN
Do shutdown things
====================
*/
void AF_UI_Renderer_Shutdown(void){

}

/*

void Game_UpdatePlayerScoreText();


void RenderGameOverScreen(AF_Input* _input);
void UpdateText(AF_ECS* _ecs);
void RenderMainMenu(AF_Input* _input, AF_Time* _time);

void UpdateText(AF_ECS* _ecs){
    // God eat count
    

    // countdown timer
    sprintf(countdownTimerLabelText, "TIME %i", (int)countdownTimer);
    countdownTimerLabelEntity->text->text = countdownTimerLabelText;
    // update the text
    countdownTimerLabelEntity->text->isDirty = TRUE;

    
    //playersCountUIEntity->text->text = playerCountCharBuff;
    // Update player counters
    // player 1
    
}
*/

/*
Game_UpdatePlayerScoreText
Update the UI score elements
*/

/*
void Game_UpdatePlayerScoreText(){
    sprintf(godsCountLabelText, "%i", godEatCount);
    godEatCountLabelEntity->text->text = godsCountLabelText;
    // our UI text rendering needs to be told an element is dirty so it will rebuild the text paragraph (for performance)
    godEatCountLabelEntity->text->isDirty = TRUE;
    sprintf(playerCountCharBuff, "%i                 %i                  %i                  %i", (int)player1Entity->playerData->score, (int)player2Entity->playerData->score, (int)player3Entity->playerData->score, (int)player4Entity->playerData->score);
    debugf("playerScore %s \n", playerCountCharBuff);
    
    playersCountUIEntity->text->text = playerCountCharBuff;
    playersCountUIEntity->text->isDirty = TRUE;
   
}

void RenderMainMenu(AF_Input* _input, AF_Time* _time){
    switch (gameState)
    {
    case GAME_STATE_MAIN_MENU:
        // Main Menu
        mainMenuTitleEntity->text->isShowing = TRUE;
        mainMenuSubTitleEntity->text->isShowing = TRUE;

        // Game Over
        gameOverTitleEntity->text->isShowing = FALSE;
        gameOverSubTitleEntity->text->isShowing = FALSE;
        gameOverTitleEntity->text->isShowing = FALSE;
        gameOverSubTitleEntity->text->isShowing = FALSE;

        // Player Counts hid
        playersCountUIEntity->text->isShowing = FALSE;

        // reset the visible text
        //Game_UpdatePlayerScoreText();
        // Header bar
        godEatCountLabelEntity->text->isShowing = FALSE;
        // gods count reset
        godEatCount = 0;

        countdownTimerLabelEntity->text->isShowing = FALSE;

        // countdown Time
        countdownTimer = COUNT_DOWN_TIME;

        // detect start button pressed
        if(_input->keys[2].pressed == TRUE){

            
            gameState = GAME_STATE_PLAYING;
            player1Entity->playerData->score = 0;
            player2Entity->playerData->score = 0;
            player3Entity->playerData->score = 0;
            player4Entity->playerData->score = 0;
            playersCountUIEntity->playerData->score = 0;
            Game_UpdatePlayerScoreText();
        }
        
        
    break;

    case GAME_STATE_PLAYING:
        // Update countdown timer
        countdownTimer -= _time->timeSinceLastFrame;
        if(countdownTimer <= 0){
            gameState = GAME_STATE_GAME_OVER_LOSE;
            countdownTimer = COUNT_DOWN_TIME;
        }

        if(godEatCount == GODS_EAT_COUNT){
            gameState = GAME_STATE_GAME_OVER_WIN;
            countdownTimer = COUNT_DOWN_TIME;
        }

    // Player Counts
        playersCountUIEntity->text->isShowing = TRUE;

        // MAin MEnu
        mainMenuTitleEntity->text->isShowing = FALSE;
        mainMenuSubTitleEntity->text->isShowing = FALSE;

        // Header bar
        godEatCountLabelEntity->text->isShowing = TRUE;
        countdownTimerLabelEntity->text->isShowing= TRUE;
    break;

    default:
        break;
    }
}
void RenderGameOverScreen(AF_Input* _input){
    switch (gameState)
    {
    case GAME_STATE_PLAYING:
        
    break;
    case GAME_STATE_GAME_OVER_WIN:
    
        // render game over win title
        // render game over sub title showing what player won
        gameOverSubTitleEntity->text->text = gameOverSubTitleWinCharBuffer;
        gameOverTitleEntity->text->isShowing = TRUE;
        gameOverSubTitleEntity->text->isShowing = TRUE;

        // detect start button pressed
        if(_input->keys[2].pressed == TRUE){
            gameState = GAME_STATE_MAIN_MENU;
        }
        break;

    case GAME_STATE_GAME_OVER_LOSE:
   
        // render Game over lose title
        // Render Game Over lose sub title
        gameOverSubTitleEntity->text->text = gameOverSubTitleLoseCharBuffer;
        gameOverTitleEntity->text->isShowing = TRUE;
        gameOverSubTitleEntity->text->isShowing = TRUE;

        // detect start button pressed
        if(_input->keys[2].pressed == TRUE){
            gameState = GAME_STATE_MAIN_MENU;
        }
        break;
    
    default:
        break;
    }
    
    // if player presses start button. Restart the game
}*/
