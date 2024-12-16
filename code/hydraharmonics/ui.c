#include "ui.h"
#include "hydra.h"
#include "intro.h"

#define CI4_PALETTE_SIZE 16

#define SIGN_TEXT_X_OFFSET 2
#define SIGN_TEXT_Y_OFFSET 26
#define SIGNS_COUNT PLAYER_MAX

#define BAR_INITIAL_HEIGHT (29 + PADDING_TOP)
#define BAR_COUNT (HEAD_STATES_MAX+1)

#define BACKGROUND_HILLS_COUNT 4
#define BACKGROUND_HILLS_LAST 0
#define BACKGROUND_HILLS_CLOUDS 3
#define BACKGROUND_HILL_SPEED_MIN 0.5
#define BACKGROUND_HILL_SPEED_VAR 0.25

#define BACKGROUND_SKY_COLOR RGBA32(0xDD, 0x87, 0xA4, 0)
#define BACKGROUND_LAKE_COLOR RGBA32(0xE6, 0xA5, 0xBB, 0)
#define BACKGROUND_HILL_FRONT_COLOR RGBA32(0x26, 0x3B, 0x52, 0)
#define BACKGROUND_HILL_BACK_COLOR RGBA32(0x82, 0x64, 0x80, 0)
#define BACKGROUND_SKY_HEIGHT 180
#define BACKGROUND_HILL_FRONT_HEIGHT (BACKGROUND_SKY_HEIGHT-21)
#define BACKGROUND_HILL_BACK_HEIGHT (BACKGROUND_HILL_FRONT_HEIGHT-32)
#define BACKGROUND_HILL_CLOUD_HEIGHT (BACKGROUND_HILL_BACK_HEIGHT-64)
#define BACKGROUND_HILL_CLOUD_RAND 16
#define BACKGROUND_LAKE_HEIGHT (240-BACKGROUND_SKY_HEIGHT-30)

#define PANEL_CORNER_WIDTH 12
#define PANEL_CORNER_HEIGHT PANEL_CORNER_WIDTH
#define PANEL_CORNER_SPRITE_WIDTH 16
#define PANEL_CORNER_SPRITE_HEIGHT PANEL_CORNER_SPRITE_WIDTH
#define PANEL_BORDER_LONG 8
#define PANEL_BORDER_SHORT 2
#define PANEL_ROPE_WIDTH 8
#define PANEL_ROPE_X_OFFSET 10
#define PANEL_ROPE_RING_HEIGHT 4

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

typedef struct bg_s {
	float x, y;
	uint8_t type;
	sprite_t* sprite;
	struct bg_s* next;
	bool toggle;
} bg_t;

static sprite_t* sign_sprite[SIGNS_COUNT];
static sprite_t* floor_bottom_sprite;
static sprite_t* floor_dark_sprite;
static sprite_t* floor_light_sprite;
static surface_t floor_bottom_surf;
static surface_t floor_dark_surf;
static surface_t floor_light_surf;
static sprite_t* panel_corner_top;
static sprite_t* panel_corner_bot;
static sprite_t* panel_bg;
static sprite_t* panel_rope;
static surface_t panel_corner_top_surf;
static surface_t panel_corner_bot_surf;
static surface_t panel_bg_surf;
static surface_t panel_rope_surf;

uint8_t sign_y_offset = 0;

// Speeds based on the stage of the game
static const float bg_speeds[NOTE_SPEED_COUNT] = {
	1.0,
	1.5,
	2.0,
};

// Speeds based on the layer
static const float bg_layer_speeds[BACKGROUND_HILLS_COUNT] = {
	0.5,
	0.75,
	1.0,
	0.5625
};

static uint8_t bg_y_offsets[BACKGROUND_HILLS_COUNT] = {
	BACKGROUND_SKY_HEIGHT - BACKGROUND_HILL_BACK_HEIGHT,
	BACKGROUND_SKY_HEIGHT - BACKGROUND_HILL_BACK_HEIGHT,
	BACKGROUND_SKY_HEIGHT - BACKGROUND_HILL_FRONT_HEIGHT,
	BACKGROUND_SKY_HEIGHT - BACKGROUND_HILL_CLOUD_HEIGHT,
};

