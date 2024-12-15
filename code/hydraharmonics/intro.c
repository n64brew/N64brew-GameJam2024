#include "intro.h"
#include "audio.h"
#include "notes.h"
#include "ui.h"

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

#define INSTRUCTIONS_TEXT_PADDING_X 15
#define INSTRUCTIONS_TEXT_PADDING_Y 20
#define INSTRUCTIONS_TEXT_LEFT_ABS_X (INSTRUCTIONS_PADDING_X - instructions_offset + INSTRUCTIONS_TEXT_PADDING_X)
#define INSTRUCTIONS_TEXT_RIGHT_ABS_X (320/2 - instructions_offset)
#define INSTRUCTIONS_ICON_PADDING 3
#define INSTRUCTIONS_ICON_PADDING_SMALL 1
#define INSTRUCTIONS_TEXT_NEWLINE_HEIGHT 12
#define INSTRUCTIONS_NOTE_SCALE 0.5
#define INSTRUCTIONS_NOTE_TIMER 32
#define INSTRUCTIONS_NOTE_PLAYER (INSTRUCTIONS_NOTE_TIMER * 4)
#define INSTRUCTIONS_ANIMATION_STATE (note_timer/INSTRUCTIONS_NOTE_TIMER)%4

static uint8_t intro_state = INTRO_INSTRUCTIONS;

static sprite_t* curtain_sprite;
static surface_t curtain_surf;
static int16_t curtain_offset[CURTAIN_COUNT_Y];
static uint16_t curtains_timer = 0;
static uint16_t instructions_offset = 0;

static rdpq_textparms_t inst_title_parms;
static rdpq_textparms_t inst_left_parms;

static sprite_t* controls_dup_sprite;
static sprite_t* controls_ddown_sprite;
static sprite_t* controls_cup_sprite;
static sprite_t* controls_cdown_sprite;
static sprite_t* controls_a_sprite;
static sprite_t* controls_z_sprite;
static sprite_t* controls_l_sprite;
static sprite_t* controls_r_sprite;

static uint16_t note_timer = 0;

static char* note_descriptions[NOTES_TYPE_COUNT] = {
	"Normal: Eat these!",
	"Back: Move backwards!",
	"Front: Move forwards!",
	"2-swap: Swap with one!",
	"4-swap: Swap with all!",
	"Sweet: Worth two points!",
	"Sour: Lose one point!",
};

void intro_init (void) {
	curtain_sprite = sprite_load("rom:/hydraharmonics/curtain.ci4.sprite");
	curtain_surf = sprite_get_pixels(curtain_sprite);
	controls_dup_sprite = sprite_load("rom:/hydraharmonics/button-dup.ci4.sprite");
	controls_ddown_sprite = sprite_load("rom:/hydraharmonics/button-ddown.ci4.sprite");
	controls_cup_sprite = sprite_load("rom:/hydraharmonics/button-cup.ci4.sprite");
	controls_cdown_sprite = sprite_load("rom:/hydraharmonics/button-cdown.ci4.sprite");
	controls_a_sprite = sprite_load("rom:/hydraharmonics/button-a.ci4.sprite");
	controls_z_sprite = sprite_load("rom:/hydraharmonics/button-z.ci4.sprite");
	controls_l_sprite = sprite_load("rom:/hydraharmonics/button-l.ci4.sprite");
	controls_r_sprite = sprite_load("rom:/hydraharmonics/button-r.ci4.sprite");
	for (uint8_t i=0; i<CURTAIN_COUNT_Y; i++) {
		curtain_offset[i] = 0;
	}
	inst_title_parms = (rdpq_textparms_t){
		.width = display_get_width() - INSTRUCTIONS_PADDING_X*2 - INSTRUCTIONS_TEXT_PADDING_X*2,
		.align = ALIGN_CENTER,
	};
	inst_left_parms = (rdpq_textparms_t){
		.width = (display_get_width() - INSTRUCTIONS_PADDING_X*2 - INSTRUCTIONS_TEXT_PADDING_X*2)/2,
		.align = ALIGN_LEFT,
		.wrap = WRAP_WORD,
	};
}

