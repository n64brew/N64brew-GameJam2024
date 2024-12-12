#include "notes.h"
#include "logic.h"
#include "effects.h"

#define NOTE_Y_OFFSET_PERIOD 32
#define NOTE_Y_OFFSET_AMPLITUDE 4
#define NOTE_THETA_PERIOD 16
#define NOTE_THETA_AMPLITUDE 0.1
#define NOTE_SCALE_PERIOD 0.1
#define NOTE_SCALE_AMPLITUDE 0.05
#define NOTE_ANIMATION_BOUNCE 0x1
#define NOTE_ANIMATION_ROTATE 0x2
#define NOTE_ANIMATION_PULSE 0x4
#define NOTE_ANIMATION_SPEED 8
#define NOTE_ANIMATION_OFFSET_Y NOTE_Y_OFFSET_AMPLITUDE * 0.35
#define NOTE_SPARKLE_CHANCE 16
#define NOTE_SPARKLE_MIN 8

#define NOTE_PLAYER_MAX_RETRIES 4
#define NOTE_CHANCE_DOUBLE 4
#define NOTE_CHANCE_TRIPLE 2
#define NOTE_CHANCE_NON_STANDARD (4-core_get_aidifficulty())
#define NOTE_CHANCE_FIXED 2

// Global vars
static note_ll_t notes;
static uint8_t sparkle_timer = 0;
sprite_t* note_sprites[NOTES_TOTAL_COUNT];

const float note_speeds[NOTE_SPEED_COUNT] = {
	2.5,
	3.0,
	4.0,
};

const float note_spawn[NOTE_SPEED_COUNT] = {
	1.0,
	0.0,
	1.75,
};

void notes_init(void) {
	char temptext[64];
	for (uint8_t i=0; i<NOTES_TOTAL_COUNT; i++) {
		// Load the note sprites
		sprintf(temptext, "rom:/hydraharmonics/note-%i.ci4.sprite", i);
		note_sprites[i] = sprite_load(temptext);
		if (i < NOTES_TOTAL_COUNT) {
			// Set how many notes are left per player
			notes.notes_left[i].regular = NOTES_PER_PLAYER;
			// Set the amount of special notes
			notes.notes_left[i].special = NOTES_PER_PLAYER_SPECIAL;
		}
	}
	// Init notes
	notes.start = notes.end = NULL;
}

notes_types_t note_get_random_type (PlyNum p) {
	uint32_t random = rand();
	// Check if we've run out of a particular type
	if (notes.notes_left[p].regular == 0) {
		return ((random % (NOTES_TYPE_COUNT-1)) + 1);
	} else if (notes.notes_left[p].special == 0) {
		return NOTES_TYPE_STANDARD;
	}
	// Check to see if the player is in 1st or last place (scorewise)
	if (p == scores_get_extreme(SCORES_GET_FIRST)) {
		// User is in first place, let's bring them down
		if (random % NOTE_CHANCE_NON_STANDARD) {
			return NOTES_TYPE_STANDARD;
		} else if (random % NOTE_CHANCE_FIXED) {
			return NOTES_TYPE_SOUR;
		} else {
			return random % NOTES_TYPE_COUNT;
		}
	} else if (p == scores_get_extreme(SCORES_GET_LAST)) {
		// Use is in last place. Let's help a bit
		if (random % NOTE_CHANCE_NON_STANDARD) {
			return NOTES_TYPE_STANDARD;
		} else if (random % NOTE_CHANCE_FIXED) {
			return NOTES_TYPE_SWEET;
		} else {
			return random % NOTES_TYPE_COUNT;
		}
	} else {
		// Standard rules
		if (random % NOTE_CHANCE_NON_STANDARD) {
			// Spawn a normal note
			return NOTES_TYPE_STANDARD;
		} else {
			return random % NOTES_TYPE_COUNT;
		}
	}
}

PlyNum note_get_free(void) {
	uint8_t i, j = 0;
	uint16_t notes_left_total;
	static PlyNum last_player = PLAYER_MAX;
	if (last_player == PLAYER_MAX) {
		last_player = rand() % PLAYER_MAX;
	}

	// Get the sum of weights
	notes_left_total = notes_get_remaining(NOTES_GET_REMAINING_UNSPAWNED);

	// Try not to pick the same note a few times
	while (j <= NOTE_PLAYER_MAX_RETRIES) {
		// Pick a random note
		uint16_t random_note = rand() % notes_left_total;

		// Find out which group it belongs to
		uint16_t notes_left_cum = 0;
		for (i=0; i<NOTES_TOTAL_COUNT; i++) {
			notes_left_cum += notes.notes_left[i].regular + notes.notes_left[i].special;
			if (random_note < notes_left_cum) {
				// Check if this is a note used by the previous player
				if (i == last_player && j++ < NOTE_PLAYER_MAX_RETRIES) {
					break;
				} else {
					last_player = i;
					return i;
				}
			}
		}
	}
	return 0;
}

