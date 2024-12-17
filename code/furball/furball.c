#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

const MinigameDef minigame_def = {
    .gamename = "FurBall",
    .developername = "Radish64",
    .description = "Kitty Cat Target Attack",
    .instructions = "Press A to shoot, hit the most targets to win!"
};

#define COUNTDOWN_DELAY 3.0
#define GAME_TIME		60.0f
#define WIN_DELAY		5.0f
#define WIN_SHOW_DELAY	2.0f

#ifndef DEBUG
#define DEBUG false
#endif

#define FONT_TEXT           1
#define TEXT_COLOR          0x6CBB3CFF
#define TEXT_OUTLINE        0x30521AFF


surface_t *zBuffer;
rspq_syncpoint_t syncPoint;
T3DViewport viewport;

rdpq_font_t *font;

T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 lightDirVec;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

xm64player_t music;

float countDownTimer;
float gameTimer;
float winTimer;
float winShowTimer;
float aiShotTimer[4];
float aiShotTime;

bool startSoundPlayed;
bool endSoundPlayed;
bool winSoundPlayed;

int winner;
char winnerText[32]; 

int skyColor[3];

bool displayFPS;

typedef struct {
	T3DModel *model;
	T3DMat4FP* modelMatFP;
	rspq_block_t *displayList;
	float modelScale[3];
	float modelRotation[3];
	T3DVec3 modelPosition;
	bool visible;
} staticMesh3d;

typedef struct {
	T3DModel *model;
	T3DMat4FP* modelMatFP;
	T3DSkeleton skeleton;
	T3DSkeleton skeletonBlend;
	T3DAnim animation;
	rspq_block_t *displayList;
	float modelScale[3];
	float modelRotation[3];
	T3DVec3 modelPosition;
	bool visible;
} skeletalMesh3d;


typedef struct {
	skeletalMesh3d mesh;
	int score;
	bool isHuman;
} player_data;

typedef struct {
	staticMesh3d mesh;
	bool go;
} ball_data;

void staticMesh3d_init(staticMesh3d * mesh, char * path, float scale, T3DVec3 position, color_t color){
	mesh->model = t3d_model_load(path);
	mesh->modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
	for (int i = 0; i < 3; ++i){
		mesh->modelScale[i] = scale;
		mesh->modelRotation[i] = 0.0f;
	}
	mesh->modelPosition = position;
	t3d_mat4fp_from_srt_euler(mesh->modelMatFP,
		mesh->modelScale,
		mesh->modelRotation,
		mesh->modelPosition.v
	);
	rspq_block_begin();
		t3d_matrix_push(mesh->modelMatFP);
		rdpq_set_prim_color(color);
		t3d_model_draw(mesh->model);
		t3d_matrix_pop(1);
	mesh->displayList = rspq_block_end();
	mesh->visible = true;
}

void skeletalMesh3d_init(skeletalMesh3d * mesh, char * path, float scale, T3DVec3 position, color_t color, char * anim){
	mesh->model = t3d_model_load(path);
	mesh->modelMatFP = malloc_uncached(sizeof(T3DMat4FP));

	for (int i = 0; i < 3; ++i){
		mesh->modelScale[i] = scale;
		mesh->modelRotation[i] = 0.0f;
	}

	mesh->modelPosition = position;
	t3d_mat4fp_from_srt_euler(mesh->modelMatFP,
		mesh->modelScale,
		mesh->modelRotation,
		mesh->modelPosition.v
	);

	mesh->skeleton = t3d_skeleton_create(mesh->model);
  	mesh->skeletonBlend = t3d_skeleton_clone(&mesh->skeleton, false); // optimiz for blending, has no matrices

	mesh->animation = t3d_anim_create(mesh->model, anim);
  	t3d_anim_set_looping(&mesh->animation, false); // don't loop this animation
  	t3d_anim_set_playing(&mesh->animation, true); // start in a paused state
  	t3d_anim_set_time(&mesh->animation, 0.0f); // start in a paused state
	t3d_anim_attach(&mesh->animation, &mesh->skeleton);


	rspq_block_begin();
		t3d_matrix_push(mesh->modelMatFP);
		rdpq_set_prim_color(color);
		t3d_model_draw_skinned(mesh->model, &mesh->skeleton);
		t3d_matrix_pop(1);
	mesh->displayList = rspq_block_end();
	mesh->visible = true;
}