static float floor_boards_vertices[FLOOR_BOARDS_COUNT];
static bg_t* hills[BACKGROUND_HILLS_COUNT];

void ui_bg_add (bg_t* current) {
	current->next = malloc(sizeof(bg_t));
	current->next->sprite = current->sprite;
	current->next->x = current->x + current->sprite->width;
	current->next->y = BACKGROUND_SKY_HEIGHT - current->sprite->height - bg_y_offsets[current->type];
	current->next->toggle = !current->toggle;
	current->next->type = current->type;
	current->next->next = NULL;
	if (current->type == BACKGROUND_HILLS_CLOUDS) {
		bg_y_offsets[BACKGROUND_HILLS_CLOUDS] = BACKGROUND_SKY_HEIGHT - BACKGROUND_HILL_CLOUD_HEIGHT + (rand() % BACKGROUND_HILL_CLOUD_RAND);
		current->next->x += (rand() % (BACKGROUND_HILL_CLOUD_RAND*3));
	}
}

void ui_init (void) {
	char temptext[64];
	bg_t* current;
	// Load sprites
	floor_bottom_sprite = sprite_load("rom:/hydraharmonics/floor-bottom.ci4.sprite");
	floor_bottom_surf = sprite_get_pixels(floor_bottom_sprite);
	floor_dark_sprite = sprite_load("rom:/hydraharmonics/floor-dark.ci4.sprite");
	floor_dark_surf = sprite_get_pixels(floor_dark_sprite);
	floor_light_sprite = sprite_load("rom:/hydraharmonics/floor-light.ci4.sprite");
	floor_light_surf = sprite_get_pixels(floor_light_sprite);
	for (uint8_t i=0; i<BACKGROUND_HILLS_COUNT; i++) {
		sprintf(temptext, "rom:/hydraharmonics/hill-%i.ci4.sprite", i);
		hills[i] = malloc(sizeof(bg_t));
		hills[i]->sprite = sprite_load(temptext);
		hills[i]->toggle = rand() % 2;
		hills[i]->type = i;
		hills[i]->x = 0 - (rand() % (hills[i]->sprite->width/2));
		hills[i]->y = BACKGROUND_SKY_HEIGHT - hills[i]->sprite->height - bg_y_offsets[i];

		current = hills[i];
		while (current->x + current->sprite->width < display_get_width()) {
			ui_bg_add(current);
			current = current->next;
		}
	}
	for (uint8_t i=0; i<SIGNS_COUNT; i++) {
		sprintf(temptext, "rom:/hydraharmonics/sign-%i.ci4.sprite", i);
		sign_sprite[i] = sprite_load(temptext);
	}
	panel_corner_top = sprite_load("rom:/hydraharmonics/panel-corner-top.ci4.sprite");
	panel_corner_bot = sprite_load("rom:/hydraharmonics/panel-corner-bot.ci4.sprite");
	panel_bg = sprite_load("rom:/hydraharmonics/panel-bg.ci4.sprite");
	panel_rope = sprite_load("rom:/hydraharmonics/panel-rope.ci4.sprite");
	panel_corner_top_surf = sprite_get_pixels(panel_corner_top);
	panel_corner_bot_surf = sprite_get_pixels(panel_corner_bot);
	panel_bg_surf = sprite_get_pixels(panel_bg);
	panel_rope_surf = sprite_get_pixels(panel_rope);

	// Designate the floor boards' position
	for (uint8_t i=0; i<FLOOR_BOARDS_COUNT; i++) {
		floor_boards_vertices[i] = i * (FLOOR_BOARDS_WIDTH) * 2;
	}
}

