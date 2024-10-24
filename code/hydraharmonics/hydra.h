#ifndef HYDRA_H
#define HYDRA_H

#include "hydraharmonics.h"

typedef struct hydra_s {
	float x,y;
	hydraharmonics_state_t state;
	uint8_t pos;
    sprite_t* head_sprite;
    sprite_t* neck_sprite;
    sprite_t* shell_sprite;
    sprite_t* eye_sprite;
} hydra_t;

extern hydra_t hydras[PLAYER_MAX];

void hydra_init (void);
void hydra_move (void);
void hydra_draw (void);
void hydra_clear (void);

#endif
