#include <libdragon.h>
#include "AF_UI.h"
#include "ECS/Components/AF_CText.h"
#include "AF_Vec2.h"
#include "AF_Util.h"


/*
====================
AF_LoadFont
Load funt util function
====================
*/
void* AF_LoadFont(int _id, const char* _path, uint8_t _color[4]){
    rdpq_font_t *fnt1 = rdpq_font_load(_path);
    rdpq_font_style(fnt1, 0, &(rdpq_fontstyle_t){
        .color = RGBA32(_color[0],_color[1], _color[2], 0xFF),//text->textColor[0],text->textColor[1], text->textColor[2], text->textColor[3]), //0xED, 0xAE, 0x49, 0xFF),
    });
    
    rdpq_text_register_font(_id, fnt1);

    return (void*)fnt1;
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
    
	//debugf("InitTextRendering\n");
    
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
        // Free the previous text data safely
        if (_text->textData != NULL) {
            rdpq_paragraph_free((rdpq_paragraph_t*)_text->textData);
            _text->textData = NULL; // Prevent double-free
        }
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
    rdpq_paragraph_t* textData = (rdpq_paragraph_t*)_text->textData;
    rdpq_paragraph_render(textData, _text->screenPos.x, _text->screenPos.y);
    

    //rdpq_paragraph_render(par, _text->screenPos.x, _text->screenPos.y);
    
    
    
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
void AF_UI_Renderer_Shutdown(AF_ECS* _ecs){
    //debugf("AF_UI_Renderer_Shutdown: \n");

    // Try to free text data that may remain
    for(int i = 0; _ecs->entitiesCount; ++i){
        //debugf("AF_UI_Renderer_Shutdown: Starting %i \n", i);
        AF_CText* text = _ecs->entities[i].text;
        if(text == NULL){
                //debugf("AF_UI_Renderer_Shutdown: text is null early exit %i \n", i);
                return;
            }
        BOOL hasText = AF_Component_GetHas(text->enabled);
        if(hasText == TRUE){
            //debugf("AF_UI_Renderer_Shutdown: has text %i \n", i);
            rdpq_paragraph_t* textData = (rdpq_paragraph_t*)text->textData;
            // try to free the text data memory
            if(textData != NULL){
                //debugf("AF_UI_Renderer_Shutdown: try to free %i \n", i);
                rdpq_paragraph_free(textData);
                textData = NULL;
                //debugf("AF_UI_Renderer_Shutdown: free success %i \n", i);
            }
        }
        //debugf("AF_UI_Renderer_Shutdown: Finished %i \n", i);
    }   
    //debugf("AF_UI_Renderer_Shutdown: Finished\n");
}
