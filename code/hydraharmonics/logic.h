#ifndef HYDRA_LOGIC_H
#define HYDRA_LOGIC_H

#include "hydraharmonics.h"

typedef struct winner_s {
	uint8_t winners[PLAYER_MAX];
	uint16_t scores[PLAYER_MAX];
	uint8_t length;
} winner_t;

typedef enum {
	SCORES_RESULTS_SHUFFLE,
	SCORES_RESULTS_FINAL,
} scores_results_draw_t;

uint16_t scores_get(uint8_t hydra);
void scores_results_draw (scores_results_draw_t type);
void scores_get_winner (void);
void scores_clear (void);
void hydra_ai (uint8_t hydra);
void note_hit_detection(void);

#endif
