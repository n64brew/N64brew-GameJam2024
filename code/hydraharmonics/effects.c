#include "effects.h"
#include "notes.h"
#include "hydra.h"

#define EFFECTS_WIDTH 15
#define EFFECTS_HEIGHT 17

#define EFFECTS_FLOWER_SPEED 10
#define EFFECTS_FLOWER_AMPLITUDE 20

#define EFFECTS_NOTE_ACCELERATION 0.5
#define EFFECTS_NOTE_MAX_SPEED 3
#define EFFECTS_NOTE_SHRINK_FACTOR 0.9

#define EFFECTS_SHOCK_SIZE_MIN 0.5
#define EFFECTS_SHOCK_SIZE_MAX 1.0
#define EFFECTS_SHOCK_FREQ 10

#define EFFECTS_SPARKLE_GROW_SPEED 20.0
#define EFFECTS_SPARKLE_TEMP (current->timer / EFFECTS_SPARKLE_GROW_SPEED)
#define EFFECTS_SPARKLE_ACCEL (1.0/64)
#define EFFECTS_SPARKLE_ACCEL_MAX 2
#define EFFECTS_SPARKLE_X_DIVISOR 1.5

static effect_ll_t effects_list;
static sprite_t* effects_sprites[NOTES_TOTAL_COUNT];
static uint8_t effects_duration[EFFECT_COUNT] = {
	25, 75, 75, HYDRA_STUN_DURATION, 75
};

void effects_init (void) {
	char temptext[64];
	for (uint8_t i=0; i<NOTES_TOTAL_COUNT; i++) {
		// Load the note sprites
		sprintf(temptext, "rom:/hydraharmonics/effects-%i.ci4.sprite", i);
		effects_sprites[i] = sprite_load(temptext);
	}
	effects_list.start = NULL;
	effects_list.end = NULL;
}

void effects_add (PlyNum player, effect_types_t type, float x, float y) {
	effect_t* effect = malloc(sizeof(effect_t));
	if (effect != NULL) {
		// Rearrange the pointers
		if (effects_list.end == NULL) {
			effects_list.start = effect;
		} else {
			effects_list.end->next = effect;
		}
		effects_list.end = effect;
		// Set the variables
		effect->sprite = effects_sprites[player];
		effect->player = player;
		effect->type = type;
		effect->x = x;
		effect->y = y;
		effect->accel = 0;
		effect->timer = 0;
		effect->blitparms = (rdpq_blitparms_t){
			.width = EFFECTS_WIDTH,
			.s0 = type * EFFECTS_WIDTH,
			.cx = EFFECTS_WIDTH/2,
			.cy = EFFECTS_HEIGHT/2,
			.scale_x = 1.0,
			.scale_y = 1.0,
		};
		effect->next = NULL;
	}
}

void effects_animate (void) {
	effect_t* current = effects_list.start;
	float temp;
	while (current != NULL) {
		effect_t* next = current->next;
		current->timer++;
		switch (current->type) {
			case EFFECT_FLOWER:
				current->accel =
					abs((current->timer % (EFFECTS_FLOWER_SPEED*2)) - EFFECTS_FLOWER_SPEED) - EFFECTS_FLOWER_SPEED/2;
				current->accel /= EFFECTS_FLOWER_AMPLITUDE;
				current->blitparms.theta += current->accel;
				current->y = hydras[current->player].y + hydra_get_hat_offset(current->player) + HYDRA_CHEW_EFFECT_OFFSET_Y;
				break;
			case EFFECT_NOTE_BIG:
			case EFFECT_NOTE_SMALL:
				if (current->accel < EFFECTS_NOTE_MAX_SPEED) {
					// Move it up first
					current->accel += EFFECTS_NOTE_ACCELERATION;
					current->y -= EFFECTS_NOTE_MAX_SPEED - current->accel;
				} else {
					// Shrink it
					current->blitparms.scale_x *= EFFECTS_NOTE_SHRINK_FACTOR;
					current->blitparms.scale_y *= EFFECTS_NOTE_SHRINK_FACTOR;
				}
				break;
			case EFFECT_SHOCK:
				// Just alternate between two shock levels
				temp = EFFECTS_SHOCK_SIZE_MAX - (((current->timer/EFFECTS_SHOCK_FREQ)%2) * EFFECTS_SHOCK_SIZE_MIN);
				current->blitparms.scale_x = current->blitparms.scale_y = temp;
				break;
			case EFFECT_SPARKLE:
				// Move it down
				current->accel += EFFECTS_SPARKLE_ACCEL;
				current->y += current->accel;
				current->x -= note_speeds[game_speed]/EFFECTS_SPARKLE_X_DIVISOR;
				// Grow then shrink the sparkle
				temp = -(EFFECTS_SPARKLE_TEMP*EFFECTS_SPARKLE_TEMP/2) + EFFECTS_SPARKLE_TEMP + 0.5;
				current->blitparms.scale_x = current->blitparms.scale_y = temp > 0 ? temp : current->blitparms.scale_x;
			default:
				break;
		}
		if (
			(current->type != EFFECT_SHOCK && current->timer > effects_duration[current->type]) ||
			(current->type == EFFECT_SHOCK && hydras[current->player].animation != HYDRA_ANIMATION_STUN)
		) {
			effects_destroy(current);
		}
		current = next;
	}
}

void effects_draw (void) {
	effect_t* current = effects_list.start;
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	while (current != NULL) {
		rdpq_sprite_blit(
			current->sprite,
			current->x,
			current->y,
			&current->blitparms
		);
		current = current->next;
	}
}

void effects_destroy (effect_t* dead_effect) {
	effect_t* current = effects_list.start;
	effect_t* prev = NULL;
	while (current != NULL) {
		if (current == dead_effect) {
			if (current == effects_list.start) {
				effects_list.start = current->next;
			}
			if (current == effects_list.end) {
				effects_list.end = prev;
			}
			if (prev) {
				prev->next = current->next;
			}
			free(dead_effect);
			return;
		}
		prev = current;
		current = current->next;
	}
}

void effects_destroy_all (void) {
	effect_t* current = effects_list.start;
	effect_t* next = NULL;
	while (current != NULL) {
		next = current->next;
		effects_destroy(current);
		current = next;
	}
}

void effects_clear (void) {
	uint8_t i;
	effects_destroy_all();
	for (i=0; i<NOTES_TOTAL_COUNT; i++) {
		sprite_free(effects_sprites[i]);
	}
}
