/*
===============================================================================
AF_RECT_H

Rect struct used in UI and 2D physics calculations
===============================================================================
*/
#ifndef AF_RECT_H
#define AF_RECT_H

#ifdef __cplusplus
extern "C" {    
#endif


/*
====================
AF_Rect
Simple rect data strcuture
====================
*/
typedef struct {
    AF_FLOAT x, y, w, h;
} AF_Rect;
#ifdef __cplusplus
}
#endif

#endif //AF_RECT_H
