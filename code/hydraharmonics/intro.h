#ifndef HYDRA_INTRO_H
#define HYDRA_INTRO_H

#include "hydraharmonics.h"

#define INSTRUCTIONS_PADDING_Y (PADDING_TOP*2-28)
#define INSTRUCTIONS_PADDING_X (PADDING_LEFT*2)

typedef enum {
	INTRO_INSTRUCTIONS,
	INTRO_INSTRUCTIONS_OUT,
	INTRO_CURTAINS_UP,
	INTRO_CURTAINS_DOWN,
} intro_stage_t;

void intro_init (void);
void intro_interact (void);
void intro_draw (void);
void intro_clear (void);

#endif
