#include <libdragon.h>
#include "AF_UI.h"
#include "ECS/Components/AF_CText.h"
#include "AF_Vec2.h"
#include "AF_Util.h"


void AF_LoadFont(int _id, const char* _path, float _color[4]){
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

    /*
	debugf("InitTextRendering\n");

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
    int enabledFlag = AF_Component_GetEnabled(_sprite->enabled);
    // skip components that don't have the text component
    if(AF_Component_GetHas(enabledFlag) == 0){
        return;
    }

    // don't proceed if no path or null
    if(_sprite->spritePath == NULL){ 
        return;
    }

    if(AF_STRING_IS_EMPTY(_sprite->spritePath) == TRUE){
        // draw default background
        rdpq_set_mode_fill(RGBA32(_sprite->spriteColor[0], _sprite->spriteColor[1], _sprite->spriteColor[2], 0xFF));
        rdpq_fill_rectangle(_sprite->pos.x, _sprite->pos.y, _sprite->size.x, _sprite->size.y);
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
    rdpq_sprite_blit((sprite_t*)_sprite->spriteData, _sprite->pos.x, _sprite->pos.y, &(rdpq_blitparms_t){
            .s0 = _sprite->currentFrame * _sprite->size.x,//frame*ANIM_FRAME_W, //Extract correct sprite from sheet
            //Set sprite center to bottom-center
            .cx = .5f,//ANIM_FRAME_W/2,
            .cy = .5f,//ANIM_FRAME_H,
            .width = _sprite->size.x, //Extract correct width from sheet
            .flip_x = false//knights[i].flip
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