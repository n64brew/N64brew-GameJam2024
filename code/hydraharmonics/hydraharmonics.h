#ifndef HYDRAHARMONICS_H
#define HYDRAHARMONICS_H

#include <libdragon.h>

#define HEAD_STATES_MAX 4
#define PLAYER_MAX MAXPLAYERS

#define HYDRA_HEAD_HEIGHT 32

#define PADDING_TOP 10
#define PADDING_LEFT 10

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

typedef struct hydra_s {
	float x,y;
	hydraharmonics_state_t state;
    sprite_t* head_sprite;
} hydra_t;

extern hydra_t hydras[PLAYER_MAX];

#endif
