/*
===============================================================================
AF_COMPONENT_H definitions

Definition for helper functions for components
===============================================================================
*/
#ifndef AF_COMPONENT_H
#define AF_COMPONENT_H

#include "AF_Lib_Define.h"
#ifdef __cplusplus
extern "C" {
#endif


/*
====================
AF_Component_SetHas
Function to set the has value
====================
*/
static inline PACKED_CHAR AF_Component_SetHas(flag_t _component, BOOL _hasFlag){
	if(_hasFlag == TRUE){ 
		// Set the bit if the key is pressed
		return _component |= FLAG_HAS;
	}
	else{
		// clear the bit is clear if has is FALSE
		return _component &= ~FLAG_HAS;
	}
}

/*
====================
AF_Component_SetEnabled
Function to set the enabled value
====================
*/
static inline PACKED_CHAR AF_Component_SetEnabled(PACKED_CHAR _component, BOOL _enabledFlag){
	if(_enabledFlag== TRUE){ 
		// Set the bit if the enabled frag is TRUE
		return _component |= FLAG_ENABLED;
	}
	else{
		// clear the bit is clear if has is FALSE
		return _component &= ~FLAG_ENABLED;
	}
}


/*
====================
AF_Component_GetEnabled
Function to decode the enabled value
====================
*/
static inline BOOL AF_Component_GetEnabled(flag_t _flags){

	return (_flags & FLAG_ENABLED) != 0; 
}

/*
====================
AF_Input_GetHas
Function to get the has value
====================
*/
static inline BOOL AF_Component_GetHas(flag_t _flags){

	return (_flags & FLAG_HAS) != 0;
}



#ifdef __cplusplus
}
#endif

#endif  // AF_COMPONENT_H
