#include "hydra.h"
#include "notes.h"
#include "logic.h"
#include "effects.h"
#include "audio.h"

#define HYDRA_AI_RANDOM_INTERVAL 60
#define HYDRA_AI_SMART_X_OFFSET (HYDRA_EATING_FRAMES + 4)

#define HYDRA_AI_DIFFICULTY_MODIFIER 3
#define HYDRA_AI_DIFFICULTY_COUNT 3
#define HYDRA_AI_WHITE_NOTE_MODIFIER 6

#define HIT_DETECTION_LEFT_OFFSET (HYDRA_HEAD_WIDTH/2)
#define HIT_DETECTION_RIGHT_OFFSET 0

static int16_t scores[PLAYER_MAX] = {0};
winner_t* winners;

void scores_get_winner (void) {
	int i, j;
	winners = malloc(sizeof(winner_t));
	winners->length = 0;

	uint8_t temp_players[PLAYER_MAX] = {0,1,2,3};
	int16_t temp_scores[PLAYER_MAX];
	int16_t temp;
	memcpy(temp_scores, scores, sizeof(int16_t) * PLAYER_MAX);

	// Sort the temp arrays
	for (i=0; i<PLAYER_MAX; i++){
		for (j=0; j<PLAYER_MAX-i-1; j++) {
			// Is this one in the wrong position compared to the next one?
			if (temp_scores[j] < temp_scores[j+1]){
				// Swap them!
				temp = temp_players[j];
				temp_players[j] = temp_players[j+1];
				temp_players[j+1] = temp;
				temp = temp_scores[j];
				temp_scores[j] = temp_scores[j+1];
				temp_scores[j+1] = temp;
			}
		}
	}

	// Place them in the winner array
	for (i=0; i<PLAYER_MAX; i++){
		if (i==0 || temp_scores[i] == temp_scores[i-1]) {
			winners->winners[i] = temp_players[i];
			winners->y_offset[i] = 0;
			winners->length++;
		} else {
			break;
		}
	}
}

bool score_is_winner (PlyNum p) {
	for (uint8_t i=0; i<winners->length; i++){
		if (winners->winners[i] == p) {
			return true;
		}
	}
	return false;
}

PlyNum scores_get_extreme (scores_extreme_t type) {
	PlyNum extreme = PLAYER_1;
	if (
		scores[PLAYER_1] == scores[PLAYER_2] &&
		scores[PLAYER_2] == scores[PLAYER_3] &&
		scores[PLAYER_3] == scores[PLAYER_4]
	) {
		return -1;
	}
	for (uint8_t i=PLAYER_2; i<PLAYER_MAX; i++){
		if (type == SCORES_GET_FIRST && scores[i] > scores[extreme]) {
			extreme = i;
		}
		if (type == SCORES_GET_LAST && scores[i] < scores[extreme]) {
			extreme = i;
		}
	}
	return extreme;
}

int16_t scores_get(uint8_t hydra) {
	assertf(hydra < PLAYER_MAX, "Invalid player number!");
	return scores[hydra];
}

void scores_clear (void) {
	// Set the winners in the game jam core
	for (uint8_t i=0; i<winners->length; i++) {
		core_set_winner(winners->winners[i]);
	}
	free(winners);
}

