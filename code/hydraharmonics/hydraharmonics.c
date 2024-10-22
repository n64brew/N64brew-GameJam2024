#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

const MinigameDef minigame_def = {
	.gamename = "A Hydra Harmonics",
	.developername = "Catch-64",
	.description = "Who is the best singer?",
	.instructions = "Grab the most notes to win."
};


#define FONT_TEXT 1
#define PLAYER_MAX 4
#define HEAD_STATES_MAX 4

rdpq_font_t *font;

sprite_t* head_sprites[PLAYER_MAX][HEAD_STATES_MAX];

/*==============================
	minigame_init
	The minigame initialization function
==============================*/
void minigame_init()
{
	// Define some variables
	uint8_t i, j;
	char temptext[64];

	// Initiate subsystems
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
	font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
	rdpq_text_register_font(FONT_TEXT, font);

	// Load the head head_sprites
	for (i=0; i<PLAYER_MAX; i++) {
		for (j=0; j<HEAD_STATES_MAX; j++) {
			sprintf(temptext, "rom:/hydraharmonics/head-%i-%i.ci4.sprite", i, j);
			head_sprites[i][j] = sprite_load(temptext);
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
	rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 155, 100, "%s", "Let's play a game");

	rdpq_set_mode_standard();
	for (i=0; i<PLAYER_MAX; i++) {
		for (j=0; j<HEAD_STATES_MAX; j++) {
			rdpq_sprite_blit(head_sprites[i][j], 10+i*head_sprites[i][j]->width, 10+j*head_sprites[i][j]->height, NULL);
		}
	}
	rdpq_detach_show();
}

/*==============================
	minigame_cleanup
	Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{

}
