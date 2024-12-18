#ifndef HYDRA_ANIMATION_H
#define HYDRA_ANIMATION_H

typedef enum {
	// Row 1
	FACE_ANIM_NONE,
	FACE_ANIM_OPEN_1,
	FACE_ANIM_OPEN_2,
	FACE_ANIM_OPEN_3,
	FACE_ANIM_OPEN_4,
	FACE_ANIM_OPEN_5,
	FACE_ANIM_OPEN_6,
	FACE_ANIM_OPEN_7,
	FACE_ANIM_OPEN_8,
	FACE_ANIM_OPEN_9,
	FACE_ANIM_OPEN_10,
	// Row 2
	FACE_ANIM_CLOSE_0,
	FACE_ANIM_CLOSE_1,
	FACE_ANIM_CLOSE_2,
	FACE_ANIM_CLOSE_3,
	FACE_ANIM_CLOSE_4,
	FACE_ANIM_CLOSE_5,
	FACE_ANIM_CLOSE_6,
	FACE_ANIM_CLOSE_7,
	FACE_ANIM_CLOSE_8,
	FACE_ANIM_CLOSE_9,
	FACE_ANIM_CLOSE_10,
	// Row 3
	FACE_ANIM_DIZZY_0,
	FACE_ANIM_DIZZY_1,
	FACE_ANIM_DIZZY_2,
	FACE_ANIM_DIZZY_3,
	FACE_ANIM_DIZZY_4,
	FACE_ANIM_DIZZY_5,
	FACE_ANIM_DIZZY_6,
	FACE_ANIM_DIZZY_7,
	FACE_ANIM_STUN,
	FACE_ANIM_GRIN,
	FACE_ANIM_SWAP,
	// Row 4
	FACE_ANIM_SWAP_WAIT,
	FACE_ANIM_NONE_PLUS_2,
} hydra_face_frames_t;

typedef enum {
	// Row 1
	EYES_ANIM_RN,
	EYES_ANIM_RD,
	EYES_ANIM_ND,
	EYES_ANIM_LD,
	EYES_ANIM_LN,
	// Row 2
	EYES_ANIM_LU,
	EYES_ANIM_NU,
	EYES_ANIM_RU,
	EYES_ANIM_HR,
	EYES_ANIM_HD,
	// Row 3
	EYES_ANIM_HL,
	EYES_ANIM_HU,
	EYES_ANIM_NN,
	EYES_ANIM_NN_BLANK,
	EYES_ANIM_CLOSED_NEUTRAL,
	// Row 4
	EYES_ANIM_CLOSED_DOWN,
	EYES_ANIM_CLOSED_UP,
	EYES_ANIM_CLOSED_DIAG_UP,
	EYES_ANIM_CLOSED_DIAG_DOWN,
	EYES_ANIM_SWIRL_0,
	// Row 5
	EYES_ANIM_SWIRL_1,
	EYES_ANIM_SWIRL_2,
	EYES_ANIM_SWIRL_3,
	EYES_ANIM_WIDE_OPEN,
	EYES_ANIM_WIDE_SHUT,
	// Aliases
	EYES_ANIM_NONE = EYES_ANIM_RN,
} hydra_eye_frames_t;

typedef struct hydra_animation_frame_s {
	hydra_face_frames_t face_sprite;
	hydra_eye_frames_t eyes_sprite;
	uint8_t frame_duration;
	hydra_flair_frames_t flair_sprite;
	int8_t flair_x;
	int8_t flair_y;
} hydra_animation_frame_t;

typedef struct hydra_animation_s {
	const hydra_animation_frame_t* frame_array;
	uint8_t animation_length;
	hydraharmonics_animations_t next;
} hydra_animation_t;

static const hydra_animation_frame_t animation_db_none[HYDRA_ANIMATION_LENGTH_NONE] = {
	{FACE_ANIM_NONE, EYES_ANIM_NONE, 1},
};

