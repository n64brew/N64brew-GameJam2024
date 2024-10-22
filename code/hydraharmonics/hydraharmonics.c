#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "hydraharmonics.h"

const MinigameDef minigame_def = {
	.gamename = "A Hydra Harmonics",
	.developername = "Catch-64",
	.description = "Who is the best singer?",
	.instructions = "Grab the most notes to win."
};

#define FONT_TEXT 1
#define PLAYER_MAX 4

#define TIMER_START 1
#define TIMER_GAME 10
#define TIMER_END 1

rdpq_font_t *font;

hydra_t players[PLAYER_MAX];

sprite_t* head_sprites[PLAYER_MAX][HEAD_STATES_MAX];
float timer;
uint8_t stage;

/*==============================
	minigame_init
	The minigame initialization function
==============================*/
void minigame_init()
{
	// Define some variables
	uint8_t i, j;
	char temptext[64];
	timer = TIMER_START + TIMER_GAME + TIMER_END;

	// Initiate subsystems
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
	font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
	rdpq_text_register_font(FONT_TEXT, font);

	// Set the player defaults
	for (i=0; i<PLAYER_MAX; i++) {
		players[i].state = STATE_MID;
		// Load the head sprites
		for (j=0; j<HEAD_STATES_MAX; j++) {
			sprintf(temptext, "rom:/hydraharmonics/head-%i-%i.ci4.sprite", i, j);
			players[i].head_sprites[j] = sprite_load(temptext);
		}
	}
}

/*==============================
	minigame_fixedloop
	Code that is called every loop, at a fixed delta time.
	Use this function for stuff where a fixed delta time is
	important, like physics.
	@param  The fixed delta time for this tick
==============================*/
void minigame_fixedloop(float deltatime)
{
	// Handle the timer to determine the part of the minigame we're at
	if (timer <= TIMER_END && timer >= 0) {
		stage = STAGE_END;
	} else if (timer <= TIMER_END + TIMER_GAME && timer >= 0) {
		stage = STAGE_GAME;
	} else  if (timer >= 0) {
		stage = STAGE_START;
	} else {
		minigame_end();
	}
	timer -= deltatime;
}

/*==============================
	minigame_loop
	Code that is called every loop.
	@param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
	// Define some variables
	uint8_t i, j;
	rdpq_attach(display_get(), NULL);
	rdpq_set_mode_fill(RGBA32(0, 0, 0, 0));
	rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
	rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 155, 100, "Delta time: %f\nStage: %i", timer, stage);

	rdpq_set_mode_standard();
	for (i=0; i<PLAYER_MAX; i++) {

		rdpq_sprite_blit(
			players[i].head_sprites[players[i].state],
			10+i*players[i].head_sprites[players[i].state]->width,
			10+players[i].state*players[i].head_sprites[players[i].state]->height,
			NULL
		);
	}

	rdpq_detach_show();

	// Handle the logic for the different stages
	if (stage == STAGE_GAME) {
		// Game stage
		// Handle inputs
		joypad_buttons_t buttons;
		for (i=0; i<core_get_playercount(); i++) {
			buttons = joypad_get_buttons(core_get_playercontroller(i));
			if (buttons.d_up) {
				players[i].state = STATE_UP;
			} else if (buttons.d_down) {
				players[i].state = STATE_DOWN;
			} else {
				players[i].state = STATE_MID;
			}
		}
	} else if (stage == STAGE_START) {
		// Intro stage / countdown
	} else if (stage == STAGE_END) {
		// Announce a winner
	}
}

/*==============================
	minigame_cleanup
	Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
	rdpq_text_unregister_font(FONT_TEXT);
	rdpq_font_free(font);
	display_close();
	// Clear the head sprites
	uint8_t i, j;
	for (i=0; i<PLAYER_MAX; i++) {
		for (j=0; j<HEAD_STATES_MAX; j++) {
			sprite_free(players[i].head_sprites[j] );
		}
	}
}
