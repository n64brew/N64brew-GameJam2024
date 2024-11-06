#include "App.h"
#include <libdragon.h>
#include "AF_Time.h"
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240
int main(void){
    

    AF_Time gameTime;

    // Initialise the app
    App_Init(WINDOW_WIDTH, WINDOW_HEIGHT, &gameTime);// Initialize lastTime before the loop

    gameTime.lastTime = timer_ticks();
    // Main game loop
    while(1) {
        gameTime.currentTime = timer_ticks();
        gameTime.timeSinceLastFrame = (float)(gameTime.currentTime - gameTime.lastTime)/ TICKS_PER_SECOND;
        gameTime.lastTime = gameTime.currentTime;
        // render stuff as fast as possible, interdependent from other code
        App_Render_Update(&gameTime);
    }

    // Do Shutdown things
    App_Shutdown();
    return 1;
}
