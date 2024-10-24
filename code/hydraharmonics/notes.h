#ifndef NOTES_H
#define NOTES_H

#define NOTE_SPEED 2.5
#include "hydraharmonics.h"

typedef struct note_s {
	float x, y;
	hydraharmonics_state_t state;
    PlyNum player;
	sprite_t* sprite;
	struct note_s* next;
} note_t;

typedef struct note_ll_s {
	note_t* start;
	note_t* end;
} note_ll_t;

void notes_init(void);
void notes_add (void);
note_t* notes_get_first(void);
void notes_move (void);
void notes_draw (void);
void notes_destroy (note_t* dead_note);
void notes_clear (void);

#endif