void player_init(player_data * player, T3DVec3 position, color_t color, bool isHuman, char * path, char * anim){
	skeletalMesh3d_init(&(player->mesh), path, 0.075f, position, color, anim);
	player->isHuman = isHuman;
	player->score = 0;
	//player->rotationFactor = 60.0f;
}

void ball_init(ball_data * ball, color_t color){
	staticMesh3d_init(&(ball->mesh), "rom:/furball/ball.t3dm", 0.075f, (T3DVec3){{0,0,0}}, color);
	ball->go = false;
	ball->mesh.visible = false;
}

void ball_reset(ball_data * ball){
	for(int i = 0; i < 3; ++i){
		ball->mesh.modelPosition = (T3DVec3){{0.0f,0.0f,0.0f}};
	}
	ball->go = false;
	ball->mesh.visible = false;
}

bool collide(staticMesh3d * this, staticMesh3d * other, float radius){
	float x = fabs(this->modelPosition.v[0] - other->modelPosition.v[0]);
	float y = fabs(this->modelPosition.v[1] - other->modelPosition.v[1]);
	float z = fabs(this->modelPosition.v[2] - other->modelPosition.v[2]);
	if(sqrtf(x*x + y*y + z*z) <= radius){
		return true;
	}
	return false;
}

bool collide_xy(staticMesh3d * this, staticMesh3d * other, float radius){
	float x = fabs(this->modelPosition.v[0] - other->modelPosition.v[0]);
	float y = fabs(this->modelPosition.v[1] - other->modelPosition.v[1]);
	if(sqrtf(x*x + y*y) <= radius){
		return true;
	}
	return false;
}

void drawStaticMesh(staticMesh3d * mesh){
	if(mesh->visible){
		t3d_mat4fp_from_srt_euler(mesh->modelMatFP,
			mesh->modelScale,
			mesh->modelRotation,
			mesh->modelPosition.v
		);
		rspq_block_run(mesh->displayList);
	}
}

void drawSkeletalMesh(skeletalMesh3d * mesh, float deltatime){
	if(mesh->visible){
		if(mesh->animation.isPlaying){
			t3d_anim_update(&mesh->animation, deltatime);
			t3d_skeleton_blend(&mesh->skeleton, &mesh->skeleton, &mesh->skeletonBlend, 0.15f);
		}

		if(syncPoint)rspq_syncpoint_wait(syncPoint);

		if(mesh->animation.isPlaying){
			t3d_skeleton_update(&mesh->skeleton);
		}

		t3d_mat4fp_from_srt_euler(mesh->modelMatFP,
			mesh->modelScale,
			mesh->modelRotation,
			mesh->modelPosition.v
		);
		rspq_block_run(mesh->displayList);
	}
}


player_data players[MAXPLAYERS];
ball_data balls[MAXPLAYERS];
staticMesh3d targets[MAXPLAYERS];
staticMesh3d ladders[MAXPLAYERS];
staticMesh3d wall;

