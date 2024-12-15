#include "hydra.h"
#include "logic.h"
#include "effects.h"
#include "audio.h"

#define HYDRA_HEAD_OFFSET_FIRST 10
#define HYDRA_HEAD_OFFSET_EACH 2
#define HYDRA_MOVEMENT_SPEED 8
#define HYDRA_SWAP_SPEED (HYDRA_MOVEMENT_SPEED / 2.0f)
#define HYDRA_SHELL_Y (PADDING_TOP+4*HYDRA_ROW_HEIGHT)
#define HYDRA_HAT_X_OFFSET 5
#define HYDRA_HAT_SPEED 4
#define HYDRA_HAT_WIDTH 14
#define HYDRA_HAT_HEIGHT 8
#define HYDRA_HAT_MAX_Y (HYDRA_SHELL_Y-HYDRA_HAT_HEIGHT)
#define HYDRA_BOUNCE_PERIOD 20
#define HYDRA_BOUNCE_AMPLITUDE 4
#define HYDRA_SHELL_WIDTH HYDRA_HEAD_WIDTH

#define HYDRA_LEG_WIDTH 56
#define HYDRA_LEG_HEIGHT 22
#define HYDRA_LEG_X_OFFSET -15
#define HYDRA_LEG_STANDING_FRAME 8
#define UPDATE_LEG_OFFSET(i) \
    (hydras[i].leg_offset_y = (hydras[i].leg_offset_y >= HYDRA_LEG_HEIGHT) ? hydras[i].leg_offset_y : hydras[i].leg_offset_y + 1)

#define HYDRA_TOPBACK_WIDTH 12
#define HYDRA_TOPBACK_HEIGHT 8

#define HYDRA_FACE_WIDTH 28
#define HYDRA_FACE_HEIGHT 64
#define HYDRA_FACE_COLS 11
#define HYDRA_FACE_ROWS 4

#define HYDRA_EYE_WIDTH 14
#define HYDRA_EYE_HEIGHT 16
#define HYDRA_EYE_OFFSET_X -3
#define HYDRA_EYE_OFFSET_Y 3
#define HYDRA_EYE_COLS 5
#define HYDRA_EYE_ROWS 5

#define HYDRA_EYELID_WIDTH 9
#define HYDRA_EYELID_HEIGHT 8

#define HYDRA_STICK_THRESHOLD 50

// Global vars
hydra_t hydras[PLAYER_MAX];
static const uint8_t collar_offsets[PLAYER_MAX] = {17, 13, 11, 7};
static sprite_t* hat_sprite;
static uint32_t shell_bounce = 0;
static uint32_t shell_frame = 0;


#include "hydra_animation.h"

// Array of hat offsets to determine the hight of the hat at each frame of animation
static const int8_t hat_offsets[HYDRA_FACE_ROWS][HYDRA_FACE_COLS] = {
	{24, 20, 14,  2, -4, -8, -6, -4, -4, -4, -4},
	{ 2, 14, 20, 26, 26, 24, 24, 24, 24, 24, 24},
	{24, 24, 24, 24, 24, 24, 24, 24,  8, 24, 48},
	{56, 22, 24, 24, 24, 24, 24, 24, 24, 24, 24},
};

void hydra_calculate_x (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		hydras[i].x = PADDING_LEFT + hydras[i].pos * HYDRA_HEAD_WIDTH;
	}
}

PlyNum hydra_get_from_pos (uint8_t pos) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		if (hydras[i].pos == pos) {
			return i;
		}
	}
	return 0;
}

