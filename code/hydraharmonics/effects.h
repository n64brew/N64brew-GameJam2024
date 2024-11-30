#ifndef HYDRA_EFFECTS_H
#define HYDRA_EFFECTS_H

#include "hydraharmonics.h"

#define HYDRA_CHEW_EFFECT_OFFSET_X 44
#define HYDRA_CHEW_EFFECT_OFFSET_Y 10
#define HYDRA_STUN_EFFECT_OFFSET_Y 5

typedef enum {
	EFFECT_FLOWER,
	EFFECT_NOTE_BIG,
	EFFECT_NOTE_SMALL,
	EFFECT_SHOCK,
	EFFECT_SPARKLE,
	EFFECT_COUNT,
} effect_types_t;

typedef struct effect_s {
	float x, y;
	float accel;
	int16_t timer;
	sprite_t* sprite;
	PlyNum player;
	effect_types_t type;
	rdpq_blitparms_t blitparms;
	struct effect_s* next;
} effect_t;

typedef struct effect_ll_s {
	effect_t* start;
	effect_t* end;
} effect_ll_t;

void effects_init (void);
void effects_draw (void);
void effects_add (PlyNum player, effect_types_t type, float x, float y);
void effects_animate (void);
void effects_destroy (effect_t* dead_effect);
void effects_destroy_all (void);
void effects_clear (void);

#endif
