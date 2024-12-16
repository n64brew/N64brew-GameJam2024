#ifndef NOTES_H
#define NOTES_H

#include "hydraharmonics.h"

#define NOTES_SPECIAL_TYPES 1
#define NOTES_PER_PLAYER 3
#define NOTES_PER_PLAYER_SPECIAL 5
#define NOTES_TOTAL_COUNT (PLAYER_MAX + NOTES_SPECIAL_TYPES)

#define NOTE_WIDTH 32
#define NOTE_HEIGHT 32

typedef enum {
	NOTES_GET_REMAINING_UNSPAWNED = 1,
	NOTES_GET_REMAINING_SPAWNED = 2,
	NOTES_GET_REMAINING_ALL = 3,
} notes_remaining_t;

typedef struct note_s {
	float x, y;
	int8_t anim_offset;
	float scale;
	float y_offset;
	PlyNum player;
	notes_types_t type;
	sprite_t* sprite;
	hydraharmonics_state_t state;
	rdpq_blitparms_t blitparms;
	struct note_s* next;
} note_t;

typedef struct notes_left_s {
	int8_t regular;
	int8_t special;
} notes_left_t;

typedef struct note_ll_s {
	note_t* start;
	note_t* end;
	notes_left_t notes_left[NOTES_TOTAL_COUNT];
} note_ll_t;

extern const float note_speeds[NOTE_SPEED_COUNT];
extern const float note_spawn[NOTE_SPEED_COUNT];
extern sprite_t* note_sprites[NOTES_TOTAL_COUNT];

void notes_init(void);
void notes_check_and_add (void);
note_t* notes_get_first(void);
void notes_move (void);
void notes_draw (void);
uint16_t notes_get_remaining (notes_remaining_t type);
void notes_destroy (note_t* dead_note);
void notes_destroy_all (void);
void notes_clear (void);

#endif
