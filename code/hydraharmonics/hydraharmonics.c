#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "hydraharmonics.h"
#include "hydra.h"
#include "notes.h"

const MinigameDef minigame_def = {
	.gamename = "A Hydra Harmonics",
	.developername = "Catch-64",
	.description = "Who is the best singer?",
	.instructions = "Grab the most notes to win."
};

#define FONT_TEXT 1

#define TIMER_START 1
#define TIMER_GAME 10
#define TIMER_END 1

rdpq_font_t *font;

float timer;
uint8_t stage;

void hit_detection(void) {
	note_t* current = notes_get_first();
	while (current != NULL) {
		if (
			(current->x < hydras[current->player].x + hydras[current->player].head_sprite->width &&
			current->state == hydras[current->player].state) ||
			(current->x < -current->sprite->width)
		){
			notes_destroy (current);
		}
		current = current->next;
	}
}

/*==============================
	minigame_init
	The minigame initialization function
==============================*/
void minigame_init()
{
	// Define some variables

	timer = TIMER_START + TIMER_GAME + TIMER_END;

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
	if (timer <= TIMER_END && timer >= 0) {
		stage = STAGE_END;
	} else if (timer <= TIMER_END + TIMER_GAME && timer >= 0) {
		stage = STAGE_GAME;
	} else  if (timer >= 0) {
		stage = STAGE_START;
	} else {
		minigame_end();
	}
	if ((int)timer != (int)(timer - deltatime)) {
		notes_add();
	}
	timer -= deltatime;
	notes_move();
	hit_detection();
}

/*==============================
	minigame_loop
	Code that is called every loop.
	@param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
	// Define some variables
	rdpq_attach(display_get(), NULL);
	rdpq_set_mode_fill(RGBA32(100, 100, 100, 0));
	rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
	rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 155, 100, "Delta time: %f\nStage: %i", timer, stage);

	// Draw things
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	hydra_draw();
	notes_draw();

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

	// Free allocated memory
	notes_clear();
	hydra_clear();
}
