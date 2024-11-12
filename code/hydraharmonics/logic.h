#ifndef HYDRA_LOGIC_H
#define HYDRA_LOGIC_H

#include "hydraharmonics.h"

typedef struct winner_s {
	uint8_t winners[PLAYER_MAX];
	uint16_t y_offset[PLAYER_MAX];
	uint16_t scores[PLAYER_MAX];
	uint8_t length;
} winner_t;

extern winner_t* winners;

uint16_t scores_get(uint8_t hydra);
void scores_get_winner (void);
void scores_clear (void);
void hydra_ai (uint8_t hydra);
void note_hit_detection(void);

#endif
