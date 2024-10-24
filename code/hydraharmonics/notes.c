#include "notes.h"

static note_ll_t notes;
static sprite_t* note_sprites[PLAYER_MAX];

void notes_init(void) {
	char temptext[64];
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Load the note sprites
		sprintf(temptext, "rom:/hydraharmonics/note-%i.ci4.sprite", i);
		note_sprites[i] = sprite_load(temptext);
	}
	// Init notes
	notes.start = notes.end = NULL;
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
		note->x = display_get_width();
		note->y = PADDING_TOP + note->state*HYDRA_HEAD_HEIGHT + 8;
		note->player = 0;
		note->sprite = note_sprites[note->player];
		note->next = NULL;
	}
}

note_t* notes_get_first(void) {
	return notes.start;
}

void notes_move (void) {
	note_t* current = notes.start;
	while (current != NULL) {
		current->x -= NOTE_SPEED;
		current = current->next;
	}
}

void notes_draw (void) {
	note_t* current = notes.start;
	while (current != NULL) {
		rdpq_sprite_blit(
			current->sprite,
			current->x,
			current->y,
			NULL
		);
		current = current->next;
	}
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
			return;
		}
		current = current->next;
	}
}

void notes_clear (void) {
	uint8_t i;
	for (i=0; i<PLAYER_MAX; i++) {
		sprite_free(note_sprites[i]);
	}
}