void hydra_init (void) {
	char temptext[64];
	uint8_t i, random, temp;
	for (i=0; i<PLAYER_MAX; i++) {
		// Load the sprites
		sprintf(temptext, "rom:/hydraharmonics/face-%i.ci4.sprite", i);
		hydras[i].face_sprite = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/eyes-%i.ci4.sprite", i);
		hydras[i].eyes_sprite = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/topback-%i.ci4.sprite", i);
		hydras[i].topback_sprite = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/shell-%i.ci4.sprite", i);
		hydras[i].shell_sprite = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/neck-%i.ci4.sprite", i);
		hydras[i].neck_sprite = sprite_load(temptext);
		hydras[i].neck_surf = sprite_get_pixels(hydras[i].neck_sprite);
		sprintf(temptext, "rom:/hydraharmonics/legs-%i.ci4.sprite", i);
		hydras[i].leg_sprite = sprite_load(temptext);
		// Flairs
		hydras[i].flair_sprite[FLAIR_NONE] = NULL;
		sprintf(temptext, "rom:/hydraharmonics/cheek-%i.ci4.sprite", i);
		hydras[i].flair_sprite[FLAIR_CHEEK] = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/grin-%i.ci4.sprite", i);
		hydras[i].flair_sprite[FLAIR_GRIN] = sprite_load(temptext);
		sprintf(temptext, "rom:/hydraharmonics/eyelid-%i.ci4.sprite", i);
		hydras[i].flair_sprite[FLAIR_EYELID_0] = sprite_load(temptext);
		hydras[i].flair_sprite[FLAIR_EYELID_1] = hydras[i].flair_sprite[FLAIR_EYELID_0];
		hydras[i].flair_sprite[FLAIR_EYELID_2] = hydras[i].flair_sprite[FLAIR_EYELID_0];

		// Set the variables
		hydras[i].state = STATE_MID;
		hydras[i].pos = i;
		hydras[i].y = PADDING_TOP + hydras[i].state * HYDRA_ROW_HEIGHT;
		hydras[i].hat_y = hydras[i].y + HYDRA_HEAD_HEIGHT/2;
		hydras[i].leg_offset_y = 0;
		hydras[i].animation = HYDRA_ANIMATION_NONE;
		hydras[i].last_eaten = NOTES_TYPE_STANDARD;
		hydras[i].frame = hydras[i].face_frame = 0;
	}
	// Common sprites
	hat_sprite = sprite_load("rom:/hydraharmonics/hats.ci4.sprite");

	// Shuffle starting positions
	for (i=0; i<PLAYER_MAX; i++) {
		random = rand() % (PLAYER_MAX-i);
		if (random && PLAYER_MAX-1) {
			temp = hydras[i].pos;
			hydras[i].pos = hydras[i+random].pos;
			hydras[i+random].pos = temp;
		}
		hydras[i].shell_pos = hydras[i].pos;
	}
	for (i=0; i<PLAYER_MAX; i++) {

	}

	// Calculate the starting positions
	hydra_calculate_x();
}

int8_t hydra_get_hat_offset (PlyNum i) {
	return hat_offsets [
		(animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].face_sprite / HYDRA_FACE_COLS)
	] [
		(animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].face_sprite % HYDRA_FACE_COLS)
	];
}

void hydra_adjust_hats (void) {
	if (pause) {
		return;
	}
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		float min_y = hydras[i].y + hydra_get_hat_offset (i);
		hydras[i].hat_y = min_y < hydras[i].hat_y + HYDRA_HAT_SPEED ? min_y : hydras[i].hat_y + HYDRA_HAT_SPEED;
	}
}

bool hydra_get_all_animation_states (hydraharmonics_animations_t a) {
	bool all_correct = true;
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		if (hydras[i].animation != a) {
			all_correct = false;
			break;
		}
	}
	return all_correct;
}