void ui_animate (void) {
	// Shift the floor boards along
	for (uint8_t i=0; i<FLOOR_BOARDS_COUNT; i++) {
		floor_boards_vertices[i] -= bg_speeds[game_speed];
	}

	// Shift the backgrounds along
	bg_t* current;
	for (uint8_t i=0; i<BACKGROUND_HILLS_COUNT; i++) {
		current = hills[i];
		while (current != NULL) {
			current->x -= bg_layer_speeds[i] * bg_speeds[game_speed];
			// Kill the first item in the list if it goes offscreen
			if (current->x <= current->sprite->width * -1 + 1) {
				hills[i] = current->next;
				free(current);
			}
			// Add a new item if the last one needs it
			if (current->next == NULL && current->x + current->sprite->width < display_get_width()) {
				ui_bg_add(current);
			}
			current = current->next;
		}
	}

	// Check if they need to be moved to the right
	if (floor_boards_vertices[0] <= FLOOR_BOARDS_LIMIT_LEFT) {
		for (uint8_t i=1; i<FLOOR_BOARDS_COUNT; i++) {
			floor_boards_vertices[i-1] = floor_boards_vertices[i];
		}
		floor_boards_vertices[FLOOR_BOARDS_COUNT-1] = floor_boards_vertices[FLOOR_BOARDS_COUNT-2] + (FLOOR_BOARDS_WIDTH*2);
	}
}