color_t colors[4];

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init(){

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
	display_set_fps_limit(display_get_refresh_rate()*0.5f);
	rdpq_init();
	zBuffer = display_get_zbuf();
	syncPoint = 0;
	t3d_init((T3DInitParams){});
	font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
  	rdpq_text_register_font(FONT_TEXT, font);

  	lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
	
	viewport = t3d_viewport_create();
	camPos = (T3DVec3){{0,0,90.0f}};
	camTarget = (T3DVec3){{0,0,0}};

  	wav64_open(&sfx_start, "rom:/core/Start.wav64");
  	wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
  	wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
  	wav64_open(&sfx_winner, "rom:/core/Winner.wav64");

	uint32_t playercount = core_get_playercount();

	T3DVec3 start_positions[] = {
		(T3DVec3){{-80,0,0}},
		(T3DVec3){{-20,0,0}},
		(T3DVec3){{20,0,0}},
		(T3DVec3){{80,0,0}}
	};

	T3DVec3 ladder_positions[] = {
		(T3DVec3){{-100,0,-5}},
		(T3DVec3){{-40,0,-5}},
		(T3DVec3){{40,0,-5}},
		(T3DVec3){{100,0,-5}}
	};

	colors[0] = PLAYERCOLOR_1;
	colors[1] = PLAYERCOLOR_2;
	colors[2] = PLAYERCOLOR_3;
	colors[3] = PLAYERCOLOR_4;

	for(int i = 0; i < MAXPLAYERS; ++i){
		if(i < 2){
			player_init(&players[i], start_positions[i], colors[i], i < playercount, "rom:/furball/fastcat.t3dm", "ThrowRight");
		}
		else{
			player_init(&players[i], start_positions[i], colors[i], i < playercount, "rom:/furball/fastcat.t3dm", "ThrowLeft");
		}
		players[i].mesh.modelRotation[1] = T3D_DEG_TO_RAD(270.0f);
		ball_init(&balls[i], colors[i]);
		staticMesh3d_init(&targets[i], "rom:/furball/target.t3dm", 0.25f, start_positions[i], RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
		staticMesh3d_init(&ladders[i], "rom:/furball/fastladder.t3dm", 0.05f, ladder_positions[i], RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
	
		targets[i].modelRotation[1] = T3D_DEG_TO_RAD(90.0f);
		ladders[i].modelRotation[1] = T3D_DEG_TO_RAD(90.0f);
		targets[i].modelPosition.v[2] = -50;
	}

	staticMesh3d_init(&wall, "rom:/furball/wall.t3dm", 0.25f, (T3DVec3){{0,0,-200}}, RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
	wall.modelRotation[1] = T3D_DEG_TO_RAD(90.0f);

	countDownTimer = COUNTDOWN_DELAY;
	gameTimer = GAME_TIME;
	winTimer = WIN_DELAY;
	winShowTimer = WIN_SHOW_DELAY;
	switch (core_get_aidifficulty()){
		case(DIFF_EASY):
			aiShotTime = 0.5f;
			break;
		case(DIFF_MEDIUM):
			aiShotTime = 0.3f;
			break;
		case(DIFF_HARD):
			aiShotTime = 0.10f;
			break;
		default:
			aiShotTime = 0.0f;
			break;
	}
	for(int i = 0; i < 4; ++i){
		aiShotTimer[i] = aiShotTime;
	}

	skyColor[0] = 0xAA;
	skyColor[1] = 0xAA;
	skyColor[2] = 0xFF;

	startSoundPlayed = false;
	endSoundPlayed = false;
	winSoundPlayed = false;

	displayFPS = DEBUG;

  	xm64player_open(&music, "rom:/furball/crystal.xm64");
  	xm64player_play(&music, 0);
	xm64player_set_vol(&music, 0.5);
	xm64player_set_loop(&music,true);

}

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is 
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/
void minigame_fixedloop(float deltatime){

	if(countDownTimer > 0.0f){
		float prevCountDown = countDownTimer;
		countDownTimer -= deltatime;
    	if ((int)prevCountDown != (int)countDownTimer && countDownTimer >= 0){
    		wav64_play(&sfx_countdown, 31);
		}
	}

	else if(gameTimer > 0.0f){
		if(!startSoundPlayed){
    		wav64_play(&sfx_start, 31);
			startSoundPlayed = true;
		}
		gameTimer -= deltatime;

		joypad_inputs_t sticks[MAXPLAYERS]; 
		joypad_buttons_t buttons[MAXPLAYERS];

		for(int i = 0; i < MAXPLAYERS; ++i){
			sticks[i] = joypad_get_inputs(core_get_playercontroller(i));
			buttons[i] = joypad_get_buttons_pressed(core_get_playercontroller(i));
		}

		for(int i = 0; i < MAXPLAYERS; ++i){
			//quit if start is pressed
			if(buttons[i].start) minigame_end();

			//movement
			if(players[i].isHuman){
				if(abs(sticks[i].stick_y) > 10){
					players[i].mesh.modelPosition.v[1] += sticks[i].stick_y * 0.1f;
					if(players[i].mesh.modelPosition.v[1] >= 100.0f){
						players[i].mesh.modelPosition.v[1] = 100.0f;
					}
					else if(players[i].mesh.modelPosition.v[1] <= -100.0f){
						players[i].mesh.modelPosition.v[1] = -100.0f;
					}
				}
			}
			else{
				if(!balls[i].go){
					if (fabs(players[i].mesh.modelPosition.v[1] - targets[i].modelPosition.v[1]) < 10.0f){
						if(aiShotTimer[i] > 0){
							aiShotTimer[i] -= deltatime;
						}
						else{
							t3d_anim_set_playing(&players[i].mesh.animation, true);
							t3d_anim_set_time(&players[i].mesh.animation, 0.0f);

							balls[i].mesh.modelPosition = players[i].mesh.modelPosition;
							balls[i].go = true;
							balls[i].mesh.visible = true;
							aiShotTimer[i] = aiShotTime;
						}
					}	
					else{
						if(players[i].mesh.modelPosition.v[1] < targets[i].modelPosition.v[1]){
							players[i].mesh.modelPosition.v[1] += 2.0f;
						}
						else if(players[i].mesh.modelPosition.v[1] > targets[i].modelPosition.v[1]){
							players[i].mesh.modelPosition.v[1] -= 2.0f;
						}
					}
				}
			}

			//ball go
			if(balls[i].go) balls[i].mesh.modelPosition.v[2] -= 200*deltatime;
			if(balls[i].mesh.modelPosition.v[2] < -200){
				ball_reset(&(balls[i]));
			}

			//hit target
			if(collide(&(balls[i].mesh), &targets[i], 20.0f)){
				ball_reset(&(balls[i]));
				targets[i].modelPosition.v[1] = (float)pow(-1,(int)(rand()%3)) * (float)rand()/((float)RAND_MAX/75);
				++players[i].score;
			}
		}
	}
	else{
  		xm64player_stop(&music);
		if(!endSoundPlayed){
      		wav64_play(&sfx_stop, 31);
			endSoundPlayed = true;
		}
		winShowTimer -= deltatime;
		winTimer -= deltatime;
		if(winTimer < 0.0f){
			minigame_end();
		}
		else if (winShowTimer < 0.0f){
			if(!winSoundPlayed){
				wav64_play(&sfx_winner, 31);
				winSoundPlayed = true;
			}
			winner = 0;
			for(int i = 0; i < MAXPLAYERS; ++i){
				if (players[i].score > players[winner].score){
					winner = i;
				}
			}
			core_set_winner(winner);
			sprintf(winnerText,"PLAYER %d WINS!", winner+1);
			players[winner].mesh.modelPosition.v[0] = 0.0f;
			players[winner].mesh.modelPosition.v[1] = -15.0f;
			players[winner].mesh.modelPosition.v[2] = 35.0f;
			players[winner].mesh.modelRotation[1] = T3D_DEG_TO_RAD(90.0f);
		}
	}
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime){

	//shoot the ball (has to go here for some reason or sometimes doesn't shoot)
	joypad_buttons_t buttons[MAXPLAYERS];

	for(int i = 0; i < MAXPLAYERS; ++i){
		buttons[i] = joypad_get_buttons_pressed(core_get_playercontroller(i));
	}
	for(int i = 0; i < MAXPLAYERS; ++i){
		if(countDownTimer <= 0.0f){
			if(players[i].isHuman){
				if(buttons[i].a) {
					t3d_anim_set_playing(&players[i].mesh.animation, true);
					t3d_anim_set_time(&players[i].mesh.animation, 0.0f);

					balls[i].mesh.modelPosition = players[i].mesh.modelPosition;
					balls[i].go = true;
					balls[i].mesh.visible = true;
				}
			}
		}
	}

  	uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
  	uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

	t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f),20.0f,160.f);
	t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

	zBuffer = display_get_zbuf();
    rdpq_attach(display_get(), zBuffer);

  	if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

	t3d_frame_start();
	t3d_viewport_attach(&viewport);
	//t3d_screen_clear_color(RGBA32(skyColor[0],skyColor[1],skyColor[2],0xFF));
	t3d_screen_clear_depth();

  	t3d_light_set_ambient(colorAmbient);
  	t3d_light_set_directional(0, colorDir, &lightDirVec);
  	t3d_light_set_count(1);


	if(winShowTimer > 0.0f){
		rdpq_mode_zbuf(false,false);
  		if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
		drawStaticMesh(&wall);
		for(int i = 0; i < MAXPLAYERS; ++i){
			drawStaticMesh(&targets[i]);
		}
		for(int i = 0; i < MAXPLAYERS; ++i){
			drawStaticMesh(&balls[i].mesh);
		}
		for(int i = 0; i < MAXPLAYERS; ++i){
			drawStaticMesh(&ladders[i]);
		}
		rdpq_mode_zbuf(true,true);
  		if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
		for(int i = 0; i < MAXPLAYERS; ++i){
			drawSkeletalMesh(&players[i].mesh, deltatime);
		}

		for(int i = 0; i < MAXPLAYERS; ++i){
			int textOffset = 0;
			switch(i){
				case 1:
					textOffset = 70;
					break;
				case 2:
					textOffset = 165;
					break;
				case 3:
					textOffset = 235;
					break;
			}

  			rdpq_font_style(font, 0, &(rdpq_fontstyle_t){
				.color = colors[i],
				.outline_color = RGBA32(0,0,0,0xFF)
			});

			char strScore[8];
			sprintf(strScore,"%d",players[i].score);
			//sprintf(strScore,"%f",players[i].mesh.modelPosition.v[1]);
			rdpq_text_printf(
				NULL, 1, 20 + textOffset, 220,
				strScore
			);

  			rdpq_font_style(font, 0, &(rdpq_fontstyle_t){
				.color = RGBA32(0xFF,0xFF,0xFF,0xFF),
				.outline_color = RGBA32(0,0,0,0xFF)
			});

			char textFPS[8];
			if(displayFPS){
				//sprintf(textFPS,"%.0f FPS", 1 / deltatime);
				sprintf(textFPS,"%.0f FPS", display_get_fps());
				rdpq_text_printf(
					NULL, 1, 260, 20,
					textFPS
				);
			}

			char textTimer[8];
			sprintf(textTimer,"%.1f", fabs(gameTimer));
			rdpq_text_printf(
				NULL, 1, 150, 20,
				textTimer
			);
		}
	}
	else{
			drawSkeletalMesh(&players[winner].mesh, deltatime);
			drawStaticMesh(&wall);

  			rdpq_font_style(font, 0, &(rdpq_fontstyle_t){
				.color = colors[winner],
				.outline_color = RGBA32(0,0,0,0xFF)
			});
			rdpq_text_printf(
				NULL, 1, 120, 60,
				winnerText
			);

		}

	syncPoint = rspq_syncpoint_new();

	rdpq_sync_tile();
	rdpq_sync_pipe();

	rdpq_detach_show();
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup(){
	for(int i = 0; i < MAXPLAYERS; ++i){
		rspq_block_free(players[i].mesh.displayList);
		free_uncached(players[i].mesh.modelMatFP);
		t3d_model_free(players[i].mesh.model);
		t3d_skeleton_destroy(&players[i].mesh.skeleton);
		t3d_skeleton_destroy(&players[i].mesh.skeletonBlend);
		t3d_anim_destroy(&players[i].mesh.animation);


		rspq_block_free(balls[i].mesh.displayList);
		free_uncached(balls[i].mesh.modelMatFP);
		t3d_model_free(balls[i].mesh.model);

		rspq_block_free(targets[i].displayList);
		free_uncached(targets[i].modelMatFP);
		t3d_model_free(targets[i].model);

		rspq_block_free(ladders[i].displayList);
		free_uncached(ladders[i].modelMatFP);
		t3d_model_free(ladders[i].model);

	}

	rspq_block_free(wall.displayList);
	free_uncached(wall.modelMatFP);
	t3d_model_free(wall.model);

  	wav64_close(&sfx_start);
  	wav64_close(&sfx_countdown);
  	wav64_close(&sfx_stop);
  	wav64_close(&sfx_winner);

  	xm64player_stop(&music);
  	xm64player_close(&music);

  	rdpq_text_unregister_font(FONT_TEXT);
  	rdpq_font_free(font);

	t3d_destroy();
	display_close();
}