void hydra_swap_animation (hydraharmonics_animations_t stage, PlyNum i) {
	if (pause) {
		return;
	}
	if (stage == HYDRA_ANIMATION_SWAP_DOWN) {
		UPDATE_LEG_OFFSET(i);
		if (hydras[i].y + HYDRA_HEAD_HEIGHT >= HYDRA_SHELL_Y) {
			hydra_animate (i, HYDRA_ANIMATION_SWAP_WAIT);
		} else {
			hydras[i].y += HYDRA_SWAP_SPEED;
		}
	} else if (stage == HYDRA_ANIMATION_SWAP_WAIT) {
		UPDATE_LEG_OFFSET(i);
		// Find another hydra in this state
		for (uint8_t j=0; j<PLAYER_MAX; j++) {
			if (hydras[j].animation == HYDRA_ANIMATION_SWAP_WAIT && j != i) {
				// Check if there's anyone else moving downwards
				uint8_t players_to_swap = 0;
				for (uint8_t k=0; k<PLAYER_MAX; k++) {
					if (
						hydras[k].animation == HYDRA_ANIMATION_SWAP_DOWN ||
						hydras[k].animation == HYDRA_ANIMATION_OPEN_TO_SWAP ||
						hydras[k].animation == HYDRA_ANIMATION_CLOSE_TO_SWAP ||
						hydras[k].animation == HYDRA_ANIMATION_CHEW_TO_SWAP
					) {
						// We're waiting for someone.
						return;
					} else if (hydras[k].animation == HYDRA_ANIMATION_SWAP_WAIT) {
						players_to_swap++;
					}
				}
				// Prepare the arrays
				uint8_t* temp_hyd = malloc(players_to_swap * sizeof(uint8_t));
				uint8_t* temp_pos = malloc(players_to_swap * sizeof(uint8_t));
				uint8_t temp = 0;
				for (uint8_t k=0; k<PLAYER_MAX; k++) {
					if (hydras[k].animation == HYDRA_ANIMATION_SWAP_WAIT) {
						temp_hyd[temp] = k;
						temp_pos[temp] = hydras[k].pos;
						temp++;
					}
				}
				// Do the swap
				temp = hydras[temp_hyd[0]].pos;
				for (uint8_t k=0; k<players_to_swap-1; k++) {
					hydras[temp_hyd[k]].pos = hydras[temp_hyd[k+1]].pos;
				}
				hydras[temp_hyd[players_to_swap-1]].pos = temp;
				// Set the default post-swap variables
				for (uint8_t k=0; k<players_to_swap; k++) {
					hydra_animate (temp_hyd[k], HYDRA_ANIMATION_SWAP_UP);
					hydras[temp_hyd[k]].state = STATE_MID;
				}
				// Set things back to normal
				hydra_calculate_x();
				free(temp_hyd);
				free(temp_pos);
				audio_sfx_play(SFX_SLIDE_WHISTLE_UP);
				break;
			}
		}
	} else if (stage == HYDRA_ANIMATION_SWAP_UP) {
		hydras[i].leg_offset_y = !hydras[i].leg_offset_y ? hydras[i].leg_offset_y : hydras[i].leg_offset_y - 1;
		if (hydras[i].y > PADDING_TOP+hydras[i].state*HYDRA_ROW_HEIGHT) {
			hydras[i].y -= HYDRA_SWAP_SPEED;
		} else {
			hydra_animate (i, HYDRA_ANIMATION_NONE);
			hydras[i].leg_offset_y = 0;
		}
	}
}

