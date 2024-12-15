/*
===============================================================================
AF_WINDOW_H

Definition of Window helper functions

===============================================================================
*/
#ifndef AF_WINDOW_H
#define AF_WINDOW_H
#include <stdint.h>
#include "AF_Lib_Define.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
====================
AF_Window
Window struct
====================
*/
typedef struct {
    void* window;           // Pointer to the window object (implementation specific)
    void* input;            // Pointer to the input handling object (implementation specific)
    const char* title;      // Title of the window
    uint16_t windowXPos;	// Window X position
    uint16_t windowYPos;	// Window Y position
    uint16_t windowWidth;   // Width of the window
    uint16_t windowHeight;  // Height of the window
    uint16_t frameBufferWidth; // Width of the framebuffer
    uint16_t frameBufferHeight; // Height of the framebuffer
} AF_Window;

/*
====================
AF_Lib_CreateWindow
Create the window and init all the window things
Platform/library dependent. Likely using glfw
====================
*/
void AF_Lib_CreateWindow(AF_Window* _window);

/*
====================
AF_Lib_UpdateWindow
Update the window
Platform/library dependent. Likely using glfw

====================
*/
BOOL AF_Lib_UpdateWindow(AF_Window* _window);

/*
====================
AF_Lib_Terminate
Close and clean up the window
Platform/library dependent. Likely using glfw

====================
*/
void AF_Lib_TerminateWindow(AF_Window* _window);


#ifdef __cplusplus
}
#endif

#endif  // AF_WINDOW_H
