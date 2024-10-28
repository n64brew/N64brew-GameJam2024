#include "hydra.h"
#include "notes.h"
#include "logic.h"

#define HYDRA_AI_RANDOM_INTERVAL 20
#define HYDRA_AI_SMART_X_OFFSET 20

static uint16_t scores[PLAYER_MAX] = {0};

winner_t* scores_get_winner (void) {
	int i, j;
	winner_t* winner = malloc(sizeof(winner_t));
	winner->length = 0;

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
			winner->winners[i] = temp_players[i];
			winner->length++;
		} else {
			break;
		}
	}

	return winner;
}

uint16_t scores_get(uint8_t hydra) {
	assertf(hydra < PLAYER_MAX, "Invalid player number!");
	return scores[hydra];
}

void note_hit_detection(void) {
	note_t* current = notes_get_first();
	while (current != NULL) {
		if (
			(
				current->x < hydras[current->player].x + hydras[current->player].head_sprite->width &&
				current->x > hydras[current->player].x - current->sprite->width &&
				current->state == hydras[current->player].state
			)
		){
			scores[current->player]++;
			notes_destroy (current);
		} else if (current->x < -current->sprite->width) {
			notes_destroy (current);
		}
		current = current->next;
	}
}

void hydra_ai (uint8_t hydra) {
	static uint16_t ai_timer = 0;
	ai_timer++;
	if (hydras[hydra].ai == HYDRA_AI_SMART) {
		note_t* current = notes_get_first();
		while (current != NULL) {
			if (current->player == hydra && current->x < hydras[hydra].x + hydras[hydra].head_sprite->width + HYDRA_AI_SMART_X_OFFSET){
				hydras[hydra].state = current->state;
			} else {
				//hydras[hydra].state = STATE_MID;
			}
			current = current->next;
		}
	} else if (hydras[hydra].ai == HYDRA_AI_RANDOM) {
		if (!(ai_timer % HYDRA_AI_RANDOM_INTERVAL)) {
			hydras[hydra].state = rand() % 3;
		}
	}
}