void hydra_move (void) {
	static bool hold_check[PLAYER_MAX] = {false};
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Handle all the states
		if (hydras[i].animation == HYDRA_ANIMATION_STUN) {
			continue;
		} else if (hydras[i].animation >= HYDRA_ANIMATION_SWAP_DOWN && hydras[i].animation <= HYDRA_ANIMATION_SWAP_WAIT) {
			hydra_swap_animation (hydras[i].animation, i);
			continue;
		}
		if (i < core_get_playercount()) {
			// Player character movement
			joypad_inputs_t joypad = joypad_get_inputs(core_get_playercontroller(i));

			// Pause the game
			if (joypad.btn.start) {
				if (!hold_check[i] ) {
					if (pause) {
						audio_music_play();
					} else {
						audio_music_stop();
					}
					pause ^= 1;
				}
				hold_check[i] = true;
			} else {
				hold_check[i] = false;
			}

			if (pause) {
				continue;
			}

			if (hydras[i].animation == HYDRA_ANIMATION_NONE) {
				if (joypad.btn.a || joypad.btn.l || joypad.btn.r || joypad.btn.z) {
					audio_sfx_play(SFX_AAH_MIN + i);
					hydra_animate (i, HYDRA_ANIMATION_OPEN);
				}
				if (joypad.btn.d_up || joypad.btn.c_up || joypad.stick_y > HYDRA_STICK_THRESHOLD) {
					hydras[i].state = STATE_UP;
				} else if (joypad.btn.d_down || joypad.btn.c_down || joypad.stick_y < -HYDRA_STICK_THRESHOLD) {
					hydras[i].state = STATE_DOWN;
				} else {
					hydras[i].state = STATE_MID;
				}
			}
		} else {
			if (pause) {
				return;
			}
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

void hydra_swap_start (PlyNum swap_player, notes_types_t note_type) {
	PlyNum player_b;
	// Find player 'b'
	if (note_type == NOTES_TYPE_SWAP_FORWARD) {
		// Swap with next player in line
		player_b = hydra_get_from_pos((hydras[swap_player].pos + 1) % PLAYER_MAX);
	} else if (note_type == NOTES_TYPE_SWAP_RANDOM) {
		// Swap with random player
		player_b = hydra_get_from_pos((hydras[swap_player].pos + 1 + (rand() % (PLAYER_MAX-1))) % PLAYER_MAX);
	} else if (note_type == NOTES_TYPE_SWAP_BACK) {
		// Swap with player behind you
		player_b = hydra_get_from_pos((hydras[swap_player].pos + PLAYER_MAX-1) % PLAYER_MAX);
	} else if (note_type == NOTES_TYPE_SWAP_ALL) {
		// Swap with all players
		for (uint8_t i=0; i<PLAYER_MAX; i++) {
			if (i != swap_player) {
				hydra_animate (i, HYDRA_ANIMATION_SWAP_DOWN);
			}
		}
		audio_sfx_play(SFX_SLIDE_WHISTLE_DOWN);
		return;
	} else if (note_type == NOTES_TYPE_SOUR) {
		// Sour note, swap with the player in the parameter instead
		player_b = swap_player;
	} else {
		return;
	}

	audio_sfx_play(SFX_SLIDE_WHISTLE_DOWN);
	hydra_animate (player_b, HYDRA_ANIMATION_SWAP_DOWN);
}

void hydra_animate (PlyNum p, hydraharmonics_animations_t a) {
	hydras[p].animation = a;
	hydras[p].face_frame = 0;
	hydras[p].frame = 0;
}

void hydra_shell_bounce (void) {
	static uint8_t shell_timer = 0;
	// Create a pattern with abs() that causes both an upward and downward movement for the bounce
	shell_bounce = abs((shell_timer % 32) / 4 - 4);
	shell_frame = (shell_timer % 32) / 4;
	shell_timer += 1 + game_speed/2;
}

void hydra_handle_animation (PlyNum p) {
	if (pause) {
		return;
	}

	hydras[p].frame++;
	uint8_t frame_cum = 0;
	uint8_t skip = 0;

	if (hydras[p].frame == 1) {
		// Check if we need to add an effect
		if (hydras[p].animation == HYDRA_ANIMATION_CHEW) {
			effects_add(
				p,
				hydras[p].last_eaten == NOTES_TYPE_SWEET ? EFFECT_FLOWER :
					(shell_frame%2 ? EFFECT_NOTE_BIG : EFFECT_NOTE_SMALL),
				hydras[p].x + HYDRA_CHEW_EFFECT_OFFSET_X,
				hydras[p].y + hydra_get_hat_offset(p) + HYDRA_CHEW_EFFECT_OFFSET_Y
			);
		}

		// Check if we need the gulp sfx
		if (
			hydras[p].animation == HYDRA_ANIMATION_CLOSE_SUCCESS ||
			hydras[p].animation == HYDRA_ANIMATION_CLOSE_TO_SWAP ||
			hydras[p].animation == HYDRA_ANIMATION_CLOSE_TO_DIZZY
		) {
			audio_sfx_play(SFX_GULP_MIN + p);
		}

		// Check if we need the eww sfx
		if (
			hydras[p].animation == HYDRA_ANIMATION_DIZZY
		) {
			audio_sfx_play(SFX_EWW_MIN + p);
		}
	}

	// Count to see if we've reached the end of the animation cycle
	for (uint8_t i = 0; i<animation_db[hydras[p].animation].animation_length; i++) {
		frame_cum += animation_db[hydras[p].animation].frame_array[i].frame_duration;
		if (hydras[p].frame < frame_cum && !skip) {
			hydras[p].face_frame = i;
			// Don't update the face_frame anumore because we want to get the total numer of frames for this animation
			skip = 1;
		}
	}

	if (hydras[p].frame == frame_cum) {
		hydras[p].animation = animation_db[hydras[p].animation].next;
		hydras[p].frame = hydras[p].face_frame = 0;
	}
}

void hydra_draw (void) {
	// Draw bottom layer
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Draw the legs
		// Make sure we draw them in the right order, from left to right
		PlyNum p = hydra_get_from_pos(i);
		rdpq_sprite_blit (
			hydras[p].leg_sprite,
			hydras[p].x + collar_offsets[hydras[p].pos] + HYDRA_LEG_X_OFFSET,
			HYDRA_SHELL_Y + hydras[p].shell_sprite->height - hydras[p].leg_offset_y,
			&(rdpq_blitparms_t){
				.width = HYDRA_LEG_WIDTH,
				.s0 = stage == STAGE_GAME && hydras[p].leg_offset_y < HYDRA_LEG_HEIGHT/2 ?
					shell_frame * HYDRA_LEG_WIDTH : HYDRA_LEG_STANDING_FRAME * HYDRA_LEG_WIDTH,
			}
		);
	}

	// Draw middle layer
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Draw the shells
		rdpq_sprite_blit (
			hydras[i].shell_sprite,
			PADDING_LEFT + hydras[i].shell_pos * HYDRA_HEAD_WIDTH,
			HYDRA_SHELL_Y + shell_bounce,
			&(rdpq_blitparms_t){
				.width = HYDRA_SHELL_WIDTH,
				.s0 = HYDRA_SHELL_WIDTH * hydras[i].shell_pos,
			}
		);
	}

	// Draw top layer
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		// Draw a hat
		if (hydras[i].animation != HYDRA_ANIMATION_SWAP_WAIT || hydras[i].hat_y != HYDRA_HAT_MAX_Y)
		rdpq_sprite_blit (
			hat_sprite,
			hydras[i].x + collar_offsets[hydras[i].pos] + HYDRA_HAT_X_OFFSET,
			hydras[i].hat_y,
			&(rdpq_blitparms_t){
				.width = HYDRA_HAT_WIDTH,
				.s0 = HYDRA_HAT_WIDTH * i,
			}
		);

		if (hydras[i].animation != HYDRA_ANIMATION_SWAP_WAIT) {

			// Draw the face
			rdpq_sprite_blit (
				hydras[i].face_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos] + HYDRA_TOPBACK_WIDTH,
				hydras[i].y,
				&(rdpq_blitparms_t){
					.width = HYDRA_FACE_WIDTH,
					.height = HYDRA_FACE_HEIGHT,
					.s0 = HYDRA_FACE_WIDTH * (animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].face_sprite % HYDRA_FACE_COLS),
					.t0 = HYDRA_FACE_HEIGHT * (animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].face_sprite / HYDRA_FACE_COLS),
				}
			);

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
					.t.repeats = 128,
				},
				0, 0,
				hydras[i].neck_sprite->width, 2
			);
			// Full neck
			rdpq_texture_rectangle(
				TILE0,
				hydras[i].x + collar_offsets[hydras[i].pos],
				hydras[i].y + HYDRA_HEAD_HEIGHT,
				hydras[i].x + collar_offsets[hydras[i].pos] + hydras[i].neck_sprite->width,
				HYDRA_SHELL_Y + shell_bounce,
				0, 0);
			// Back of the head
			rdpq_texture_rectangle(
				TILE0,
				hydras[i].x + collar_offsets[hydras[i].pos],
				hydras[i].y + hydra_get_hat_offset (i) + HYDRA_HAT_HEIGHT + HYDRA_TOPBACK_HEIGHT,
				hydras[i].x + collar_offsets[hydras[i].pos] + hydras[i].neck_sprite->width/2,
				hydras[i].y + HYDRA_HEAD_HEIGHT,
				0, 0);

			// Topback of the head
			rdpq_sprite_blit (
				hydras[i].topback_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos],
				hydras[i].y + hydra_get_hat_offset (i) + HYDRA_HAT_HEIGHT,
				NULL
			);

			// Draw the eye
			rdpq_sprite_blit (
				hydras[i].eyes_sprite,
				hydras[i].x + collar_offsets[hydras[i].pos] + HYDRA_TOPBACK_WIDTH + HYDRA_EYE_OFFSET_X,
				hydras[i].y + hydra_get_hat_offset (i) + HYDRA_HAT_HEIGHT + HYDRA_EYE_OFFSET_Y,
				&(rdpq_blitparms_t){
					.width = HYDRA_EYE_WIDTH,
					.height = HYDRA_EYE_HEIGHT,
					.s0 = HYDRA_EYE_WIDTH * (animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].eyes_sprite % HYDRA_EYE_COLS),
					.t0 = HYDRA_EYE_HEIGHT * (animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].eyes_sprite / HYDRA_EYE_COLS),
				}
			);

			// Draw the flair
			hydra_flair_frames_t flair = animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].flair_sprite;
			if (flair) {
				rdpq_blitparms_t flair_parms =
					flair == FLAIR_EYELID_0 ||
					flair == FLAIR_EYELID_1 ||
					flair == FLAIR_EYELID_2
				?
					(rdpq_blitparms_t){
						.width = HYDRA_EYELID_WIDTH,
						.s0 = HYDRA_EYELID_WIDTH * (flair - FLAIR_EYELID),
				} : (rdpq_blitparms_t){0};
				rdpq_sprite_blit (
					hydras[i].flair_sprite[flair],
					hydras[i].x + collar_offsets[hydras[i].pos] + animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].flair_x,
					hydras[i].y + hydra_get_hat_offset (i) + HYDRA_HAT_HEIGHT + animation_db[hydras[i].animation].frame_array[hydras[i].face_frame].flair_y,
					&flair_parms
				);
			}

			// Handle the animation frame
			hydra_handle_animation(i);

		}
	}
/*
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		rdpq_text_printf(NULL, FONT_DEFAULT, hydras[i].x, HYDRA_SHELL_Y,
			"%.0f : %i", hydras[i].hat_y, HYDRA_HAT_MAX_HEIGHT
		);
	}
*/
}

void hydra_clear (void) {
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		sprite_free(hydras[i].face_sprite);
		sprite_free(hydras[i].eyes_sprite);
		sprite_free(hydras[i].topback_sprite);
		sprite_free(hydras[i].shell_sprite);
		sprite_free(hydras[i].neck_sprite);
		sprite_free(hydras[i].leg_sprite);
		surface_free(&hydras[i].neck_surf);
		sprite_free(hydras[i].flair_sprite[FLAIR_CHEEK]);
		sprite_free(hydras[i].flair_sprite[FLAIR_GRIN]);
		sprite_free(hydras[i].flair_sprite[FLAIR_EYELID]);

	}
	sprite_free(hat_sprite);
}
