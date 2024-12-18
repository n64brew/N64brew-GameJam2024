/*================
App to kick of the rest of the game and game functions
App will handle communication between the main entry point, update, input, rendering, sound, physics etc.
==================*/
#ifndef APP_H
#define APP_H
#include <libdragon.h>
#include <stdint.h>
#include "AF_Input.h"
#include "ECS/Entities/AF_ECS.h"
#include "AF_Time.h"
#include "GameplayData.h"
#include "AppData.h"

void AppData_Init(AppData* _appData, uint16_t _windowWidth, uint16_t _windowHeight);
void App_Init(AppData* _appData);
void App_Update(AppData* _appData);
void App_Render_Update(AppData* _appData);
void App_Shutdown(AppData* _appData);

/*================
PrintAppDataSize
Until function to print out the amount of memory the app data takes up
==================*/
static inline void PrintAppDataSize(AppData* _appData){
    size_t windowSize = sizeof(_appData->windowWidth) + sizeof(_appData->windowHeight);
    size_t gameTimeSize = sizeof(_appData->gameTime);
    size_t inputSize = sizeof(_appData->input);
    size_t gameplayDataSize = sizeof(_appData->gameplayData);
    size_t ecsSize = sizeof(_appData->ecs);
    size_t totalSize = sizeof(*_appData);

    // Conversion factors to KB and MB
    double windowSizeKB = windowSize / 1024.0;
    double gameTimeSizeKB = gameTimeSize / 1024.0;
    double inputSizeKB = inputSize / 1024.0;
    double gameplayDataSizeKB = gameplayDataSize / 1024.0;
    double ecsSizeKB = ecsSize / 1024.0;
    double totalSizeKB = totalSize / 1024.0;

    double totalSizeMB = totalSize / (1024.0 * 1024.0);

    // Print sizes in bytes, KB, and MB for total
    debugf("Size of window dimensions: %lu bytes (%.3f KB)\n", (unsigned long)windowSize, windowSizeKB);
    debugf("Size of gameTime: %lu bytes (%.3f KB)\n", (unsigned long)gameTimeSize, gameTimeSizeKB);
    debugf("Size of input: %lu bytes (%.3f KB)\n", (unsigned long)inputSize, inputSizeKB);
    debugf("Size of gameplayData: %lu bytes (%.3f KB)\n", (unsigned long)gameplayDataSize, gameplayDataSizeKB);
    debugf("Size of ecs: %lu bytes (%.3f KB)\n", (unsigned long)ecsSize, ecsSizeKB);
    debugf("Total size of AppData: %lu bytes (%.3f KB, %.6f MB)\n", (unsigned long)totalSize, totalSizeKB, totalSizeMB);
}
#endif
