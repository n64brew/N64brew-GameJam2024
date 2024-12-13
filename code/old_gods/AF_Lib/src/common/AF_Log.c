/*
===============================================================================
AF_LOG Implementation

Logging helper functions for the game
Calls vfprintf but adds some colour to text output
===============================================================================
*/
#include "AF_Log.h"

/*
====================
AF_Log_Error
Log standard error to console
====================
*/
void AF_Log_Error(const char* _message,...) {
    if (_message == NULL) {
        fprintf(stderr, COLOR_RED "Error: Invalid format string\n" COLOR_RESET);
        return;
    }

    fprintf(stderr, COLOR_RED "Error: " COLOR_RESET);  // Set color to red for errors
    //fprintf("%s",_message);
    
    va_list args;
    va_start(args, _message);
    
    vfprintf(stderr, _message, args);
    va_end(args);
    
}

/*
====================
AF_Log
Normal log to console
====================
*/
void AF_Log(const char* _message,...){
    printf(COLOR_GREEN);  // Set color to cyan for regular logs
    va_list args;
    va_start(args, _message);
    vfprintf(stdout, _message, args);
    //AFSaveLog(_message, "LOG: ", args);
    va_end(args);
}


/*
====================
AF_Game_Log
Game specific log to console
====================
*/
void AF_Game_Log(const char* _message,...){
    printf(COLOR_CYAN);  // Set color to cyan for regular logs
    va_list args;
    va_start(args, _message);
    vfprintf(stdout, _message, args);
    //AFSaveLog(_message, "LOG: ", args);
    va_end(args);
}


/*
====================
AF_Log_Warning
Log warning to console
====================
*/
void AF_Log_Warning(const char* _message,...){
    printf(COLOR_YELLOW "Warning: ");  // Set color to yellow for warnings
    va_list args;
    va_start(args, _message);
    vfprintf(stdout, _message, args);
    va_end(args);
}
