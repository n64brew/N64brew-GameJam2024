#include "intro.h"

#define INSTRUCTION_MOVE_SPEED 8
#define CURTAIN_MOVE_SPEED 1
#define CURTAIN_COUNT_X 8
#define CURTAIN_COUNT_Y 9
#define CURTAIN_TIMER_OFFSET 2
#define CURTAIN_TOP_Y 10
#define CURTAIN_BG_HEIGHT 2
#define CURTAIN_BORDER_HEIGHT 10
#define CURTAIN_BORDER_DISTANCE 25
#define CURTAIN_GOLD_COUNT 1
#define CURTAIN_GOLD_Y 15

static uint8_t intro_state = INTRO_INSTRUCTIONS;

static sprite_t* curtain_sprite;
static surface_t curtain_surf;
static int16_t curtain_offset[CURTAIN_COUNT_Y];
static uint16_t curtains_timer = 0;
static uint16_t instructions_offset = 0;

void intro_init (void) {
	curtain_sprite = sprite_load("rom:/hydraharmonics/curtain.ci4.sprite");
	curtain_surf = sprite_get_pixels(curtain_sprite);
	for (uint8_t i=0; i<CURTAIN_COUNT_Y; i++) {
		curtain_offset[i] = 0;
	}
}

void intro_interact (void) {
	if (intro_state == INTRO_INSTRUCTIONS) {
		for (uint8_t i=0; i<core_get_playercount(); i++) {
			// Player character movement
			joypad_buttons_t joypad_buttons;
			joypad_buttons = joypad_get_buttons_pressed(core_get_playercontroller(i));
			if (1 || joypad_buttons.start || joypad_buttons.a) {
				intro_state = INTRO_INSTRUCTIONS_OUT;
			}
		}
	} else if (intro_state == INTRO_INSTRUCTIONS_OUT) {
		// Move the instruction boxes vertically
		instructions_offset += INSTRUCTION_MOVE_SPEED;
		// Check to see if the instructions have flown off screen
		if (display_get_height() - INSTRUCTIONS_PADDING_Y < instructions_offset) {
			intro_state = INTRO_CURTAINS_UP;
		}
	} else if (intro_state == INTRO_CURTAINS_UP) {
		// Move the curtains out of the way
		if (curtain_offset[CURTAIN_COUNT_Y-1] > CURTAIN_BORDER_HEIGHT + CURTAIN_TOP_Y + ((CURTAIN_COUNT_Y-1) * CURTAIN_BORDER_DISTANCE)) {
			intro_state = INTRO_CURTAINS_DOWN;
		} else {
			curtains_timer += CURTAIN_MOVE_SPEED;
			for (uint8_t i=0; i<CURTAIN_COUNT_Y; i++) {
				if (curtains_timer > i * CURTAIN_TIMER_OFFSET) {
					curtain_offset[i] = ((curtains_timer - i * CURTAIN_TIMER_OFFSET)*(curtains_timer - i * CURTAIN_TIMER_OFFSET))/5 - 3*(curtains_timer - i * CURTAIN_TIMER_OFFSET);
				}
			}
		}
	} else if (intro_state == INTRO_CURTAINS_DOWN) {
		// Move the bottommost curtain fold down a little bit
		if (curtain_offset[CURTAIN_COUNT_Y-1] < CURTAIN_BORDER_HEIGHT + CURTAIN_TOP_Y + ((CURTAIN_COUNT_Y-1) * CURTAIN_BORDER_DISTANCE) - CURTAIN_GOLD_Y) {
			stage = STAGE_GAME;
		} else {
			curtain_offset[CURTAIN_COUNT_Y-1] -= CURTAIN_MOVE_SPEED;
		}

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
	uint8_t i, j;
	// Set modes
	rdpq_set_mode_copy(true);
	rdpq_mode_tlut(TLUT_RGBA16);

	// Upload the textures
	rdpq_tex_upload_tlut(
		sprite_get_palette(curtain_sprite),
		0,
		16
	);
	rdpq_tex_multi_begin();
	rdpq_tex_upload_sub(
		TILE0,
		&curtain_surf,
		&(rdpq_texparms_t){
			.t.repeats = 256
		},
		0, 0,
		curtain_sprite->width, CURTAIN_BG_HEIGHT
	);
	rdpq_tex_upload_sub(
		TILE1,
		&curtain_surf,
		NULL,
		0, CURTAIN_BG_HEIGHT,
		curtain_sprite->width, CURTAIN_BG_HEIGHT + CURTAIN_BORDER_HEIGHT
	);
	rdpq_tex_upload_sub(
		TILE2,
		&curtain_surf,
		NULL,
		0, CURTAIN_BG_HEIGHT + CURTAIN_BORDER_HEIGHT,
		curtain_sprite->width, curtain_sprite->height
	);
	rdpq_tex_multi_end();

	// Draw the curtain
	for (i=0; i<CURTAIN_COUNT_X; i++) {
		rdpq_texture_rectangle(
			TILE0,
			i*curtain_sprite->width - curtain_sprite->width/2,
			0,
			i*curtain_sprite->width - curtain_sprite->width/2 + curtain_sprite->width,
			CURTAIN_BORDER_HEIGHT/2 + CURTAIN_TOP_Y + ((CURTAIN_COUNT_Y-1) * CURTAIN_BORDER_DISTANCE) - curtain_offset[CURTAIN_COUNT_Y-1],
			0, 0
		);
		for (j=0; j<CURTAIN_COUNT_Y; j++) {
			rdpq_texture_rectangle(
				j >= CURTAIN_COUNT_Y-CURTAIN_GOLD_COUNT ? TILE2 : TILE1,
				i*curtain_sprite->width - curtain_sprite->width/2,
				 CURTAIN_TOP_Y + (j * CURTAIN_BORDER_DISTANCE) - curtain_offset[j],
				i*curtain_sprite->width - curtain_sprite->width/2 + curtain_sprite->width,
				CURTAIN_BORDER_HEIGHT + CURTAIN_TOP_Y + (j * CURTAIN_BORDER_DISTANCE) - curtain_offset[j],
				0, j >= CURTAIN_COUNT_Y-CURTAIN_GOLD_COUNT ? CURTAIN_BG_HEIGHT + CURTAIN_BORDER_HEIGHT : CURTAIN_BG_HEIGHT
			);
		}
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
	surface_free(&curtain_surf);
}