static const hydra_animation_frame_t animation_db_open[HYDRA_ANIMATION_LENGTH_OPEN] = {
	{FACE_ANIM_NONE,	EYES_ANIM_NONE,			1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_1,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_2,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_3,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_4,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_5,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_6,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_7,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_8,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_9,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_OPEN_10,	EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
};

static const hydra_animation_frame_t animation_db_close[HYDRA_ANIMATION_LENGTH_CLOSE] = {
	{FACE_ANIM_CLOSE_0,		EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_1,		EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_2,		EYES_ANIM_WIDE_OPEN,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_3,		EYES_ANIM_WIDE_SHUT,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_4,		EYES_ANIM_WIDE_SHUT,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_5,		EYES_ANIM_WIDE_SHUT,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_6,		EYES_ANIM_WIDE_SHUT,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_7,		EYES_ANIM_WIDE_SHUT,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_8,		EYES_ANIM_WIDE_SHUT,	1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_9,		EYES_ANIM_NONE,			1, FLAIR_NONE, 0, 0},
	{FACE_ANIM_CLOSE_10,	EYES_ANIM_NONE,			1, FLAIR_NONE, 0, 0},
};

// Aliases for other open/close animations
static const hydra_animation_frame_t* const animation_db_open_success = animation_db_open;
static const hydra_animation_frame_t* const animation_db_close_success = animation_db_close;
static const hydra_animation_frame_t* const animation_db_open_swap = animation_db_open;
static const hydra_animation_frame_t* const animation_db_close_swap = animation_db_close;
static const hydra_animation_frame_t* const animation_db_open_stun = animation_db_open;
static const hydra_animation_frame_t* const animation_db_close_stun = animation_db_close;

static const hydra_animation_frame_t animation_db_chew[HYDRA_ANIMATION_LENGTH_CHEW] = {
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_UP,	5, FLAIR_CHEEK,	-4,	15},
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_UP,	5, FLAIR_CHEEK,	-4,	13},
	{FACE_ANIM_NONE_PLUS_2,	EYES_ANIM_CLOSED_UP,	5, FLAIR_CHEEK,	-4,	15},
	{FACE_ANIM_NONE_PLUS_2,	EYES_ANIM_CLOSED_UP,	5, FLAIR_CHEEK,	-4,	17},
};

// Aliases for chew animations
static const hydra_animation_frame_t* const animation_db_chew_swap = animation_db_chew;
static const hydra_animation_frame_t* const animation_db_chew_dizzy = animation_db_chew;

static const hydra_animation_frame_t animation_db_sleep[HYDRA_ANIMATION_LENGTH_SLEEP] = {
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_DOWN,	25, FLAIR_NONE,	0,	0},
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_DOWN,	25, FLAIR_NONE,	0,	0},
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_DOWN,	25, FLAIR_NONE,	0,	0},
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_DOWN,	25, FLAIR_NONE,	0,	0},
};

static const hydra_animation_frame_t animation_db_stun[HYDRA_ANIMATION_LENGTH_STUN] = {
	{FACE_ANIM_STUN,		EYES_ANIM_NN_BLANK,	HYDRA_STUN_DURATION, FLAIR_NONE,	0,	0},
};

static const hydra_animation_frame_t* const animation_db_stun_loop = animation_db_stun;

static const hydra_animation_frame_t animation_db_dizzy[HYDRA_ANIMATION_LENGTH_DIZZY] = {
	{FACE_ANIM_DIZZY_0,		EYES_ANIM_SWIRL_0,	5, FLAIR_NONE,	0,	0},
	{FACE_ANIM_DIZZY_1,		EYES_ANIM_SWIRL_1,	5, FLAIR_NONE,	0,	0},
	{FACE_ANIM_DIZZY_2,		EYES_ANIM_SWIRL_2,	5, FLAIR_NONE,	0,	0},
	{FACE_ANIM_DIZZY_3,		EYES_ANIM_SWIRL_3,	5, FLAIR_NONE,	0,	0},
	{FACE_ANIM_DIZZY_4,		EYES_ANIM_SWIRL_0,	5, FLAIR_NONE,	0,	0},
	{FACE_ANIM_DIZZY_5,		EYES_ANIM_SWIRL_1,	5, FLAIR_NONE,	0,	0},
	{FACE_ANIM_DIZZY_6,		EYES_ANIM_SWIRL_2,	5, FLAIR_NONE,	0,	0},
	{FACE_ANIM_DIZZY_7,		EYES_ANIM_SWIRL_3,	5, FLAIR_NONE,	0,	0},
};

