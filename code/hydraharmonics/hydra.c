#include "hydra.h"

#define HYDRA_HEAD_OFFSET_FIRST 10
#define HYDRA_HEAD_OFFSET_EACH 2
#define SCALE 1

hydra_t hydras[PLAYER_MAX];
const uint8_t collar_offsets[PLAYER_MAX] = {17, 13, 11, 7};

void hydra_init (void) {
    char temptext[64];
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		hydras[i].state = STATE_MID;
		// Load the sprites
		sprintf(temptext, "rom:/hydraharmonics/head-%i.ci4.sprite", i);
		hydras[i].head_sprite = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/shell-%i.ci4.sprite", i);
		hydras[i].shell_sprite = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/neck-%i.ci4.sprite", i);
		hydras[i].neck_sprite = sprite_load(temptext);
		hydras[i].eye_sprite = sprite_load("rom:/hydraharmonics/eyes.ci4.sprite");

		// Set the variables
		hydras[i].pos = i;
		hydras[i].x = PADDING_LEFT + hydras[i].pos * hydras[i].head_sprite->width;
		hydras[i].y = PADDING_TOP + hydras[i].state * hydras[i].head_sprite->height;
	}
}

void hydra_move (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		if (i < core_get_playercount()) {
			// Player character movement
			joypad_buttons_t buttons;
			buttons = joypad_get_buttons(core_get_playercontroller(i));
			if (buttons.d_up) {
				hydras[i].state = STATE_UP;
			} else if (buttons.d_down) {
				hydras[i].state = STATE_DOWN;
			} else if (buttons.z) {
				hydras[i].state = STATE_HIDE;
			} else {
				hydras[i].state = STATE_MID;
			}
		} else {
			// Bot character movement
		}
		// Adjust the Y position
		hydras[i].y = PADDING_TOP+hydras[i].state*hydras[i].head_sprite->height;
	}
}

void hydra_draw (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Draw the shells
		rdpq_sprite_blit (
			hydras[i].shell_sprite,
			hydras[i].x,
			PADDING_TOP+3*hydras[i].head_sprite->height,
			&(rdpq_blitparms_t){
				.width = 48,
				.s0 = 48 * hydras[i].pos,
			}
		);

		if (hydras[i].state != STATE_HIDE) {
			// Draw the heads
			rdpq_sprite_blit (
				hydras[i].head_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos],
				hydras[i].y,
				NULL
			);

			// Draw the collar
			rdpq_sprite_blit (
				hydras[i].neck_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos],
				PADDING_TOP+3*hydras[i].head_sprite->height,
				NULL
			);

			// Draw the neck
			rdpq_sprite_blit (
				hydras[i].neck_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos],
				hydras[i].y + hydras[i].head_sprite->height,
				&(rdpq_blitparms_t){
					.height = 1,
					.scale_y = (2-hydras[i].state)*hydras[i].head_sprite->height,
				}
			);
		} else {
			// Hydra is hiding
			// Draw the eyes
			rdpq_sprite_blit (
				hydras[i].eye_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos],
				PADDING_TOP+3*hydras[i].head_sprite->height,
				NULL
			);
		}
	}
}

void hydra_clear (void) {
	uint8_t i;
	for (i=0; i<PLAYER_MAX; i++) {
		sprite_free(hydras[i].head_sprite);
		sprite_free(hydras[i].shell_sprite);
		sprite_free(hydras[i].neck_sprite);
	}
}
