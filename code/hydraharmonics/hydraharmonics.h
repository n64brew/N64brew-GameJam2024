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

#define PADDING_TOP 25
#define PADDING_LEFT 10

#define FONT_DEFAULT 1
#define FONT_CLARENDON 2

#define STATES_TOTAL 4
#define STATES_USABLE 3

typedef enum {
	STAGE_START,
    STAGE_GAME,
    STAGE_END,
    STAGE_RETURN_TO_MENU,
} hydraharmonics_stage_t;

typedef enum {
	STATE_UP,
    STATE_MID,
    STATE_DOWN,
    STATE_COUNT,
} hydraharmonics_state_t;

typedef enum {
	NOTES_TYPE_STANDARD,
	NOTES_TYPE_SWAP_BACK,
	NOTES_TYPE_SWAP_FORWARD,
	NOTES_TYPE_SWAP_RANDOM,
	NOTES_TYPE_SWAP_ALL,
	NOTES_TYPE_SWEET,
	NOTES_TYPE_SOUR,
	NOTES_TYPE_COUNT,
} notes_types_t;

extern hydraharmonics_stage_t stage;
extern bool pause;

#endif
