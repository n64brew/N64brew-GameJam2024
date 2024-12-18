/*
===============================================================================
AF_MATERIAL_H defninitions

Implementation of logging helper functions for the game
Calls vfprintf but adds some colour to text output
===============================================================================
*/

#ifndef AF_MATERIAL_H
#define AF_MATERIAL_H
typedef struct __attribute__((packed)) {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} AF_Color;

typedef struct{
	// TODO pack this
	uint32_t shaderID;
	uint32_t textureID;
	AF_Color color;
} AF_Material;

static inline AF_Material AF_Material_ZERO(void){
	AF_Color color = {0,0,0,255};
	AF_Material returnMaterial = {
		.shaderID = 0,
		.textureID = 0, 
		.color = color
	};

	return returnMaterial;
}

#endif