void notes_add_more (void) {
	// Check to see if anyone is left with zero notes
	for (uint8_t i=0; i<NOTES_TOTAL_COUNT; i++) {
		if (notes.notes_left[i].regular + notes.notes_left[i].special) {
			continue;
		}
		// Give everyone some notes
		for (uint8_t i=0; i<NOTES_TOTAL_COUNT; i++) {
			notes.notes_left[i].regular += 1;
			notes.notes_left[i].special += 1 + core_get_aidifficulty();
		}
		return;
	}
}

void notes_add (PlyNum player, notes_types_t type, hydraharmonics_state_t state) {
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
		note->player = player;
		note->state = state;
		note->type = type;
		note->sprite = note_sprites[player];
		note->x = display_get_width() + NOTE_WIDTH/2;
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
			.t0 = NOTE_HEIGHT * note->type,
		};
		note->scale = 0;
		note->y_offset = 0;
		note->next = NULL;

		// Decrease the counters
		if (type == NOTES_TYPE_STANDARD) {
			notes.notes_left[player].regular--;
		} else {
			notes.notes_left[player].special--;
		}
	}
}

// This function looks awful. Please forgive me
void notes_check_and_add (void) {
	AiDiff diff = core_get_aidifficulty();
	// Spawn the first note
	if (!notes_get_remaining(NOTES_GET_REMAINING_UNSPAWNED))
		return;
	PlyNum p = note_get_free();
	notes_add(
		p,
		note_get_random_type(p),
		rand() % STATES_USABLE
	);

	// Have a chance to spawn two notes at medium and hard difficulty
	if ( (diff == DIFF_MEDIUM || diff == DIFF_HARD) && !(rand() % NOTE_CHANCE_DOUBLE) && notes_get_remaining(NOTES_GET_REMAINING_UNSPAWNED)) {
		hydraharmonics_state_t last_note_state = notes.end->state;
		hydraharmonics_state_t new_note_state = 0;
		while (new_note_state == last_note_state) {
			new_note_state = (new_note_state + 1) % STATES_USABLE;
		}
		p = note_get_free();
		notes_add(
			p,
			note_get_random_type(p),
			new_note_state
		);

		// Have a chance to spawn a third note at a harder difficulty
		if (diff == DIFF_HARD && !(rand() % NOTE_CHANCE_TRIPLE) && notes_get_remaining(NOTES_GET_REMAINING_UNSPAWNED)) {
			new_note_state = 0;
			while (new_note_state == last_note_state || new_note_state == notes.end->state) {
				new_note_state = (new_note_state + 1) % STATES_USABLE;
			}
			p = note_get_free();
			notes_add(
				p,
				note_get_random_type(p),
				new_note_state
			);
		}
	}
	notes_add_more();
}

note_t* notes_get_first(void) {
	return notes.start;
}

void notes_move (void) {
	note_t* current = notes.start;
	sparkle_timer++;
	while (current != NULL) {
		// Move and animate the note
		current->x -= note_speeds[game_speed];
		current->y_offset = sin(current->x / NOTE_Y_OFFSET_PERIOD) * NOTE_Y_OFFSET_AMPLITUDE;
		current->blitparms.theta = sin((current->x - current->anim_offset) / NOTE_THETA_PERIOD) * NOTE_THETA_AMPLITUDE;
		current->blitparms.scale_x = 1 + sin((current->x + current->anim_offset) / NOTE_SCALE_PERIOD) * NOTE_SCALE_AMPLITUDE;
		current->blitparms.scale_y = 1 + sin((current->x + current->anim_offset) / NOTE_SCALE_PERIOD) * NOTE_SCALE_AMPLITUDE;
		current->blitparms.s0 = (current->y_offset < -NOTE_ANIMATION_OFFSET_Y ? 2 : (current->y_offset > NOTE_ANIMATION_OFFSET_Y ? 0 : 1)) * NOTE_WIDTH;
		// Randomly generate a sparkle
		if (
			(current->type == NOTES_TYPE_SWEET ||
			sparkle_timer >= NOTE_SPARKLE_MIN )
			&& current->type != NOTES_TYPE_SOUR
			&& !(rand() % NOTE_SPARKLE_CHANCE)
		) {
			effects_add(
				current->player,
				EFFECT_SPARKLE,
				current->x - NOTE_WIDTH/4 + (rand()%NOTE_WIDTH/2),
				current->y - NOTE_HEIGHT/4 + (rand()%NOTE_HEIGHT/2)
			);
			sparkle_timer = 0;
		}
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
	/*
	for (uint8_t i=0; i<NOTES_TOTAL_COUNT; i++) {
		rdpq_text_printf(NULL, FONT_DEFAULT, 20, 20+i*20,
			"%i, %i", notes.notes_left[i].regular, notes.notes_left[i].special
		);
	}
	*/
}

uint16_t notes_get_remaining (notes_remaining_t type) {
	uint16_t remaining = 0;
	uint8_t i;
	note_t* current = notes.start;
	// Get unspawned notes
	for (i=0; i<NOTES_TOTAL_COUNT && (type & NOTES_GET_REMAINING_UNSPAWNED); i++) {
		remaining += notes.notes_left[i].regular + notes.notes_left[i].special;
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
	for (uint8_t i=0; i<NOTES_TOTAL_COUNT; i++) {
		sprite_free(note_sprites[i]);
	}
	notes_destroy_all();
}