void ui_panel_draw (float x0, float y0, float x1, float y1) {
	// Load textures
	uint16_t tmem_bytes = 0;
	uint8_t repetitions_y = ((y1 - PANEL_BORDER_LONG) - (y0 + PANEL_BORDER_LONG)) / PANEL_BORDER_SHORT;
	uint8_t repetitions_x = ((x1 - PANEL_BORDER_LONG) - (x0 + PANEL_BORDER_LONG)) / PANEL_BORDER_SHORT;
	rdpq_set_mode_copy(true);
	rdpq_mode_tlut(TLUT_RGBA16);
	rdpq_tex_upload_tlut(
		sprite_get_palette(panel_corner_top),
		0,
		CI4_PALETTE_SIZE
	);
	rdpq_tex_upload_tlut(
		sprite_get_palette(panel_corner_bot),
		CI4_PALETTE_SIZE * 1,
		CI4_PALETTE_SIZE
	);
	rdpq_tex_upload_tlut(
		sprite_get_palette(panel_bg),
		CI4_PALETTE_SIZE * 2,
		CI4_PALETTE_SIZE
	);
	rdpq_tex_upload_tlut(
		sprite_get_palette(panel_rope),
		CI4_PALETTE_SIZE * 3,
		CI4_PALETTE_SIZE
	);

	// Load the textures
	// Left/right side
	tmem_bytes += rdpq_tex_upload_sub(
		TILE0,
		&panel_corner_top_surf,
		&(rdpq_texparms_t){
			.palette = 0,
			.tmem_addr = tmem_bytes,
			.t.repeats = repetitions_y,
			.s.repeats = 2,
			.s.mirror = true,
		},
		0, PANEL_CORNER_HEIGHT - PANEL_BORDER_SHORT,
		PANEL_BORDER_LONG, PANEL_CORNER_HEIGHT
	);
	// Top side
	tmem_bytes += rdpq_tex_upload_sub(
		TILE1,
		&panel_corner_top_surf,
		&(rdpq_texparms_t){
			.palette = 0,
			.tmem_addr = tmem_bytes,
			.s.repeats = repetitions_x,
			.t.repeats = 2,
			.t.mirror = true,
		},
		PANEL_CORNER_WIDTH - PANEL_BORDER_SHORT, 0,
		PANEL_CORNER_WIDTH, PANEL_BORDER_LONG
	);
	// Top corners
	tmem_bytes += rdpq_tex_upload(
		TILE2,
		&panel_corner_top_surf,
		&(rdpq_texparms_t){
			.palette = 0,
			.tmem_addr = tmem_bytes,
			.s.repeats = 2,
			.s.mirror = true,
		}
	);
	// Bottom side
	tmem_bytes += rdpq_tex_upload_sub(
		TILE3,
		&panel_corner_bot_surf,
		&(rdpq_texparms_t){
			.palette = 1,
			.tmem_addr = tmem_bytes,
			.s.repeats = repetitions_x,
		},
		PANEL_CORNER_WIDTH - PANEL_BORDER_SHORT, PANEL_BORDER_LONG,
		PANEL_CORNER_WIDTH, PANEL_BORDER_LONG*2
	);
	// Bottom corners
	tmem_bytes += rdpq_tex_upload(
		TILE4,
		&panel_corner_bot_surf,
		&(rdpq_texparms_t){
			.palette = 1,
			.tmem_addr = tmem_bytes,
			.s.repeats = 2,
			.s.mirror = true,
		}
	);
	// Background
	tmem_bytes += rdpq_tex_upload(
		TILE5,
		&panel_bg_surf,
		&(rdpq_texparms_t){
			.palette = 2,
			.tmem_addr = tmem_bytes,
			.s.repeats = (x1 - x0) / panel_bg->width,
			.t.repeats = (y1 - y0) / panel_bg->height,
		}
	);
	// Rope
	tmem_bytes += rdpq_tex_upload_sub(
		TILE6,
		&panel_rope_surf,
		&(rdpq_texparms_t){
			.palette = 3,
			.tmem_addr = tmem_bytes,
			.t.repeats = (y0) / panel_rope->height,
		},
		0, 0,
		panel_rope->width, 4
	);

	// Draw the rectangles
	// Background
	rdpq_texture_rectangle(
		TILE5,
		x0,
		y0,
		x1,
		y1,
		0,0
	);
	// Left side
	rdpq_texture_rectangle(
		TILE0,
		x0,
		y0 + PANEL_CORNER_HEIGHT,
		x0 + PANEL_BORDER_LONG,
		y1 - PANEL_CORNER_HEIGHT,
		0, PANEL_BORDER_LONG
	);
	// Right side
	rdpq_texture_rectangle(
		TILE0,
		x1 - PANEL_BORDER_LONG,
		y0 + PANEL_CORNER_HEIGHT,
		x1,
		y1 - PANEL_CORNER_HEIGHT,
		-PANEL_BORDER_LONG, 0
	);
	// Top side
	rdpq_texture_rectangle(
		TILE1,
		x0 + PANEL_CORNER_WIDTH,
		y0,
		x1 - PANEL_CORNER_WIDTH,
		y0 + PANEL_BORDER_LONG,
		0, 0
	);
	// Bottom side
	rdpq_texture_rectangle(
		TILE3,
		x0 + PANEL_CORNER_WIDTH,
		y1 - PANEL_BORDER_LONG,
		x1 - PANEL_CORNER_WIDTH,
		y1,
		PANEL_CORNER_WIDTH - PANEL_BORDER_SHORT, PANEL_BORDER_LONG
	);
	// Corners
	// Top-left
	rdpq_texture_rectangle(
		TILE2,
		x0, y0, x0 + PANEL_CORNER_WIDTH, y0 + PANEL_CORNER_HEIGHT,
		0, 0
	);
	// Top-right
	rdpq_texture_rectangle(
		TILE2,
		x1 - PANEL_CORNER_WIDTH, y0, x1, y0 + PANEL_CORNER_HEIGHT,
		-PANEL_CORNER_WIDTH, 0
	);
	// Bottom-left
	rdpq_texture_rectangle(
		TILE4,
		x0, y1 - PANEL_CORNER_HEIGHT, x0 + PANEL_CORNER_WIDTH, y1,
		0, PANEL_CORNER_SPRITE_HEIGHT - PANEL_CORNER_HEIGHT
	);
	// Bottom-left
	rdpq_texture_rectangle(
		TILE4,
		x1 - PANEL_CORNER_WIDTH, y1 - PANEL_CORNER_HEIGHT, x1, y1,
		-PANEL_CORNER_WIDTH, PANEL_CORNER_SPRITE_HEIGHT - PANEL_CORNER_HEIGHT
	);
	// Rope
	rdpq_texture_rectangle(
		TILE6,
		x0 + PANEL_ROPE_X_OFFSET, 0, x0 + PANEL_ROPE_X_OFFSET + PANEL_ROPE_WIDTH, y0 - PANEL_ROPE_RING_HEIGHT,
		0, 0
	);
	rdpq_texture_rectangle(
		TILE6,
		x1 - PANEL_ROPE_X_OFFSET - PANEL_ROPE_WIDTH, 0, x1 - PANEL_ROPE_X_OFFSET, y0 - PANEL_ROPE_RING_HEIGHT,
		0, 0
	);
	rdpq_sprite_blit (
		panel_rope,
		x0 + PANEL_ROPE_X_OFFSET,
		y0 - panel_rope->height,
		NULL
	);
	rdpq_sprite_blit (
		panel_rope,
		x1 - PANEL_ROPE_X_OFFSET - PANEL_ROPE_WIDTH,
		y0 - panel_rope->height,
		NULL
	);
}

