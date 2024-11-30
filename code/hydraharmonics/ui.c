#include "ui.h"
#include "hydra.h"
#include "intro.h"

#define CI4_PALETTE_SIZE 16

#define SIGN_TEXT_X_OFFSET 2
#define SIGN_TEXT_Y_OFFSET 26

#define BG_MOVE_SPEED 1

#define BAR_INITIAL_HEIGHT (29 + PADDING_TOP)

#define FLOOR_BOARDS_COUNT 10
#define FLOOR_BOARDS_WIDTH 25.0
#define FLOOR_BOARDS_LIMIT_LEFT -60.0

#define FLOOR_TEX_WIDTH 2
#define FLOOR_TEX_HEIGHT 10
#define FLOOR_CENTRE_X (display_get_width()/2)
#define FLOOR_CENTRE_Y 100
#define FLOOR_END_Y (display_get_height() - floor_bottom_sprite->height)
#define FLOOR_TRI_HEIGHT (FLOOR_END_Y - FLOOR_CENTRE_Y)
#define FLOOR_TRI_HEIGHT_RATIO (((float)FLOOR_TRI_HEIGHT/FLOOR_TEX_HEIGHT)/-2)
#define FLOOR_TEXTURE_X_OFFSET 20

#define NOT_SET 0.0

static sprite_t* score_sprite;
static sprite_t* bg_sprite;
static sprite_t* floor_bottom_sprite;
static sprite_t* floor_dark_sprite;
static sprite_t* floor_light_sprite;
static surface_t floor_bottom_surf;
static surface_t floor_dark_surf;
static surface_t floor_light_surf;

static float floor_boards_vertices[FLOOR_BOARDS_COUNT];
static uint16_t bg_offset = 0;

void ui_init (void) {
	score_sprite = sprite_load("rom:/hydraharmonics/signs.ci4.sprite");
	bg_sprite = sprite_load("rom:/hydraharmonics/background.ci4.sprite");
	floor_bottom_sprite = sprite_load("rom:/hydraharmonics/floor-bottom.ci4.sprite");
	floor_bottom_surf = sprite_get_pixels(floor_bottom_sprite);
	floor_dark_sprite = sprite_load("rom:/hydraharmonics/floor-dark.ci4.sprite");
	floor_dark_surf = sprite_get_pixels(floor_dark_sprite);
	floor_light_sprite = sprite_load("rom:/hydraharmonics/floor-light.ci4.sprite");
	floor_light_surf = sprite_get_pixels(floor_light_sprite);

	// Designate the floor boards' position
	for (uint8_t i=0; i<FLOOR_BOARDS_COUNT; i++) {
		floor_boards_vertices[i] = i * (FLOOR_BOARDS_WIDTH) * 2;
	}
}

void ui_animate (void) {
	// Shift them along
	for (uint8_t i=0; i<FLOOR_BOARDS_COUNT; i++) {
		floor_boards_vertices[i] -= BG_MOVE_SPEED;
	}
	bg_offset = (uint16_t)(bg_offset + BG_MOVE_SPEED) % (display_get_width());
	// Check if they need to be moved to the right
	if (floor_boards_vertices[0] <= FLOOR_BOARDS_LIMIT_LEFT) {
		for (uint8_t i=1; i<FLOOR_BOARDS_COUNT; i++) {
			floor_boards_vertices[i-1] = floor_boards_vertices[i];
		}
		floor_boards_vertices[FLOOR_BOARDS_COUNT-1] = floor_boards_vertices[FLOOR_BOARDS_COUNT-2] + (FLOOR_BOARDS_WIDTH*2);
	}
}

static void ui_bg_draw (void) {
	rdpq_set_mode_copy(false);
	// Draw the main sprite
	rdpq_sprite_blit (
		bg_sprite,
		0,
		0,
		&(rdpq_blitparms_t){
			.width = bg_sprite->width - bg_offset,
			.s0 = bg_offset,
		}
	);
	// Draw the offset sprite
	rdpq_sprite_blit (
		bg_sprite,
		bg_sprite->width - bg_offset,
		0,
		&(rdpq_blitparms_t){
			.width = bg_offset,
			.s0 = 0,
		}
	);
}

