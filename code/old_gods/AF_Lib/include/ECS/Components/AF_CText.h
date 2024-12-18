/*
===============================================================================
AF_CTEXT_H definitions

Definition for the camera component struct
and helper functions
===============================================================================
*/
#include "AF_Vec2.h"
#include "AF_Vec4.h"
#include "AF_Lib_Define.h"

#ifndef AF_CTEXT_H
#define AF_CTEXT_H

typedef struct AF_CText {
    PACKED_CHAR enabled;	    // 1 byte
    BOOL isDirty;
    BOOL isShowing;
    uint8_t fontID;
    const char* fontPath;
    const char* text;
    Vec2 screenPos;
    Vec2 textBounds;
    uint8_t textColor[4];
    void* textData;
} AF_CText;

/*
====================
AF_CText_ZERO
Function used to create an empty text component
====================
*/
static inline AF_CText AF_CText_ZERO(void){
    AF_CText returnMesh = {
	.enabled = FALSE,
    .isDirty = FALSE,
    .isShowing = FALSE,
    .fontID = 0,
    .fontPath = NULL,
    .screenPos = {0,0},
    .text = NULL,
    .textColor = {0,0,0,1},
    .textData = NULL
    };
    return returnMesh;
}

/*
====================
AF_CText_ADD
Function used to Add the component
====================
*/
static inline AF_CText AF_CText_ADD(void){
    PACKED_CHAR component = 0;
    component = AF_Component_SetHas(component, TRUE);
    component = AF_Component_SetEnabled(component, TRUE);

    AF_CText returnMesh = {
	.enabled = component,
    .isDirty = TRUE,
    .isShowing = TRUE,
    .fontID = 0,
    .fontPath = NULL,
    .screenPos = {0,0},
    .text = NULL,
    .textColor = {0,0,0,1},
    .textData = NULL
    };
    return returnMesh;
}

#endif