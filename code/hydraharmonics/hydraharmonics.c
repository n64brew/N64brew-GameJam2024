#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "hydraharmonics.h"
#include "hydra.h"
#include "notes.h"
#include "logic.h"
#include "intro.h"
#include "outro.h"
#include "ui.h"
#include "effects.h"
#include "audio.h"

const MinigameDef minigame_def = {
	.gamename = "Hydra Harmonics",
	.developername = "Catch-64",
	.description = "Taste the music! Eat the most notes to win!",
	.instructions = "Use A to eat and up or down to move."
};

#define TIMER_GAME (NOTES_PER_PLAYER * PLAYER_MAX) + (NOTES_PER_PLAYER_SPECIAL * NOTES_SPECIAL_TYPES) + (13 / NOTES_SPEED)
#define TIMER_END_ANNOUNCE 2
#define TIMER_END_DISPLAY 1
#define TIMER_END_FANFARE 2
#define TIMER_END_TOTAL TIMER_END_ANNOUNCE + TIMER_END_DISPLAY + TIMER_END_FANFARE

rdpq_font_t *font_default;
rdpq_font_t *font_clarendon;

float timer = 0;
hydraharmonics_stage_t stage = STAGE_START;
bool pause = false;
hydraharmonics_speed_t game_speed = NOTE_SPEED_SLOW;

/*==============================
	minigame_init
	The minigame initialization function
==============================*/
void minigame_init()
{
	// Initiate subsystems
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

	// Define some variables
	font_default = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
	font_clarendon = rdpq_font_load("rom:/hydraharmonics/Superclarendon-Regular-01.font64");
	rdpq_text_register_font(FONT_DEFAULT, font_default);
	rdpq_text_register_font(FONT_CLARENDON, font_clarendon);

	// Initiate game elements
	notes_init();
	hydra_init();
	ui_init ();
	intro_init();
	outro_init();
	effects_init();
	audio_sfx_init();
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
	// Handle the stages
	if (stage == STAGE_START) {
		intro_interact();
	} else if (stage == STAGE_GAME) {
		// Check if game is paused
		if (pause) {
			return;
		}
		// Add a new note every second
		float last_timer = timer;
		timer += deltatime;
		if (
			(int)(timer*note_spawn[game_speed]) != (int)(last_timer*note_spawn[game_speed]) &&
			notes_get_remaining(NOTES_GET_REMAINING_UNSPAWNED) &&
			audio_music_get_pattern() < 17
		) {
			notes_check_and_add();
		}
		// Skip to the end if there are no notes left
		if (
			audio_music_get_pattern() == 18 &&
			notes_get_remaining(NOTES_GET_REMAINING_SPAWNED) == 0 &&
			hydra_get_all_animation_states (HYDRA_ANIMATION_NONE)
		) {
			audio_sfx_play(SFX_DRUMROLL);
			notes_destroy_all();
			effects_destroy_all();
			scores_get_winner();
			stage = STAGE_END;
		}
		// Do all the frame-by-frame tasks

		notes_move();
		note_hit_detection();
		hydra_shell_bounce();
		ui_animate();
		effects_animate();
		audio_music_monitor();
	} else if (stage == STAGE_END) {
		outro_interact();
	} else if (stage == STAGE_RETURN_TO_MENU) {
		minigame_end();
	}
}

/*==============================
	minigame_loop
	Code that is called every loop.
	@param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
	// Prepare the RDP
	rdpq_attach(display_get(), NULL);

	// Draw things
	rdpq_set_mode_copy(true);
	ui_draw();
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	hydra_draw();
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	notes_draw();
	ui_signs_draw();
	effects_draw();

	// Things that should only be drawn at particular stages
	if (stage == STAGE_GAME) {
		// Game stage
		hydra_move();
	} else if (stage == STAGE_START) {
		// Intro stage / countdown
		intro_draw();
	} else if (stage == STAGE_END) {
		// Announce a winner
		outro_sign_draw();
	}

	//rdpq_text_printf(NULL, FONT_DEFAULT, 200, 180, "Timer: %f\nRemaining:%i\nStage:%i\n", timer, notes_get_remaining(NOTES_GET_REMAINING_ALL), stage);

	rdpq_detach_show();
	hydra_adjust_hats();
}

/*==============================
	minigame_cleanup
	Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
	// Free allocated memory
	notes_clear();
	hydra_clear();
	scores_clear();
	intro_clear();
	outro_clear();
	ui_clear();
	effects_clear();
	audio_sfx_close();

	// Free the fonts
	rdpq_text_unregister_font(FONT_DEFAULT);
	rdpq_font_free(font_default);
	rdpq_text_unregister_font(FONT_CLARENDON);
	rdpq_font_free(font_clarendon);

	// Close the subsystems
	display_close();
}
