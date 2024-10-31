#include "hydra.h"
#include "logic.h"

#define HYDRA_HEAD_OFFSET_FIRST 10
#define HYDRA_HEAD_OFFSET_EACH 2
#define HYDRA_MOVEMENT_SPEED 8
#define HYDRA_HAT_X_OFFSET 5
#define HYDRA_HAT_SPEED 4
#define HYDRA_HAT_WIDTH 14
#define HYDRA_LEG_HEIGHT 16
#define HYDRA_LEG_X_OFFSET 5
#define HYDRA_SHELL_Y PADDING_TOP+4*HYDRA_ROW_HEIGHT
#define HYDRA_BOUNCE_PERIOD 20
#define HYDRA_BOUNCE_AMPLITUDE 4
#define SCALE 1

// Global vars
hydra_t hydras[PLAYER_MAX];
static const uint8_t collar_offsets[PLAYER_MAX] = {17, 13, 11, 7};
static sprite_t* hat_sprite;
static sprite_t* eye_sprite;
static sprite_t* leg_sprite;
static uint32_t shell_bounce = HYDRA_BOUNCE_AMPLITUDE * 2;

// DB of head animations
typedef enum {
	ANIMATION_DB_X,
	ANIMATION_DB_Y,
	ANIMATION_DB_LENGTH,
	ANIMATION_DB_FRAME_DURATION,
	ANIMATION_DB_X_OFFSET,
	ANIMATION_DB_NEXT,
	ANIMATION_DB_COUNT,
} animation_offsets_t;
static const int8_t animation_offsets[HYDRA_ANIMATION_COUNT][ANIMATION_DB_COUNT] = {
		{0,	0,	1,	1,	0,	HYDRA_ANIMATION_NONE},		// Idle
		{0,	0,	11,	1,	0,	HYDRA_ANIMATION_CLOSE},		// Open
		{0,	1,	11,	1,	0,	HYDRA_ANIMATION_CHEW},		// Close
		{0,	2,	4,	5,	-4,	HYDRA_ANIMATION_NONE},		// Chew
		{4,	2,	3,	50,	0,	HYDRA_ANIMATION_SLEEP_2},	// Sleep
		{5,	2,	1,	50,	0,	HYDRA_ANIMATION_SLEEP},		// Sleep
		{7,	2,	1,	60,	0,	HYDRA_ANIMATION_NONE},		// Stun
	};

// Array of hat offsets to determine the hight of the hat at each frame of animation
static const int8_t hat_offsets[3][11] = {
	{24, 20, 14,  2, -4, -8, -6, -4, -4, -4, -4},
	{ 2, 14, 20, 26, 26, 24, 24, 24, 24, 24, 24},
	{24, 24, 22, 22, 24, 22, 20,  8,  0,  0,  0},
};

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
		hydras[i].neck_surf = sprite_get_pixels(hydras[i].neck_sprite);

		// Set the variables
		hydras[i].pos = i;
		hydras[i].x = PADDING_LEFT + hydras[i].pos * HYDRA_HEAD_WIDTH;
		hydras[i].y = PADDING_TOP + hydras[i].state * HYDRA_HEAD_HEIGHT;
		hydras[i].hat_y = hydras[i].y + HYDRA_HEAD_HEIGHT/2;
		hydras[i].ai = i < core_get_playercount() ? 0 : (rand()%(HYDRA_AI_COUNT-1) + 1);
		hydras[i].animation = HYDRA_ANIMATION_NONE;
		hydras[i].frame = 0;
	}
	hat_sprite = sprite_load("rom:/hydraharmonics/hats.ci4.sprite");
	eye_sprite = sprite_load("rom:/hydraharmonics/eyes.ci4.sprite");
	leg_sprite = sprite_load("rom:/hydraharmonics/legs.ci4.sprite");
}

void hydra_adjust_hats (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		float min_y =
			hydras[i].y +
			hat_offsets [
				animation_offsets[hydras[i].animation][ANIMATION_DB_Y]
			] [
				(hydras[i].frame/animation_offsets[hydras[i].animation][ANIMATION_DB_FRAME_DURATION] +
				animation_offsets[hydras[i].animation][ANIMATION_DB_X])
			];

		hydras[i].hat_y = min_y < hydras[i].hat_y + HYDRA_HAT_SPEED ? min_y : hydras[i].hat_y + HYDRA_HAT_SPEED;
	}
}

void hydra_move (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		if (hydras[i].animation == HYDRA_ANIMATION_STUN) {
			continue;
		}
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
			hydra_ai (i);
		}
		// Adjust the Y position by moving it towards the selected row
		if (hydras[i].y < PADDING_TOP+hydras[i].state*HYDRA_ROW_HEIGHT){
			hydras[i].y += HYDRA_MOVEMENT_SPEED;
		} else if (hydras[i].y > PADDING_TOP+hydras[i].state*HYDRA_ROW_HEIGHT) {
			hydras[i].y -= HYDRA_MOVEMENT_SPEED;
		}
	}
}

