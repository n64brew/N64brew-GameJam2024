/*
===============================================================================
AF_INPUT_H definitions

Definition for the AF_Key and AF_Input structs
and helper functions
===============================================================================
*/
#ifndef AF_INPUT_H
#define AF_INPUT_H
#include "AF_Lib_Define.h"
#include "AF_Vec2.h"

#ifdef __cplusplus
extern "C" {
#endif
#define AF_INPUT_KEYS_MAPPED 10


#define PRESSED_MASK 0x80  // Pressed bit mask (8th bit)
#define KEYCODE_MASK 0x7F  // Keycode bit mask (lower 7 bits)
#define CONTROLLER_COUNT 4

/*
====================
AF_Key 
Key struct to be used with input system
====================
*/
typedef struct {
	char code;
	unsigned pressed;
    unsigned held;
} AF_Key;



/*
====================
AF_Input
Input struct to store the registered keys
====================
*/
typedef struct {
    // input buffer que
    // TODO: make array for size CONTROLLER_COUNT
    AF_Key keys[CONTROLLER_COUNT][AF_INPUT_KEYS_MAPPED];

    Vec2 controlSticks[CONTROLLER_COUNT];

    // Mouse
    //int mouseDown;
    //float mouseX;
    //float mouseY;

} AF_Input;

/*
====================
AF_Input_ZERO
Input struct Initialise to zero
====================
*/
static inline AF_Input AF_Input_ZERO(void){
    AF_Input input;
    for(int i = 0; i < AF_INPUT_KEYS_MAPPED; ++i){
        AF_Key key = {0, 0, 0};
        input.keys[0][i] = key; // Player 1
        input.keys[1][i] = key; // Player 2
        input.keys[2][i] = key; // Player 3
        input.keys[3][i] = key; // Player 4
    }
    for(int i = 0; i < CONTROLLER_COUNT; ++i){
        Vec2 controlStick = {0, 0};
        input.controlSticks[i] = controlStick;
    }
    return input;
}


/*
====================
AF_Input_Input
Init definition
====================
*/
void AF_Input_Init(void);

/*
====================
AF_Input_Update
Update definition
====================
*/
void AF_Input_Update(AF_Input* _input);


/*
====================
AF_Input_Shutdown
Shutdown definition
====================
*/
void AF_Input_Shutdown(void);


/*
====================
AF_Input_EncodeKey
Function to encode the key into a pressed state
====================
*/
static inline char AF_Input_EncodeKey(PACKED_CHAR _keyCode, BOOL _isPressed) {
    char returnedChar = _keyCode & KEYCODE_MASK; // Ensure only lower 7 bits are used for keycode

    if (_isPressed) {
        returnedChar |= PRESSED_MASK; // Set the 8th bit if the key is pressed
    }

    return returnedChar;
}



/*
====================
AF_Input_GetKeyCode
Function to decode the key value
====================
*/
static inline PACKED_CHAR AF_Input_GetKeyCode(PACKED_CHAR _encodedKey) {
    return _encodedKey & KEYCODE_MASK;  // Return the lower 7 bits as the keycode
}

/*
====================
AF_Input_IsKeyPressed
Function to check if the key is pressed
====================
*/
static inline BOOL AF_Input_IsKeyPressed(PACKED_CHAR _encodedKey) {
    return (_encodedKey & PRESSED_MASK) != 0;  // Check if the 8th bit is set
}
#ifdef __cplusplus
}
#endif
#endif // AF_INPUT_H
