#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "hydraharmonics.h"
#include "hydra.h"
#include "notes.h"
#include "logic.h"

const MinigameDef minigame_def = {
	.gamename = "A Hydra Harmonics",
	.developername = "Catch-64",
	.description = "Who is the best singer?",
	.instructions = "Grab the most notes to win."
};

#define FONT_TEXT 1

#define TIMER_START 0
#define TIMER_GAME NOTES_PER_PLAYER * PLAYER_MAX + (10 / NOTE_SPEED)
#define TIMER_END_ANNOUNCE 2
#define TIMER_END_DISPLAY 1
#define TIMER_END_FANFARE 2
#define TIMER_END_TOTAL TIMER_END_ANNOUNCE + TIMER_END_DISPLAY + TIMER_END_FANFARE

rdpq_font_t *font;

float timer;
uint8_t stage;
winner_t* winners;

/*==============================
	minigame_init
	The minigame initialization function
==============================*/
void minigame_init()
{
	// Define some variables

	timer = TIMER_START + TIMER_GAME + TIMER_END_TOTAL;

	// Initiate subsystems
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
	font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
	rdpq_text_register_font(FONT_TEXT, font);

	// Initiate game elements
	notes_init();
	hydra_init();
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
	if (timer <= TIMER_END_TOTAL && timer >= 0) {
		if (stage != STAGE_END) {
			notes_destroy_all();
			winners = scores_get_winner();
			stage = STAGE_END;
		}
	} else if (timer <= TIMER_END_TOTAL + TIMER_GAME && timer >= 0) {
		stage = STAGE_GAME;
		if ((int)timer != (int)(timer - deltatime) && notes_get_remaining(NOTES_GET_REMAINING_UNSPAWNED)) {
			notes_add();
		}
		// Skip to the end if there are no notes left
		if (!notes_get_remaining(NOTES_GET_REMAINING_ALL)) {
			timer = TIMER_END_TOTAL;
		}
		notes_move();
		note_hit_detection();
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
	char scores_buffer[64] = "";
	char winners_buffer[64] = "";
	char mini_buffer[32];

	// Prepare the RDP
	rdpq_attach(display_get(), NULL);
	rdpq_set_mode_fill(RGBA32(100, 100, 100, 0));
	rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());

	rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 180, 200, "Timer: %f\nRemaining:%i", timer, notes_get_remaining(NOTES_GET_REMAINING_ALL));

	// Draw things
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	hydra_draw();
	notes_draw();


	// Things that should only be drawn at particular stages
	if (stage == STAGE_GAME) {
		// Game stage
	} else if (stage == STAGE_START) {
		// Intro stage / countdown
	} else if (stage == STAGE_END) {
		// Announce a winner
		if (timer < TIMER_END_FANFARE + TIMER_END_DISPLAY) {
			// Get the player scores
			for (uint8_t i=0; i<PLAYER_MAX; i++) {
				sprintf(mini_buffer, "P %i: %i\n", i+1, scores_get(i));
				strcat(scores_buffer, mini_buffer);
			}
			// Get the winner(s)
			for (uint8_t i=0; i<winners->length; i++) {
				sprintf(mini_buffer, "Player %i wins!\n", winners->winners[i]+1);
				strcat(winners_buffer, mini_buffer);
			}
			// Print it out
			rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 100, 100, "And the winner is...\n%s\n%s", scores_buffer, winners_buffer);
		} else if (timer < TIMER_END_FANFARE) {
			// Play a song
		} else {
			// Shuffle around
			for (uint8_t i=0; i<PLAYER_MAX; i++) {
				sprintf(mini_buffer, "P %i: %i\n", i+1, rand()%100);
				strcat(scores_buffer, mini_buffer);
			}
			rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 100, 100, "And the winner is...\n%s", scores_buffer);
		}
		if (timer > TIMER_END_TOTAL/2) {

		}
	}

	rdpq_detach_show();

	// Handle the logic for the different stages
	if (stage == STAGE_GAME) {
		// Game stage
		// Handle inputs
		hydra_move();
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

	// Set the winners
	for (uint8_t i=0; i<winners->length; i++) {
		core_set_winner(winners->winners[i]);
	}

	// Free allocated memory
	notes_clear();
	hydra_clear();
	free(winners);
}