void hydra_animate (PlyNum p, hydraharmonics_animations_t a) {
	hydras[p].animation = a;
	hydras[p].frame = 0;
}

void hydra_shell_bounce (void) {
	static uint8_t shell_timer = 0;
	// Create a pattern with abs() that causes both an upward and downward movement for the bounce
	shell_bounce =
		shell_timer % HYDRA_BOUNCE_PERIOD <= HYDRA_BOUNCE_AMPLITUDE ?
		abs(HYDRA_BOUNCE_AMPLITUDE/2- (shell_timer % HYDRA_BOUNCE_PERIOD)-HYDRA_BOUNCE_AMPLITUDE/2) : 0;
	shell_timer++;
}

void hydra_draw (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Draw the legs
		rdpq_sprite_blit (
			leg_sprite,
			hydras[i].x + collar_offsets[hydras[i].pos] - HYDRA_LEG_X_OFFSET,
			HYDRA_SHELL_Y + hydras[i].shell_sprite->height,
			&(rdpq_blitparms_t){
				.height = HYDRA_LEG_HEIGHT,
				.t0 = HYDRA_LEG_HEIGHT * i,
			}
		);

		// Draw the shells
		rdpq_sprite_blit (
			hydras[i].shell_sprite,
			hydras[i].x,
			HYDRA_SHELL_Y + shell_bounce,
			&(rdpq_blitparms_t){
				.width = 48,
				.s0 = 48 * hydras[i].pos,
			}
		);

		// Draw a hat
		rdpq_sprite_blit (
			hat_sprite,
			hydras[i].x + collar_offsets[hydras[i].pos] + HYDRA_HAT_X_OFFSET,
			hydras[i].hat_y,
			&(rdpq_blitparms_t){
				.width = HYDRA_HAT_WIDTH,
				.s0 = HYDRA_HAT_WIDTH * i,
			}
		);

		if (hydras[i].state != STATE_HIDE) {
			// Draw the heads
			rdpq_sprite_blit (
				hydras[i].head_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos] + animation_offsets[hydras[i].animation][ANIMATION_DB_X_OFFSET],
				hydras[i].y,
				&(rdpq_blitparms_t){
					.width = HYDRA_HEAD_WIDTH,
					.height = HYDRA_HEAD_HEIGHT,
					.s0 = HYDRA_HEAD_WIDTH
						* (hydras[i].frame/animation_offsets[hydras[i].animation][ANIMATION_DB_FRAME_DURATION]
							+ animation_offsets[hydras[i].animation][ANIMATION_DB_X]),
					.t0 = HYDRA_HEAD_HEIGHT * animation_offsets[hydras[i].animation][ANIMATION_DB_Y],
				}
			);

			// Handle the animation frame
			if (++hydras[i].frame == animation_offsets[hydras[i].animation][ANIMATION_DB_LENGTH] * animation_offsets[hydras[i].animation][ANIMATION_DB_FRAME_DURATION]) {
				hydras[i].animation = animation_offsets[hydras[i].animation][ANIMATION_DB_NEXT];
				hydras[i].frame = 0;
			}

			// Draw the collar
			rdpq_sprite_blit (
				hydras[i].neck_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos],
				HYDRA_SHELL_Y + shell_bounce,
				NULL
			);

			// Draw the neck
			rdpq_tex_upload_sub(
				TILE0,
				&hydras[i].neck_surf,
				&(rdpq_texparms_t){
					.t.repeats = (2-hydras[i].state) * HYDRA_ROW_HEIGHT/2,
				},
				0, 0,
				hydras[i].neck_sprite->width, 2
			);
			rdpq_texture_rectangle(
				TILE0,
				hydras[i].x + collar_offsets[hydras[i].pos],
				hydras[i].y + HYDRA_HEAD_HEIGHT,
				hydras[i].x + collar_offsets[hydras[i].pos] + hydras[i].neck_sprite->width,
				HYDRA_SHELL_Y + shell_bounce,
				0, 0);
		} else {
			// Hydra is hiding
			// Draw the eyes
			rdpq_sprite_blit (
				eye_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos],
				HYDRA_SHELL_Y,
				NULL
			);
		}
	}
}



void hydra_clear (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		sprite_free(hydras[i].head_sprite);
		sprite_free(hydras[i].shell_sprite);
		sprite_free(hydras[i].neck_sprite);
		surface_free(&hydras[i].neck_surf);
	}
	sprite_free(eye_sprite);
	sprite_free(hat_sprite);
	sprite_free(leg_sprite);
}