static void ui_floor_draw (void) {
	// Set things up
	rdpq_set_mode_copy(true);
	rdpq_mode_tlut(TLUT_RGBA16);

	// Load the textures
	rdpq_tex_upload_tlut(
		sprite_get_palette(floor_bottom_sprite),
		CI4_PALETTE_SIZE * 0,
		CI4_PALETTE_SIZE
	);
	rdpq_tex_upload_tlut(
		sprite_get_palette(floor_light_sprite),
		CI4_PALETTE_SIZE * 1,
		CI4_PALETTE_SIZE
	);
	rdpq_tex_multi_begin();
	rdpq_tex_upload(
		TILE0,
		&floor_bottom_surf,
		&(rdpq_texparms_t){
			.palette = 0,
			.s.repeats = 256,
		}
	);
	rdpq_tex_upload(
		TILE1,
		&floor_light_surf,
		&(rdpq_texparms_t){
			.palette = 1,
			.s.repeats = 256,
		}
	);
	rdpq_tex_multi_end();

	// Draw the background rectangles
	rdpq_texture_rectangle(
		TILE0,
		0, display_get_height() - floor_bottom_sprite->height,
		display_get_width(), display_get_height(),
		0, 0
	);
	rdpq_texture_rectangle(
		TILE1,
		0, display_get_height() - floor_bottom_sprite->height - floor_light_sprite->height,
		display_get_width(), display_get_height() - floor_bottom_sprite->height,
		0, 0
	);

	// Draw the triangles
	// Load the texture
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	rdpq_sprite_upload(
		TILE2,
		floor_dark_sprite,
		&(rdpq_texparms_t){
			.s.repeats = 16,
			.t.repeats = 16,
		}
	);
	// Set the vertex defaults
	float v1[5] = {FLOOR_CENTRE_X,	FLOOR_CENTRE_Y,	NOT_SET,	FLOOR_TEX_HEIGHT * FLOOR_TRI_HEIGHT_RATIO,	1.0};
	float v2[5] = {NOT_SET,			FLOOR_END_Y,	NOT_SET,	FLOOR_TEX_HEIGHT,							1.0};
	float v3[5] = {NOT_SET,			FLOOR_END_Y,	NOT_SET,	FLOOR_TEX_HEIGHT,							1.0};
	// Loop through all triangles
	for (uint8_t i=0; i<FLOOR_BOARDS_COUNT; i++) {
		// Calculate the differing variables per-triangle
		v1[2] = FLOOR_CENTRE_X - floor_boards_vertices[i] + FLOOR_TEXTURE_X_OFFSET;
		v2[0] = floor_boards_vertices[i];
		v2[2] = FLOOR_TEXTURE_X_OFFSET;
		v3[0] = floor_boards_vertices[i] + FLOOR_BOARDS_WIDTH;
		v3[2] = ((float)FLOOR_BOARDS_WIDTH/FLOOR_TEX_WIDTH)*2+FLOOR_TEXTURE_X_OFFSET;
		// Draw the trianlge
		rdpq_triangle(
			&TRIFMT_TEX,
			v1, v2, v3
		);
	}
}

void ui_signs_draw (void) {
	rdpq_set_mode_copy(true);
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Draw the signs
		rdpq_sprite_blit (
			score_sprite,
			hydras[i].x,
			display_get_height() - score_sprite->height,
			&(rdpq_blitparms_t){
				.width = score_sprite->width / PLAYER_MAX,
				.s0 = (score_sprite->width / PLAYER_MAX) * i,
			}
		);
	}
	rdpq_set_mode_standard();
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
			display_get_height() - score_sprite->height + SIGN_TEXT_Y_OFFSET,
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
	ui_floor_draw();
	ui_bg_draw();
	// Signs are drawn in hydraharmonics.c so that they can be in front of the hydras
	ui_bars_draw();
	if (stage == STAGE_GAME || stage == STAGE_END) {
		intro_draw();
	}

}

void ui_clear (void) {
	sprite_free(score_sprite);
	sprite_free(bg_sprite);
	sprite_free(floor_bottom_sprite);
	sprite_free(floor_dark_sprite);
	sprite_free(floor_light_sprite);
	surface_free(&floor_bottom_surf);
	surface_free(&floor_dark_surf);
	surface_free(&floor_light_surf);
}