void note_hit_detection(void) {
	note_t* current = notes_get_first();
	int8_t add_score;
	// Loop trhough notes
	while (current != NULL) {
		// Loop through players
		for (uint8_t i=0; i<PLAYER_MAX; i++) {
			// Check if the note and player are colliding
			if (
				(
					current->x < hydras[i].x + HYDRA_HEAD_WIDTH + HIT_DETECTION_RIGHT_OFFSET &&
					current->x > hydras[i].x + HIT_DETECTION_LEFT_OFFSET &&
					hydras[i].y == PADDING_TOP+current->state*HYDRA_ROW_HEIGHT
				)
			){
				// Check if this note's colour matches the player'
				add_score = 1;
					//(HYDRA_EATING_FRAMES - abs(HYDRA_EATING_FRAMES-((hydras[i].animation-1)*HYDRA_EATING_FRAMES + hydras[i].frame))) / 5 + 1;
				if (current->player == i || current->player == PLAYER_MAX) {
					// Check if the animation is correct
					if (hydras[i].animation == HYDRA_ANIMATION_OPEN || hydras[i].animation == HYDRA_ANIMATION_CLOSE) {
						// Note is eaten
						hydras[i].last_eaten = current->type;
						if (current->type == NOTES_TYPE_STANDARD) {
							// No swap, just eat
							hydras[i].animation += HYDRA_ANIMATION_SUCCESS_DIFF;
						} else if (
							current->type == NOTES_TYPE_SWAP_BACK ||
							current->type == NOTES_TYPE_SWAP_FORWARD ||
							current->type == NOTES_TYPE_SWAP_RANDOM ||
							current->type == NOTES_TYPE_SWAP_ALL
						) {
							// Perform a swap
							hydra_swap_start (i, current->type);
							hydras[i].animation += HYDRA_ANIMATION_TO_SWAP_DIFF;
						} else if (
							current->type == NOTES_TYPE_SWEET
						) {
							// Just ate a sweet note
							add_score *= 2;
							hydras[i].animation += HYDRA_ANIMATION_SUCCESS_DIFF;
						} else if (
							current->type == NOTES_TYPE_SOUR
						) {
							// Just ate a sour note
							add_score = -1;
							hydras[i].animation += HYDRA_ANIMATION_TO_DIZZY_DIFF;
						}
						scores[i] += add_score;
						notes_destroy (current);
					}
				} else if (
					current->type == NOTES_TYPE_SOUR
				) {
					// Enemy sour note
					if (hydras[i].animation == HYDRA_ANIMATION_OPEN || hydras[i].animation == HYDRA_ANIMATION_CLOSE) {
						hydra_swap_start (current->player, current->type);
						hydras[i].animation += HYDRA_ANIMATION_TO_SWAP_DIFF;
						notes_destroy (current);
					}
				} else if (
					(hydras[i].animation < HYDRA_ANIMATION_SWAP_DOWN || hydras[i].animation > HYDRA_ANIMATION_SWAP_WAIT) &&
					hydras[i].animation != HYDRA_ANIMATION_OPEN_TO_SWAP &&
					hydras[i].animation != HYDRA_ANIMATION_CLOSE_TO_SWAP &&
					hydras[i].animation != HYDRA_ANIMATION_CHEW_TO_SWAP
				) {
					// It's someone else's note. Stun them.
					if (hydras[i].animation != HYDRA_ANIMATION_STUN) {
						audio_sfx_play(SFX_OW);
						effects_add(
							i,
							EFFECT_SHOCK,
							hydras[i].x + HYDRA_CHEW_EFFECT_OFFSET_X,
							hydras[i].y + hydra_get_hat_offset(i) - HYDRA_STUN_EFFECT_OFFSET_Y
						);
					}
					hydra_animate (i, HYDRA_ANIMATION_STUN);
				}
			} else if (current->x < -current->sprite->width) {
				notes_destroy (current);
			}
		}
		current = current->next;
	}
}

void hydra_ai (uint8_t hydra) {
	// Loop through all notes
	note_t* current = notes_get_first();
	while (current != NULL) {
		// Check if there is a note in range
		if (
			current->x < hydras[hydra].x + HYDRA_HEAD_WIDTH + HYDRA_AI_SMART_X_OFFSET &&
			current->x > hydras[hydra].x &&
			hydras[hydra].animation == HYDRA_ANIMATION_NONE
		){
			// Check if we should do something smart (and skip sour notes on hard difficulty)
			if (
				!(rand() % (HYDRA_AI_DIFFICULTY_MODIFIER * 3 - HYDRA_AI_DIFFICULTY_MODIFIER * core_get_aidifficulty())) &&
				!(core_get_aidifficulty() == DIFF_HARD && current->type == NOTES_TYPE_SOUR && (rand() % (HYDRA_AI_DIFFICULTY_MODIFIER*6+1)))
			) {
				// Chance to ignore white notes and sour notes on high difficulty
				if (current->player == PLAYER_MAX && (rand() % HYDRA_AI_WHITE_NOTE_MODIFIER)) {
					return;
				}
				// Check if this is a note that we want to eat
				if (current->player == hydra || current->player == PLAYER_MAX) {
					// It's the hydra's note!
					audio_sfx_play(SFX_AAH_MIN + hydra);
					hydras[hydra].animation = HYDRA_ANIMATION_OPEN;
					hydras[hydra].state = current->state;
				} else if (hydras[hydra].state == current->state) {
					// It's an enemy note! Move one row down
					hydras[hydra].state = (hydras[hydra].state + 1) % HEAD_STATES_MAX;
				}
			}
		}
		current = current->next;
	}
}
