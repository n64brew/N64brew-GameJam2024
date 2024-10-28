#ifndef NOTES_H
#define NOTES_H

#include "hydraharmonics.h"

#define NOTES_PER_PLAYER 5
#define NOTE_SPEED 2.5

typedef struct note_s {
	float x, y;
	int8_t anim_offset;
	float scale;
	float y_offset;
	hydraharmonics_state_t state;
	PlyNum player;
	sprite_t* sprite;
	struct note_s* next;
	rdpq_blitparms_t blitparms;
} note_t;

typedef struct note_ll_s {
	note_t* start;
	note_t* end;
	uint8_t notes_left[PLAYER_MAX];
} note_ll_t;

typedef enum {
	NOTES_GET_REMAINING_UNSPAWNED = 1,
	NOTES_GET_REMAINING_SPAWNED = 2,
	NOTES_GET_REMAINING_ALL = 3,
} notes_remaining_t;

void notes_init(void);
void notes_add (void);
note_t* notes_get_first(void);
void notes_move (void);
void notes_draw (void);
uint16_t notes_get_remaining (notes_remaining_t type);
void notes_destroy (note_t* dead_note);
void notes_destroy_all (void);
void notes_clear (void);

#endif
