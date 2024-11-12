#include "intro.h"

#define INSTRUCTION_MOVE_SPEED 8
#define CURTAIN_MOVE_SPEED 2

static uint8_t intro_state = INTRO_INSTRUCTIONS;

static sprite_t* curtain_sprite;
static uint16_t curtain_offset = 0;
static uint16_t instructions_offset = 0;

void intro_init (void) {
	curtain_sprite = sprite_load("rom:/hydraharmonics/curtain.ci4.sprite");
}

void intro_interact (void) {
	if (intro_state == INTRO_INSTRUCTIONS) {
		for (uint8_t i=0; i<core_get_playercount(); i++) {
			// Player character movement
			joypad_buttons_t joypad_buttons;
			joypad_buttons = joypad_get_buttons_pressed(core_get_playercontroller(i));
			if (joypad_buttons.start || joypad_buttons.a) {
				intro_state = INTRO_INSTRUCTIONS_OUT;
			}
		}
	} else if (intro_state == INTRO_INSTRUCTIONS_OUT) {
		// Move the instruction boxes vertically
		instructions_offset += INSTRUCTION_MOVE_SPEED;
		// Check to see if the instructions have flown off screen
		if (display_get_height() - INSTRUCTIONS_PADDING_Y < instructions_offset) {
			intro_state = INTRO_CURTAINS_OUT;
		}
	} else if (intro_state == INTRO_CURTAINS_OUT) {
		if (curtain_offset >= curtain_sprite->width) {
			stage = STAGE_GAME;
		}
		curtain_offset += CURTAIN_MOVE_SPEED;
	}
}

void intro_instructions_draw (void) {
	if (intro_state == INTRO_INSTRUCTIONS || intro_state == INTRO_INSTRUCTIONS_OUT) {
		// Draw the rectangles
		rdpq_set_mode_fill(RGBA32(0xCC, 0xCC, 0xCC, 0));
		rdpq_fill_rectangle(
			INSTRUCTIONS_PADDING_X,
			INSTRUCTIONS_PADDING_Y + instructions_offset,
			display_get_width()/2 - INSTRUCTIONS_PADDING_X/2,
			display_get_height() - INSTRUCTIONS_PADDING_Y + instructions_offset
		);
		rdpq_fill_rectangle(
			display_get_width()/2 + INSTRUCTIONS_PADDING_X/2,
			INSTRUCTIONS_PADDING_Y - instructions_offset,
			display_get_width() - INSTRUCTIONS_PADDING_X,
			display_get_height() - INSTRUCTIONS_PADDING_Y - instructions_offset
		);
	}
}

void intro_curtain_draw (void) {
	float scale;

	if (curtain_offset < curtain_sprite->width) {
		// Set things up
		scale = (curtain_sprite->width - curtain_offset) / (float)curtain_sprite->width;
		rdpq_set_mode_standard();
		rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
		// Left curtain
		rdpq_sprite_blit (
			curtain_sprite,
			0,
			0,
			&(rdpq_blitparms_t){
				.width = curtain_sprite->width - curtain_offset,
				.scale_x = scale,
				.s0 = 0,
			}
		);
		// Right curtain
		rdpq_sprite_blit (
			curtain_sprite,
			display_get_width()/2 + curtain_offset + curtain_offset * scale,
			0,
			&(rdpq_blitparms_t){
				.width = curtain_sprite->width - curtain_offset,
				.scale_x = scale,
				.s0 = curtain_offset,
			}
		);
	}
}

void intro_draw (void) {
	intro_curtain_draw();
	if (intro_state == INTRO_INSTRUCTIONS || intro_state == INTRO_INSTRUCTIONS_OUT) {
		intro_instructions_draw();
	}
}

void intro_clear (void) {
	sprite_free(curtain_sprite);
}
