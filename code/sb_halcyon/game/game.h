#ifndef GAME_H
#define GAME_H

enum GAME_STATES
{
	INTRO,
	MAIN_MENU,
	CHARACTER_SELECT,
	GAMEPLAY,
	PAUSE,
	GAME_OVER
};

typedef struct
{

	uint8_t state;
	Screen screen;
	TimeData timing;
	rspq_syncpoint_t syncPoint;
	int diff;
	int8_t winTimer;
	uint8_t winnerID;
	uint8_t countdownTimer;
	uint8_t humanCount;
	uint8_t deadPool;
	bool actorSet;
	bool winnerSet;
	Scene scene;

} Game;

void game_init(Game *game);

void game_init(Game *game)
{

	screen_initDisplay(&game->screen);
	screen_initT3dViewport(&game->screen);

	t3d_init((T3DInitParams){});

	// TPX
	ptx_init(&cloudMist);

	time_init(&game->timing);

	scene_init(&game->scene);

	//
	ui_init();
	sound_load();
	//

	game->countdownTimer = 150; // Oops, forget to set this
	game->syncPoint = 0;
	game->state = INTRO;
	game->humanCount = core_get_playercount();
	game->deadPool = 0;
}

#endif