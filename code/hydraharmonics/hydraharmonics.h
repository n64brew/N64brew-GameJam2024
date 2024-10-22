#ifndef HYDRAHARMONICS_H
#define HYDRAHARMONICS_H

#include <libdragon.h>

#define HEAD_STATES_MAX 4

typedef struct hydra_s {
	uint8_t state;
    sprite_t* head_sprites[HEAD_STATES_MAX];
} hydra_t;

typedef enum {
	STAGE_START,
    STAGE_GAME,
    STAGE_END,
} hydraharmonics_stage_t;

typedef enum {
	STATE_UP,
    STATE_MID,
    STATE_DOWN,
} hydraharmonics_state_t;

#endif
