#include "outro.h"
#include "hydra.h"
#include "audio.h"

#define HYDRA_OUTRO_SHUFFLE_TIME 100
#define HYDRA_OUTRO_WINNER_DROP_TIME 25
#define HYDRA_OUTRO_ANNOUNCE_TIME 80
#define HYDRA_OUTRO_CHOMP_ANIMATION_TIME 40

#define SIGN_SPEED_X 2
#define SIGN_SPEED_Y 2
#define SIGN_PADDING_BIG_X 15
#define SIGN_PADDING_BIG_Y 35
#define SIGN_PADDING_SMALL_X SIGN_PADDING_BIG_X
#define SIGN_PADDING_SMALL_Y 20


static sprite_t* sign_big_sprite;
static sprite_t* sign_small_sprite;

uint32_t outro_timer = 0;
uint8_t sign_x_offset = 0;

void outro_init (void) {
	sign_big_sprite = sprite_load("rom:/hydraharmonics/sign-big.ci4.sprite");
	sign_small_sprite = sprite_load("rom:/hydraharmonics/sign-small.ci4.sprite");
}

void outro_interact (void) {
	outro_timer++;
	// Move the signs left
	if (sign_x_offset <= sign_big_sprite->width) {
		sign_x_offset += SIGN_SPEED_X;
	}
	// Move the winners sign down
	if (outro_timer >= HYDRA_OUTRO_SHUFFLE_TIME) {
		for (uint8_t i=0; i<winners->length; i++) {
			if (winners->y_offset[i] < sign_small_sprite->height * (i+1)) {
				winners->y_offset[i] += SIGN_SPEED_Y;
			} else {
				winners->y_offset[i] = sign_small_sprite->height * (i+1);
			}

		}
	}
	// And the game
	if (outro_timer >= HYDRA_OUTRO_SHUFFLE_TIME + HYDRA_OUTRO_ANNOUNCE_TIME + HYDRA_OUTRO_WINNER_DROP_TIME * (winners->length+1)) {
		stage = STAGE_RETURN_TO_MENU;
	}
	// Animate the hydras
	if (outro_timer < HYDRA_OUTRO_SHUFFLE_TIME) {
		for (uint8_t i=0; i<PLAYER_MAX; i++) {
			hydra_animate (i, HYDRA_ANIMATION_STUN_LOOP);
		}
	} else {
		if (outro_timer == HYDRA_OUTRO_SHUFFLE_TIME) {
			audio_sfx_play(SFX_WINNER);
		}
		for (uint8_t i=0; i<PLAYER_MAX; i++) {
			if (score_is_winner(i)) {
				if (outro_timer < HYDRA_OUTRO_SHUFFLE_TIME + HYDRA_OUTRO_CHOMP_ANIMATION_TIME && hydras[i].animation != HYDRA_ANIMATION_OPEN) {
					hydra_animate (i, HYDRA_ANIMATION_OPEN);
				} else if (hydras[i].animation == HYDRA_ANIMATION_NONE) {
					hydra_animate (i, HYDRA_ANIMATION_GRIN);
				}

			} else if (!score_is_winner(i) && hydras[i].animation != HYDRA_ANIMATION_LOSER) {
				hydra_animate (i, HYDRA_ANIMATION_LOSER);
			}
		}
	}
}

void outro_results_draw (scores_results_draw_t type) {
	char scores_buffer[64] = "";
	char mini_buffer[32];

	// Get the player scores
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		sprintf(mini_buffer, "P %i: %i\n", i+1, type == SCORES_RESULTS_SHUFFLE ? rand()%100 : scores_get(i));
		strcat(scores_buffer, mini_buffer);
	}

	// Print it out
	rdpq_text_printf(
		NULL,
		FONT_CLARENDON,
		display_get_width() - sign_x_offset + SIGN_PADDING_BIG_X,
		SIGN_PADDING_BIG_Y,
		"Hats off to...\n%s", scores_buffer
	);
}

void outro_sign_draw (void) {
	// Draw the little signs
	for (int8_t i=winners->length-1; i>=0; i--) {
		rdpq_set_mode_standard();
		rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
		rdpq_sprite_blit (
			sign_small_sprite,
			display_get_width() - sign_x_offset,
			sign_big_sprite->height - sign_small_sprite->height + winners->y_offset[i],
			NULL
		);
		rdpq_text_printf(
			NULL,
			FONT_CLARENDON,
			display_get_width() - sign_x_offset + SIGN_PADDING_SMALL_X,
			sign_big_sprite->height - sign_small_sprite->height + winners->y_offset[i] + SIGN_PADDING_SMALL_Y,
			"Player %i!", winners->winners[i]+1
		);
	}

	// Draw the big sign
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	rdpq_sprite_blit (
		sign_big_sprite,
		display_get_width() - sign_x_offset,
		0,
		NULL
	);
	if (outro_timer < HYDRA_OUTRO_SHUFFLE_TIME) {
		outro_results_draw (SCORES_RESULTS_SHUFFLE);
	} else {
		outro_results_draw (SCORES_RESULTS_FINAL);
	}
}

void outro_clear (void) {
	sprite_free(sign_big_sprite);
	sprite_free(sign_small_sprite);
}
