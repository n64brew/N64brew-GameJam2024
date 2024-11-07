#include "hydra.h"
#include "notes.h"
#include "logic.h"

#define HYDRA_AI_RANDOM_INTERVAL 60
#define HYDRA_AI_SMART_X_OFFSET (HYDRA_EATING_FRAMES + 4)

#define HYDRA_AI_DIFFICULTY_MODIFIER 3
#define HYDRA_AI_DIFFICULTY_COUNT 3

#define HIT_DETECTION_LEFT_OFFSET (HYDRA_HEAD_WIDTH/2)
#define HIT_DETECTION_RIGHT_OFFSET 0

static uint16_t scores[PLAYER_MAX] = {0};
winner_t* winners;

void scores_get_winner (void) {
	int i, j;
	winners = malloc(sizeof(winner_t));
	winners->length = 0;

	uint8_t temp_players[PLAYER_MAX] = {0,1,2,3};
	uint16_t temp_scores[PLAYER_MAX];
	uint16_t temp;
	memcpy(temp_scores, scores, sizeof(uint16_t) * PLAYER_MAX);

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
			winners->length++;
		} else {
			break;
		}
	}
}

uint16_t scores_get(uint8_t hydra) {
	assertf(hydra < PLAYER_MAX, "Invalid player number!");
	return scores[hydra];
}

void scores_results_draw (scores_results_draw_t type) {
	char scores_buffer[64] = "";
	char winners_buffer[64] = "";
	char mini_buffer[32];

	// Get the player scores
	for (uint8_t i=0; i<PLAYER_MAX; i++) {
		sprintf(mini_buffer, "P %i: %i\n", i+1, type == SCORES_RESULTS_SHUFFLE ? rand()%100 : scores_get(i));
		strcat(scores_buffer, mini_buffer);
	}

	// Get the winner(s)
	for (uint8_t i=0; type == SCORES_RESULTS_FINAL && i<winners->length; i++) {
		sprintf(mini_buffer, "Player %i wins!\n", winners->winners[i]+1);
		strcat(winners_buffer, mini_buffer);
	}

	// Print it out
	rdpq_text_printf(NULL, FONT_CLARENDON, 60, 50, "And the winner is...\n%s\n%s", scores_buffer, winners_buffer);
}

void scores_clear (void) {
	// Set the winners in the game jam core
	for (uint8_t i=0; i<winners->length; i++) {
		core_set_winner(winners->winners[i]);
	}

	// Free memory
	free(winners);
}

void note_hit_detection(void) {
	note_t* current = notes_get_first();
	uint8_t add_score;
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
				add_score =
					(HYDRA_EATING_FRAMES - abs(HYDRA_EATING_FRAMES-((hydras[i].animation-1)*HYDRA_EATING_FRAMES + hydras[i].frame))) / 5 + 1;
				if (i == current->player) {
					// Check if the animation is correct
					if (hydras[i].animation == HYDRA_ANIMATION_OPEN || hydras[i].animation == HYDRA_ANIMATION_CLOSE) {
						// Note is eaten
						scores[i] += add_score;
						hydras[i].animation += HYDRA_ANIMATION_SUCCESS_DIFF;
						notes_destroy (current);
					}
				} else if (current->player >= PLAYER_MAX) {
					// It's a special note
					if (hydras[i].animation == HYDRA_ANIMATION_OPEN || hydras[i].animation == HYDRA_ANIMATION_CLOSE) {
						scores[i] += add_score;
						hydra_swap_start (i, current->player);
						notes_destroy (current);
					}
				} else if (
					(hydras[i].animation < HYDRA_ANIMATION_SWAP_DOWN || hydras[i].animation > HYDRA_ANIMATION_SWAP_WAIT) &&
					hydras[i].animation != HYDRA_ANIMATION_OPEN_TO_SWAP &&
					hydras[i].animation != HYDRA_ANIMATION_CLOSE_TO_SWAP &&
					hydras[i].animation != HYDRA_ANIMATION_CHEW_TO_SWAP
				) {
					// It's someone else's note. Stun them.
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
	static uint16_t ai_timer = 0;
	ai_timer++;
	note_t* current;
	if (hydras[hydra].ai == HYDRA_AI_SMART) {
		// Loop through all notes
		current = notes_get_first();
		while (current != NULL) {
			// Check if there is a note in range
			if (
				current->x < hydras[hydra].x + HYDRA_HEAD_WIDTH + HYDRA_AI_SMART_X_OFFSET &&
				current->x > hydras[hydra].x &&
				hydras[hydra].animation == HYDRA_ANIMATION_NONE
			){
				// Check if we should do something smart
				if (!(rand() % (HYDRA_AI_DIFFICULTY_MODIFIER * 3 - HYDRA_AI_DIFFICULTY_MODIFIER * core_get_aidifficulty()))) {
					// Check if this is a note that we want to eat
					if (current->player == hydra) {
						// It's the hydra's note!
						hydras[hydra].animation = HYDRA_ANIMATION_OPEN;
						hydras[hydra].state = current->state;
					} else if (hydras[hydra].state == current->state) {
						// It's an enemy note! Move one row down
						hydras[hydra].state = (hydras[hydra].state + 1) % HEAD_STATES_PLAYABLE;
					}
				}
			} else {
				//hydras[hydra].state = STATE_MID;
			}
			current = current->next;
		}
	} else if (hydras[hydra].ai == HYDRA_AI_RANDOM) {
		// Loop through all notes
		current = notes_get_first();
		while (current != NULL) {
			if (
				current->player == hydra &&
				current->x < hydras[hydra].x + HYDRA_HEAD_WIDTH + HYDRA_AI_SMART_X_OFFSET &&
				current->x > hydras[hydra].x &&
				hydras[hydra].animation == HYDRA_ANIMATION_NONE &&
				hydras[hydra].state == current->state
			){
				hydras[hydra].animation = HYDRA_ANIMATION_OPEN;
			}
			current = current->next;
		}
		if (!(ai_timer % HYDRA_AI_RANDOM_INTERVAL) && hydras[hydra].animation == HYDRA_ANIMATION_NONE) {
			hydras[hydra].state = rand() % 3;
		}
	}
}
