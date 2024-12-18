/*
===============================================================================
AF_LOG_H definitions

Definition of logging helper functions for the game
Calls vfprintf but adds some colour to text output
===============================================================================
*/

#ifndef AF_LOG_H
#define AF_LOG_H
#include <stdio.h>

#include <stdarg.h>

#ifdef DEBUG
#define LOG_TO_CONSOLE = 1
#else
#define LOG_TO_CONSOLE = 0
#endif

// ANSI escape codes for colors
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_CYAN    "\033[36m"


#ifdef __cplusplus
extern "C" {
#endif

// Log to console a generic message
void AF_Log(const char* _message,...);

// Log to console game specific message
void AF_Game_Log(const char* _message,...);

// Log to console warning message
void AF_Log_Warning(const char* _message,...);

// Log to console error message
void AF_Log_Error(const char* _message,...);

#ifdef __cplusplus
}
#endif


#endif // AF_LOG_H
