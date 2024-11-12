#include "notes.h"

#define NOTE_SCALE_MULTIPLIER 1.0
#define NOTE_Y_OFFSET_PERIOD 32
#define NOTE_Y_OFFSET_AMPLITUDE 4
#define NOTE_THETA_PERIOD 16
#define NOTE_THETA_AMPLITUDE 0.1
#define NOTE_SCALE_PERIOD 0.1
#define NOTE_SCALE_AMPLITUDE 0.05
#define NOTE_ANIMATION_BOUNCE 0x1
#define NOTE_ANIMATION_ROTATE 0x2
#define NOTE_ANIMATION_PULSE 0x4
#define NOTE_WIDTH 32
#define NOTE_HEIGHT 32
#define NOTE_ANIMATION_SPEED 8
#define NOTE_ANIMATION_OFFSET_Y NOTE_Y_OFFSET_AMPLITUDE * 0.35

// Global vars
static note_ll_t notes;
static sprite_t* note_sprites[NOTES_TOTAL_COUNT];

void notes_init(void) {
	char temptext[64];
	for (uint8_t i=0; i<NOTES_TOTAL_COUNT; i++) {
		// Load the note sprites
		sprintf(temptext, "rom:/hydraharmonics/note-%i.ci4.sprite", i);
		note_sprites[i] = sprite_load(temptext);
		if (i < PLAYER_MAX) {
			// Set how many notes are left per player
			notes.notes_left[i] = NOTES_PER_PLAYER;
		} else {
			// Set the amount of special notes
			notes.notes_left[i] = NOTES_PER_PLAYER_SPECIAL;
		}
	}
	// Init notes
	notes.start = notes.end = NULL;
}

PlyNum note_get_free(void) {
	uint8_t i;
	uint16_t notes_left_total = 0;
	// Get the sum of weights
	for (i=0; i<NOTES_TOTAL_COUNT; i++) {
		notes_left_total += notes.notes_left[i];
	}

	// Pick a random note
	uint16_t random_note = rand() % notes_left_total;

	// Find out which group it belongs to
	uint16_t notes_left_cum = 0;
	for (i=0; i<NOTES_TOTAL_COUNT; i++) {
		notes_left_cum += notes.notes_left[i];
		if (random_note < notes_left_cum) {
			notes.notes_left[i]--;
			return i;
		}
	}
	return 0;
}

void notes_add (void) {
	note_t* note = malloc(sizeof(note_t));
	uint32_t random = rand();
	if (note != NULL) {
		// Rearrange the pointers
		if (notes.end == NULL) {
			notes.start = note;
		} else {
			notes.end->next = note;
		}
		notes.end = note;

		// Set the note's starting  values
		if (random%3 == 0) {
			note->state = STATE_UP;
		} else if (random%3 == 1) {
			note->state = STATE_MID;
		} else {
			note->state = STATE_DOWN;
		}
		note->player = note_get_free();
		note->sprite = note_sprites[note->player];
		note->x = display_get_width();
		note->y = PADDING_TOP + (note->state+1) * HYDRA_ROW_HEIGHT + 8;
		note->anim_offset = random % UINT8_MAX;
		note->blitparms = (rdpq_blitparms_t){
			.theta = 0,
			.width = NOTE_WIDTH,
			.height = NOTE_HEIGHT,
			.scale_x = 0,
			.scale_y = 0,
			.cx = NOTE_WIDTH/2,
			.cy = NOTE_HEIGHT/2,
		};
		note->scale = 0;
		note->y_offset = 0;
		note->next = NULL;
	}
}

note_t* notes_get_first(void) {
	return notes.start;
}

void notes_move (void) {
	note_t* current = notes.start;
	while (current != NULL) {
		current->x -= NOTES_SPEED;
		current->y_offset = sin(current->x / NOTE_Y_OFFSET_PERIOD) * NOTE_Y_OFFSET_AMPLITUDE;
		current->blitparms.theta = sin((current->x - current->anim_offset) / NOTE_THETA_PERIOD) * NOTE_THETA_AMPLITUDE;
		current->blitparms.scale_x = 1 + sin((current->x + current->anim_offset) / NOTE_SCALE_PERIOD) * NOTE_SCALE_AMPLITUDE;
		current->blitparms.scale_y = 1 + sin((current->x + current->anim_offset) / NOTE_SCALE_PERIOD) * NOTE_SCALE_AMPLITUDE;
		current->blitparms.s0 = (current->y_offset < -NOTE_ANIMATION_OFFSET_Y ? 2 : (current->y_offset > NOTE_ANIMATION_OFFSET_Y ? 0 : 1)) * NOTE_WIDTH;
		current = current->next;
	}
}

void notes_draw (void) {
	note_t* current = notes.start;
	while (current != NULL) {
		rdpq_sprite_blit(
			current->sprite,
			current->x,
			current->y + current->y_offset,
			&current->blitparms
		);
		current = current->next;
	}
}

uint16_t notes_get_remaining (notes_remaining_t type) {
	uint16_t remaining = 0;
	uint8_t i;
	note_t* current = notes.start;
	// Get unspawned notes
	for (i=0; i<NOTES_TOTAL_COUNT && (type & NOTES_GET_REMAINING_UNSPAWNED); i++) {
		remaining += notes.notes_left[i];
	}
	// Get spawned notes
	while (current != NULL && (type & NOTES_GET_REMAINING_SPAWNED)) {
		remaining++;
		current = current->next;
	}
	return remaining;
}

void notes_destroy (note_t* dead_note) {
	note_t* current = notes.start;
	note_t* prev = NULL;
	while (current != NULL) {
		if (current == dead_note) {
			if (current == notes.start) {
				notes.start = current->next;
			}
			if (current == notes.end) {
				notes.end = prev;
			}
			if (prev) {
				prev->next = current->next;
			}
			free(dead_note);
			return;
		}
		prev = current;
		current = current->next;
	}
}

void notes_destroy_all (void) {
	note_t* current = notes.start;
	note_t* next = NULL;
	while (current != NULL) {
		next = current->next;
		notes_destroy(current);
		current = next;
	}
}

void notes_clear (void) {
	uint8_t i;
	for (i=0; i<NOTES_TOTAL_COUNT; i++) {
		sprite_free(note_sprites[i]);
	}
	notes_destroy_all();
}
