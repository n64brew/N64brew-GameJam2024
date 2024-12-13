/*================
Implementation file for AF_Input
Specific to n64
Input contains everything needed to collect input from the system from Libdragon and feed it back into the game via an AF_Input struct
=================*/
#include <libdragon.h>
#include "AF_Input.h"
#include "ECS/Entities/AF_ECS.h"

// Key mappings for n64 controller to joypad_button struct, polled from libdragon
#define A_KEY 0			// A Button		
#define B_KEY 1			// B Button
#define START_KEY 2		// Start Button

/* ================
AF_Input_Init
Init the input
================ */
void AF_Input_Init(){
	debugf("Input Init\n");
	// Init Libdragon input
	joypad_init();
	
}

/* ================
AF_Input_Update
Implementation of AF_Input_Update
Using Libdragon joypad inputs, figure out what the input means for this game, storing the results in the input struct
================ */
void AF_Input_Update(AF_Input* _input){
	if(_input == NULL){
		debugf("Input: Input_Update: passed in a null reference to input\n");
		return;
	}
	joypad_poll();

		// Player 1
        joypad_inputs_t inputs = joypad_get_inputs(JOYPAD_PORT_1);
		joypad_buttons_t pressed1 = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        joypad_buttons_t held1 = joypad_get_buttons_held(JOYPAD_PORT_1);

		// Player 2
        joypad_inputs_t inputs2 = joypad_get_inputs(JOYPAD_PORT_2);
		joypad_buttons_t pressed2 = joypad_get_buttons_pressed(JOYPAD_PORT_2);
        joypad_buttons_t held2 = joypad_get_buttons_held(JOYPAD_PORT_2);


		// Player 3
        joypad_inputs_t inputs3 = joypad_get_inputs(JOYPAD_PORT_3);
		joypad_buttons_t pressed3 = joypad_get_buttons_pressed(JOYPAD_PORT_3);
        joypad_buttons_t held3 = joypad_get_buttons_held(JOYPAD_PORT_3);

		// Player 4
        joypad_inputs_t inputs4 = joypad_get_inputs(JOYPAD_PORT_4);
		joypad_buttons_t pressed4 = joypad_get_buttons_pressed(JOYPAD_PORT_4);
        joypad_buttons_t held4 = joypad_get_buttons_held(JOYPAD_PORT_4);
		
		// flush the keys
		for(int i = 0; i < AF_INPUT_KEYS_MAPPED; ++i){
			_input->keys[0][i].pressed = FALSE;
			_input->keys[1][i].pressed = FALSE;
			_input->keys[2][i].pressed = FALSE;
			_input->keys[3][i].pressed = FALSE;
		}

		// ==== HELD ====
		// Held A key
        _input->keys[0][A_KEY].held = held1.a;
		_input->keys[1][A_KEY].held = held2.a;
		_input->keys[2][A_KEY].held = held3.a;
		_input->keys[3][A_KEY].held = held4.a;
        
		// Held B key
        _input->keys[0][B_KEY].held = held1.b;
		_input->keys[1][B_KEY].held = held2.b;
		_input->keys[2][B_KEY].held = held3.b;
		_input->keys[3][B_KEY].held = held4.b;
	
		// ==== PRESSED ====
		// Pressed A key
        _input->keys[0][A_KEY].pressed = pressed1.a;
		_input->keys[1][A_KEY].pressed = pressed2.a;
		_input->keys[2][A_KEY].pressed = pressed3.a;
		_input->keys[3][A_KEY].pressed = pressed4.a;
		//debugf("Pressed: %i, %i, %i, %i \n", _input->keys[0][A_KEY].pressed, _input->keys[1][A_KEY].pressed, _input->keys[2][A_KEY].pressed, _input->keys[3][A_KEY].pressed);
        
		// Pressed B key
        _input->keys[0][B_KEY].pressed = pressed1.b;
		_input->keys[1][B_KEY].pressed = pressed2.b;
		_input->keys[2][B_KEY].pressed = pressed3.b;
		_input->keys[3][B_KEY].pressed = pressed4.b;

		// Pressed Start key
        _input->keys[0][START_KEY].pressed = pressed1.start;
		_input->keys[1][START_KEY].pressed = pressed2.start;
		_input->keys[2][START_KEY].pressed = pressed3.start;
		_input->keys[3][START_KEY].pressed = pressed4.start;
	


		// Player 1
        float y = inputs.stick_y / 128.0f;
        float x = inputs.stick_x / 128.0f;

		// Player 2
		float x2 = inputs2.stick_x / 128.0f;
		float y2 = inputs2.stick_y / 128.0f;

		// Player 2
		float x3 = inputs3.stick_x / 128.0f;
		float y3 = inputs3.stick_y / 128.0f;

		// Player 2
		float x4 = inputs4.stick_x / 128.0f;
		float y4 = inputs4.stick_y / 128.0f;

		//player 1
		_input->controlSticks[0].x = x;
		_input->controlSticks[0].y = y;	

		// Player 2
		_input->controlSticks[1].x = x2;
		_input->controlSticks[1].y= y2;

		// Player 3
		_input->controlSticks[2].x = x3;
		_input->controlSticks[2].y = y3;

		// Player 4
		_input->controlSticks[3].x = x4;
		_input->controlSticks[3].y = y4;


		
}

/* ================
AF_Input_Shutdown
Implementation of shutdown
Does nothing for now
================ */
void AF_Input_Shutdown(void){
	debugf("Input Shutdown\n");
}
