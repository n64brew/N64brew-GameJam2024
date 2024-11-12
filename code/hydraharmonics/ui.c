#include "ui.h"
#include "hydra.h"

#define SIGN_TEXT_X_OFFSET 2
#define SIGN_TEXT_Y_OFFSET 26

#define BAR_INITIAL_HEIGHT 39

static sprite_t* score_sprite;
static sprite_t* bg_sprite;
static sprite_t* floor_sprite;

void ui_init (void) {
	score_sprite = sprite_load("rom:/hydraharmonics/signs.ci4.sprite");
	bg_sprite = sprite_load("rom:/hydraharmonics/background.ci4.sprite");
	floor_sprite = sprite_load("rom:/hydraharmonics/floor.ci4.sprite");
}

static void ui_bg_draw (void) {
	rdpq_sprite_blit (
		bg_sprite,
		0,
		0,
		NULL
	);
	rdpq_sprite_blit (
		floor_sprite,
		0,
		display_get_height() - floor_sprite->height,
		NULL
	);
}

static void ui_signs_draw (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Draw the signs
		rdpq_sprite_blit (
			score_sprite,
			hydras[i].x,
			display_get_height() - score_sprite->height - PADDING_TOP,
			&(rdpq_blitparms_t){
				.width = score_sprite->width / PLAYER_MAX,
				.s0 = (score_sprite->width / PLAYER_MAX) * i,
			}
		);
	}
		// Draw the player numbers
	rdpq_textparms_t sign_params = (rdpq_textparms_t) {
		.width = score_sprite->width / PLAYER_MAX,
		.align = ALIGN_CENTER
	};
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		rdpq_text_printf(
			&sign_params,
			FONT_CLARENDON,
			hydras[i].x + SIGN_TEXT_X_OFFSET,
			display_get_height() - score_sprite->height - PADDING_TOP + SIGN_TEXT_Y_OFFSET,
			"P%i", i+1
		);
	}
}

static void ui_bars_draw (void) {
	for (uint8_t i=0; i<HEAD_STATES_MAX; i++) {
		rdpq_set_mode_fill(RGBA32(255, 255, 255, 0));
		rdpq_fill_rectangle(0, BAR_INITIAL_HEIGHT + i*HYDRA_ROW_HEIGHT, display_get_width(), BAR_INITIAL_HEIGHT + i*HYDRA_ROW_HEIGHT + 1);
		rdpq_set_mode_fill(RGBA32(0, 0, 0, 0));
		rdpq_fill_rectangle(0, BAR_INITIAL_HEIGHT + i*HYDRA_ROW_HEIGHT + 1, display_get_width(), BAR_INITIAL_HEIGHT + i*HYDRA_ROW_HEIGHT + 2);
	}
}

void ui_draw (void) {
	ui_bg_draw();
	ui_signs_draw();
	ui_bars_draw();
}

void ui_clear (void) {
	sprite_free(score_sprite);
	sprite_free(bg_sprite);
	sprite_free(floor_sprite);
}
