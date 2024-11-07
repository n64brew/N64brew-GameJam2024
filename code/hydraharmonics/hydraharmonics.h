#ifndef HYDRAHARMONICS_H
#define HYDRAHARMONICS_H

#include <libdragon.h>
#include "../../core.h"

#define HEAD_STATES_MAX 4
#define HEAD_STATES_PLAYABLE (HEAD_STATES_MAX - 1)
#define PLAYER_MAX MAXPLAYERS

#define HYDRA_HEAD_WIDTH 48
#define HYDRA_HEAD_HEIGHT 64
#define HYDRA_ROW_HEIGHT 32

#define PADDING_TOP 10
#define PADDING_LEFT 10

#define FONT_DEFAULT 1
#define FONT_CLARENDON 2

typedef enum {
	STAGE_START,
    STAGE_GAME,
    STAGE_END,
} hydraharmonics_stage_t;

typedef enum {
	STATE_UP,
    STATE_MID,
    STATE_DOWN,
    STATE_HIDE,
} hydraharmonics_state_t;

typedef enum {
	NOTES_SPECIAL_WHITE = 4,
	NOTES_SPECIAL_GREY = 5,
	NOTES_SPECIAL_BLACK = 6,
} notes_special_t;

#endif
