#include <stdio.h>
#include <stdint.h>
#include "AF_Window.h"
#include "AF_Input.h"
#include "AF_Renderer.h"
#include "AF_Log.h"
#include "AF_Camera.h"
#include "AF_MeshData.h"

// Forward declare of c function
int main(void);

const char * windowTitle = "AF_Lib";
const uint16_t windowWidth = 1280;
const uint16_t windowHeight = 720;

int main(void) {
    AF_Log("MAIN: ENTRY POINT\n");
    
    // Meshdata
    AF_MeshData gameMeshData;

    // Camera
    AF_CCamera gameCamera;
    // Input
    AF_Input gameInput =  {0,0,0.0f, 0.0f};

   // Create the window
    AF_Window gameWindow = {0, &gameInput, windowTitle, windowWidth, windowHeight};
    AF_Lib_CreateWindow(&gameWindow);

    // Create the renderer
    AF_LIB_InitRenderer(&gameWindow);

    
    // update the window
    while(AF_Lib_UpdateWindow(&gameWindow)){

        // Display the renderer
        AF_LIB_DisplayRenderer(&gameWindow, &gameCamera, &gameMeshData);

        // Check for escape key pressed
        if(gameInput.lastKeyCodePressed == 256){
            AF_Log("GAME: ESC pressed \n");
            AF_Log_Warning("warning\n");
            AF_Log_Error("error test\n");
            break;
        }
    }

    // Destroy the renderer
    AF_LIB_DestroyRenderer();

    // Destroy the window
    AF_Lib_TerminateWindow(&gameWindow);
    return 0;
}