static void ui_bg_draw (void) {
	// Draw the background sky/lake rectangles
	rdpq_set_mode_fill(BACKGROUND_SKY_COLOR);
	rdpq_fill_rectangle(0, 0, display_get_width(), BACKGROUND_HILL_BACK_HEIGHT);
	rdpq_set_mode_fill(BACKGROUND_HILL_BACK_COLOR);
	rdpq_fill_rectangle(0, BACKGROUND_HILL_BACK_HEIGHT, display_get_width(), BACKGROUND_HILL_FRONT_HEIGHT);
	rdpq_set_mode_fill(BACKGROUND_HILL_FRONT_COLOR);
	rdpq_fill_rectangle(0, BACKGROUND_HILL_FRONT_HEIGHT, display_get_width(), BACKGROUND_SKY_HEIGHT);
	rdpq_set_mode_fill(BACKGROUND_LAKE_COLOR);
	rdpq_fill_rectangle(0, BACKGROUND_SKY_HEIGHT, display_get_width(), BACKGROUND_SKY_HEIGHT + BACKGROUND_LAKE_HEIGHT);

	// Draw the hills
	rdpq_set_mode_copy(true);
	bg_t* current;
	for (uint8_t i=0; i<BACKGROUND_HILLS_COUNT; i++) {
		current = hills[i];
		while (current != NULL) {
			if ((i!=BACKGROUND_HILLS_LAST || current->toggle) && (i != BACKGROUND_HILLS_CLOUDS || !current->toggle)) {
				rdpq_sprite_blit (
					current->sprite,
					current->x,
					current->y,
					NULL
				);
			}
			current = current->next;
		}
	}
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
			sign_sprite[i],
			hydras[i].x,
			display_get_height() - sign_y_offset,
			NULL
		);
	}

	// Pause text
	if (pause) {
		rdpq_text_printf(
			&(rdpq_textparms_t){
				.width = display_get_width(),
				.height = display_get_height(),
				.align = ALIGN_CENTER,
				.valign = VALIGN_CENTER,
				.char_spacing = 5,
			},
			FONT_CLARENDON,
			0,
			0,
			"PAUSE"
		);
	}
}

static void ui_bars_draw (void) {
	for (uint8_t i=0; i<BAR_COUNT; i++) {
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

void ui_bg_clear (bg_t* bg) {
	if (bg == NULL) {
		return;
	}
	ui_bg_clear (bg->next);
	free(bg);
}

void ui_clear (void) {
	sprite_free(floor_bottom_sprite);
	sprite_free(floor_dark_sprite);
	sprite_free(floor_light_sprite);
	surface_free(&floor_bottom_surf);
	surface_free(&floor_dark_surf);
	surface_free(&floor_light_surf);
	for (uint8_t i=0; i<BACKGROUND_HILLS_COUNT; i++) {
		sprite_free(hills[i]->sprite);
		ui_bg_clear(hills[i]);
	}
	for (uint8_t i=0; i<SIGNS_COUNT; i++) {
		sprite_free(sign_sprite[i]);
	}
	sprite_free(panel_corner_top);
	sprite_free(panel_corner_bot);
	sprite_free(panel_bg);
	sprite_free(panel_rope);
	surface_free(&panel_corner_top_surf);
	surface_free(&panel_corner_bot_surf);
	surface_free(&panel_bg_surf);
	surface_free(&panel_rope_surf);
}