static const hydra_animation_frame_t animation_db_loser[HYDRA_ANIMATION_LENGTH_LOSER] = {
	{FACE_ANIM_DIZZY_0,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
	{FACE_ANIM_DIZZY_1,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
	{FACE_ANIM_DIZZY_2,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
	{FACE_ANIM_DIZZY_3,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
	{FACE_ANIM_DIZZY_4,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
	{FACE_ANIM_DIZZY_5,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
	{FACE_ANIM_DIZZY_6,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
	{FACE_ANIM_DIZZY_7,		EYES_ANIM_LD,	5, FLAIR_EYELID_0,	14,	3},
};

static const hydra_animation_frame_t animation_db_swap_down[HYDRA_ANIMATION_LENGTH_SWAP_DOWN] = {
	{FACE_ANIM_SWAP,		EYES_ANIM_NONE,	1, FLAIR_NONE,	0,	0},
};

// Aliases for swap animations
static const hydra_animation_frame_t* const animation_db_swap_up = animation_db_swap_down;

static const hydra_animation_frame_t animation_db_swap_wait[HYDRA_ANIMATION_LENGTH_SWAP_WAIT] = {
	{FACE_ANIM_SWAP_WAIT,	EYES_ANIM_NONE,	1, FLAIR_NONE,	0,	0},
};

static const hydra_animation_frame_t animation_db_grin[HYDRA_ANIMATION_LENGTH_GRIN] = {
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_UP,	5, FLAIR_GRIN,	8,	11},
	{FACE_ANIM_NONE,		EYES_ANIM_CLOSED_UP,	5, FLAIR_GRIN,	8,	10},
	{FACE_ANIM_NONE_PLUS_2,	EYES_ANIM_CLOSED_UP,	5, FLAIR_GRIN,	8,	11},
	{FACE_ANIM_NONE_PLUS_2,	EYES_ANIM_CLOSED_UP,	5, FLAIR_GRIN,	8,	12},
};

// Main database of animations
static const hydra_animation_t animation_db [HYDRA_ANIMATION_COUNT] = {
		// Animation				// Length								// Next animation
	{	animation_db_none,			HYDRA_ANIMATION_LENGTH_NONE,			HYDRA_ANIMATION_NONE},
	{	animation_db_open,			HYDRA_ANIMATION_LENGTH_OPEN,			HYDRA_ANIMATION_CLOSE},
	{	animation_db_close,			HYDRA_ANIMATION_LENGTH_CLOSE,			HYDRA_ANIMATION_NONE},
	{	animation_db_open_swap,		HYDRA_ANIMATION_LENGTH_OPEN_SUCCESS,	HYDRA_ANIMATION_CLOSE_SUCCESS},
	{	animation_db_close_swap,	HYDRA_ANIMATION_LENGTH_CLOSE_SUCCESS,	HYDRA_ANIMATION_CHEW},
	{	animation_db_open_swap,		HYDRA_ANIMATION_LENGTH_OPEN_TO_SWAP,	HYDRA_ANIMATION_CLOSE_TO_SWAP},
	{	animation_db_close_swap,	HYDRA_ANIMATION_LENGTH_CLOSE_TO_SWAP,	HYDRA_ANIMATION_CHEW_TO_SWAP},
	{	animation_db_open_stun,		HYDRA_ANIMATION_LENGTH_OPEN_TO_DIZZY,	HYDRA_ANIMATION_CLOSE_TO_DIZZY},
	{	animation_db_close_stun,	HYDRA_ANIMATION_LENGTH_CLOSE_TO_DIZZY,	HYDRA_ANIMATION_CHEW_TO_DIZZY},
	{	animation_db_chew,			HYDRA_ANIMATION_LENGTH_CHEW,			HYDRA_ANIMATION_NONE},
	{	animation_db_chew_swap,		HYDRA_ANIMATION_LENGTH_CHEW_TO_SWAP,	HYDRA_ANIMATION_SWAP_DOWN},
	{	animation_db_chew_dizzy,	HYDRA_ANIMATION_LENGTH_CHEW_TO_DIZZY,	HYDRA_ANIMATION_DIZZY},
	{	animation_db_sleep,			HYDRA_ANIMATION_LENGTH_SLEEP,			HYDRA_ANIMATION_SLEEP},
	{	animation_db_stun,			HYDRA_ANIMATION_LENGTH_STUN,			HYDRA_ANIMATION_NONE},
	{	animation_db_stun_loop,		HYDRA_ANIMATION_LENGTH_STUN_LOOP,		HYDRA_ANIMATION_STUN},
	{	animation_db_dizzy,			HYDRA_ANIMATION_LENGTH_DIZZY,			HYDRA_ANIMATION_NONE},
	{	animation_db_loser,			HYDRA_ANIMATION_LENGTH_LOSER,			HYDRA_ANIMATION_LOSER},
	{	animation_db_swap_down,		HYDRA_ANIMATION_LENGTH_SWAP_DOWN,		HYDRA_ANIMATION_SWAP_DOWN},
	{	animation_db_swap_up,		HYDRA_ANIMATION_LENGTH_SWAP_UP,			HYDRA_ANIMATION_SWAP_UP},
	{	animation_db_swap_wait,		HYDRA_ANIMATION_LENGTH_SWAP_WAIT,		HYDRA_ANIMATION_SWAP_WAIT},
	{	animation_db_grin,			HYDRA_ANIMATION_LENGTH_GRIN,			HYDRA_ANIMATION_GRIN},
};
#endif