void intro_interact (void) {
	if (intro_state == INTRO_INSTRUCTIONS) {
		for (uint8_t i=0; i<core_get_playercount(); i++) {
			// Player character movement
			joypad_buttons_t joypad_buttons;
			joypad_buttons = joypad_get_buttons_pressed(core_get_playercontroller(i));
			if (joypad_buttons.start || joypad_buttons.a || joypad_buttons.l || joypad_buttons.r || joypad_buttons.z) {
				intro_state = INTRO_INSTRUCTIONS_OUT;
			}
		}
	} else if (intro_state == INTRO_INSTRUCTIONS_OUT) {
		// Move the instruction boxes vertically
		instructions_offset += INSTRUCTION_MOVE_SPEED;
		// Check to see if the instructions have flown off screen
		if (display_get_width() - INSTRUCTIONS_PADDING_X < instructions_offset) {
			intro_state = INTRO_CURTAINS_UP;
			audio_music_play();
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
		bool curtain_ready = false;
		bool signs_ready = false;
		// Move the bottommost curtain fold down a little bit
		if (curtain_offset[CURTAIN_COUNT_Y-1] < CURTAIN_BORDER_HEIGHT + CURTAIN_TOP_Y + ((CURTAIN_COUNT_Y-1) * CURTAIN_BORDER_DISTANCE) - CURTAIN_GOLD_Y) {
			curtain_ready = true;
		} else {
			curtain_offset[CURTAIN_COUNT_Y-1] -= CURTAIN_MOVE_SPEED;
		}
		// Move the signs up
		if (sign_y_offset >= SIGN_HEIGHT) {
			sign_y_offset = SIGN_HEIGHT;
			signs_ready = true;
		} else {
			sign_y_offset += CURTAIN_MOVE_SPEED;
		}
		if (curtain_ready && signs_ready) {
			stage = STAGE_GAME;
		}
	}
}

void intro_instructions_draw (void) {
	if (intro_state == INTRO_INSTRUCTIONS || intro_state == INTRO_INSTRUCTIONS_OUT) {
		rdpq_textmetrics_t textmetrics;
		uint8_t text_y_offset = 0;
		uint8_t title_y_offset = 0;
		// Draw the panel
		ui_panel_draw (
			INSTRUCTIONS_PADDING_X - instructions_offset,
			INSTRUCTIONS_PADDING_Y,
			display_get_width() - INSTRUCTIONS_PADDING_X - instructions_offset,
			display_get_height() - INSTRUCTIONS_PADDING_Y
		);
		// Header text
		textmetrics = rdpq_text_printf(
			&inst_title_parms,
			FONT_CLARENDON,
			INSTRUCTIONS_TEXT_LEFT_ABS_X,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y,
			"Hydra Harmonics\n"
		);
		title_y_offset += textmetrics.advance_y;
		// Left-hand text
		textmetrics = rdpq_text_printf(
			&inst_left_parms,
			FONT_DEFAULT,
			INSTRUCTIONS_TEXT_LEFT_ABS_X,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset,
			"Taste the music! Eat the most notes to win.\nControls:"
		);
		text_y_offset += textmetrics.advance_y + INSTRUCTIONS_ICON_PADDING;
		// Icons
		// "Up" row
		rdpq_set_mode_copy(true);
		rdpq_sprite_blit (
			controls_dup_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		rdpq_sprite_blit (
			controls_cup_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X + controls_dup_sprite->width + INSTRUCTIONS_ICON_PADDING_SMALL,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		textmetrics = rdpq_text_printf(
			NULL,
			FONT_DEFAULT,
			INSTRUCTIONS_TEXT_LEFT_ABS_X + controls_dup_sprite->width + controls_cup_sprite->width + INSTRUCTIONS_ICON_PADDING,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset + INSTRUCTIONS_TEXT_NEWLINE_HEIGHT,
			"Move up!"
		);
		text_y_offset += controls_dup_sprite->height + INSTRUCTIONS_ICON_PADDING;
		// "Down" row
		rdpq_set_mode_copy(true);
		rdpq_sprite_blit (
			controls_ddown_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		rdpq_sprite_blit (
			controls_cdown_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X + controls_dup_sprite->width + INSTRUCTIONS_ICON_PADDING_SMALL,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		textmetrics = rdpq_text_printf(
			NULL,
			FONT_DEFAULT,
			INSTRUCTIONS_TEXT_LEFT_ABS_X + controls_dup_sprite->width + controls_cup_sprite->width + INSTRUCTIONS_ICON_PADDING,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset + INSTRUCTIONS_TEXT_NEWLINE_HEIGHT,
			"Move down!"
		);
		text_y_offset += controls_ddown_sprite->height + INSTRUCTIONS_ICON_PADDING;
		// "Chomp" row
		rdpq_set_mode_copy(true);
		rdpq_sprite_blit (
			controls_l_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		rdpq_sprite_blit (
			controls_r_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X + controls_dup_sprite->width + INSTRUCTIONS_ICON_PADDING_SMALL,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		text_y_offset += controls_l_sprite->height + INSTRUCTIONS_ICON_PADDING;
		rdpq_sprite_blit (
			controls_a_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		rdpq_sprite_blit (
			controls_z_sprite,
			INSTRUCTIONS_TEXT_LEFT_ABS_X + controls_dup_sprite->width + INSTRUCTIONS_ICON_PADDING_SMALL,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			NULL
		);
		textmetrics = rdpq_text_printf(
			NULL,
			FONT_DEFAULT,
			INSTRUCTIONS_TEXT_LEFT_ABS_X + controls_dup_sprite->width + controls_cup_sprite->width + INSTRUCTIONS_ICON_PADDING,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + title_y_offset + text_y_offset,
			"Chomp!"
		);
		text_y_offset += controls_a_sprite->height + INSTRUCTIONS_ICON_PADDING;
		textmetrics = rdpq_text_printf(
			NULL,
			FONT_DEFAULT,
			INSTRUCTIONS_TEXT_LEFT_ABS_X,
			INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + INSTRUCTIONS_TEXT_NEWLINE_HEIGHT + title_y_offset + text_y_offset,
			"N64 Squid: Lead programmer\nGary Jones III: Art director, music\nBrozilla: Audio conversion; StatycTyr: Additional art"
		);
		text_y_offset = 0;

		// Right column
		// Draw the notes
		rdpq_set_mode_standard();
		rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
		for (uint8_t i=0; i<NOTES_TYPE_COUNT; i++) {
			rdpq_sprite_blit(
				note_sprites[(note_timer/INSTRUCTIONS_NOTE_PLAYER) % NOTES_TOTAL_COUNT],
				INSTRUCTIONS_TEXT_RIGHT_ABS_X,
				INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_NEWLINE_HEIGHT + title_y_offset + i*NOTE_HEIGHT * INSTRUCTIONS_NOTE_SCALE,
				&(rdpq_blitparms_t){
					.width = NOTE_WIDTH,
					.height = NOTE_HEIGHT,
					.scale_x = INSTRUCTIONS_NOTE_SCALE,
					.scale_y = INSTRUCTIONS_NOTE_SCALE,
					.s0 = NOTE_WIDTH * (INSTRUCTIONS_ANIMATION_STATE == 3 ? 1 : INSTRUCTIONS_ANIMATION_STATE),
					.t0 = NOTE_HEIGHT * i,
				}
			);
		}
		note_timer++;
		// Draw the descriptions
		for (uint8_t i=0; i<NOTES_TYPE_COUNT; i++) {
			rdpq_text_printf(
				NULL,
				FONT_DEFAULT,
				INSTRUCTIONS_TEXT_RIGHT_ABS_X + NOTE_WIDTH * INSTRUCTIONS_NOTE_SCALE + INSTRUCTIONS_ICON_PADDING,
				INSTRUCTIONS_PADDING_Y + INSTRUCTIONS_TEXT_PADDING_Y + INSTRUCTIONS_ICON_PADDING + title_y_offset + i*NOTE_HEIGHT * INSTRUCTIONS_NOTE_SCALE,
				note_descriptions[i]
			);
		}
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
	sprite_free(controls_dup_sprite);
	sprite_free(controls_ddown_sprite);
	sprite_free(controls_cup_sprite);
	sprite_free(controls_cdown_sprite);
	sprite_free(controls_a_sprite);
	sprite_free(controls_z_sprite);
	sprite_free(controls_l_sprite);
	sprite_free(controls_r_sprite);
}
