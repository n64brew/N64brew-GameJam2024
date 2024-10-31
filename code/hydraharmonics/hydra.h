#ifndef HYDRA_H
#define HYDRA_H

#include "hydraharmonics.h"

typedef enum {
	HYDRA_AI_NONE,
	HYDRA_AI_RANDOM,
	HYDRA_AI_SMART,
	HYDRA_AI_COUNT,
} hydraharmonics_ai_t;

typedef enum {
	HYDRA_ANIMATION_NONE,
	HYDRA_ANIMATION_OPEN,
	HYDRA_ANIMATION_CLOSE,
	HYDRA_ANIMATION_CHEW,
	HYDRA_ANIMATION_SLEEP,
	HYDRA_ANIMATION_SLEEP_2,
	HYDRA_ANIMATION_STUN,
	HYDRA_ANIMATION_COUNT,
} hydraharmonics_animations_t;

typedef struct hydra_s {
	float x,y, hat_y;
	hydraharmonics_state_t state;
    hydraharmonics_ai_t ai;
	hydraharmonics_animations_t animation;
	uint8_t frame;
	uint8_t pos;
    sprite_t* head_sprite;
    sprite_t* neck_sprite;
    sprite_t* shell_sprite;
    surface_t neck_surf;
} hydra_t;

extern hydra_t hydras[PLAYER_MAX];

void hydra_init (void);
void hydra_adjust_hats (void);
void hydra_move (void);
void hydra_ai (uint8_t hydra);
void hydra_animate (PlyNum p, hydraharmonics_animations_t a);
void hydra_shell_bounce (void);
void hydra_draw (void);
void hydra_clear (void);

#endif
