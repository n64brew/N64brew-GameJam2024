#ifndef HYDRA_OUTRO_H
#define HYDRA_OUTRO_H

#include "hydraharmonics.h"
#include "logic.h"

extern uint32_t outro_timer;

typedef enum {
	SCORES_RESULTS_SHUFFLE,
	SCORES_RESULTS_FINAL,
} scores_results_draw_t;

void outro_init (void);
void outro_interact (void);
void outro_sign_draw (void);
void outro_clear (void);

#endif
