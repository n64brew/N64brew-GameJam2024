#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>
#include "./snakeplayer.h"
#include "./dirtblock.h"
#include "./chest.h"

const MinigameDef minigame_def = {
  .gamename = "Underground Grind",
  .developername = "raisedwizardry",
  .description = "Find the treasure by grinding it out underground in a flat 3 x 3 grid",
  .instructions = "Press A and Z in squence to dig through the blocks. Use the control stick move around"
};

#define FONT_TEXT           1
#define FONT_BILLBOARD      2
#define TEXT_COLOR          0x6CBB3CFF
#define TEXT_OUTLINE        0x30521AFF

#define HITBOX_RADIUS       19.f

#define ATTACK_OFFSET       10
#define ATTACK_RADIUS       5.f

#define ATTACK_TIME_START   0.333f
#define ATTACK_TIME_END     0.4f

#define COUNTDOWN_DELAY     3.0f
#define GO_DELAY            1.0f
#define WIN_DELAY           5.0f
#define WIN_SHOW_DELAY      2.0f

#define TOTAL_BLOCKS        9

#define BILLBOARD_YOFFSET   15.0f

/**
 * Example project showcasing the usage of the animation system.
 * This includes instancing animations, blending animations, and controlling playback.
 */

surface_t *depthBuffer;
T3DViewport viewport;
rdpq_font_t *font;
rdpq_font_t *fontBillboard;
T3DMat4FP* mapMatFP;
rspq_block_t *dplMap;
T3DModel *snakeModel;
T3DModel *shadowModel;
T3DModel *modelMap;
T3DModel *dirtBlockModel;
T3DModel *chestModel;
T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 lightDirVec;
xm64player_t music;

int chestBlockNumber;

SnakePlayer players[MAXPLAYERS];

DirtBlock dirtBlocks[TOTAL_BLOCKS];

Chest chests[1];

float countDownTimer;
bool isEnding;
float endTimer;
PlyNum winner;
int blockGridSize;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

rspq_syncpoint_t syncPoint;

void minigame_init(void)
{
  chestBlockNumber = rand() % TOTAL_BLOCKS;
  
  int diff = core_get_aidifficulty();
  if (diff == DIFF_EASY) { blockGridSize = 3;}
  if (diff == DIFF_MEDIUM) { blockGridSize = 4;}
  if (diff == DIFF_HARD) { blockGridSize = 5;}

  const color_t colors[] = {
    PLAYERCOLOR_1,
    PLAYERCOLOR_2,
    PLAYERCOLOR_3,
    PLAYERCOLOR_4,
  };

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
  depthBuffer = display_get_zbuf();

  t3d_init((T3DInitParams){});

  font = rdpq_font_load("rom:/undergroundgrind/m6x11plus.font64");
  rdpq_text_register_font(FONT_TEXT, font);
  rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = color_from_packed32(TEXT_COLOR) });

  fontBillboard = rdpq_font_load("rom:/squarewave.font64");
  rdpq_text_register_font(FONT_BILLBOARD, fontBillboard);
  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    rdpq_font_style(fontBillboard, i, &(rdpq_fontstyle_t){ .color = colors[i] });
  }

  viewport = t3d_viewport_create();

  mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0, 0, -10});

  camPos = (T3DVec3){{0, 125.0f, 100.0f}};
  camTarget = (T3DVec3){{0, 0, 40}};

  lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
  t3d_vec3_norm(&lightDirVec);

  modelMap = t3d_model_load("rom:/undergroundgrind/map.t3dm");
  shadowModel = t3d_model_load("rom:/undergroundgrind/shadow.t3dm");

  // Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html
  snakeModel = t3d_model_load("rom:/undergroundgrind/snake.t3dm");
  
  dirtBlockModel = t3d_model_load("rom:/undergroundgrind/one-by-one.t3dm");

  chestModel = t3d_model_load("rom:/undergroundgrind/chest.t3dm");

  rspq_block_begin();
    t3d_matrix_push(mapMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(modelMap);
    t3d_matrix_pop(1);
  dplMap = rspq_block_end();

  T3DVec3 start_positions[] = {
    (T3DVec3){{-100,0.15f,0}},
    (T3DVec3){{0,0.15f,-100}},
    (T3DVec3){{100,0.15f,0}},
    (T3DVec3){{0,0.15f,100}},
  };

  float start_rotations[] = {
    M_PI/2,
    0,
    3*M_PI/2,
    M_PI
  };

  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    initSnakePlayer(&players[i], colors[i], start_positions[i], start_rotations[i], shadowModel, snakeModel);
    players[i].plynum = i;
  }

  T3DVec3 blockPositions[] = {
  	(T3DVec3){{41.0f, 0.0f, 41.0f}},
  	(T3DVec3){{41.0f, 0.0f, 0.0f}},
	(T3DVec3){{41.0f, 0.0f, -41.0f}},
	(T3DVec3){{0.0f, 0.0f, 41.0f}},
	(T3DVec3){{0.0f, 0.0f, 0.0f}},
	(T3DVec3){{0.0f, 0.0f, -41.0f}},
	(T3DVec3){{-41.0f, 0.0f, 41.0f}},
	(T3DVec3){{-41.0f, 0.0f, 0.0f}},
	(T3DVec3){{-41.0f, 0.0f, -41.0f}}
  };

  float blockScale = 0.5f;
  
  for (size_t i = 0; i < TOTAL_BLOCKS; i++)
  {
  	initDirtBlock(&dirtBlocks[i], dirtBlockModel, blockScale, RGBA32(255, 0, 0, 255), blockPositions[i]);
  }

  initChest(&chests[0], chestModel, 0.4f, RGBA32(255, 0, 0, 255), blockPositions[chestBlockNumber], chestBlockNumber);

  dirtBlocks[chestBlockNumber].isContainingChest = true;
  
  countDownTimer = COUNTDOWN_DELAY;

  syncPoint = 0;
  wav64_open(&sfx_start, "rom:/core/Start.wav64");
  wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
  wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
  wav64_open(&sfx_winner, "rom:/core/Winner.wav64");
  xm64player_open(&music, "rom:/undergroundgrind/bottled_bubbles.xm64");
  xm64player_play(&music, 0);
}

void player_do_damage(SnakePlayer *player)
{
  float s, c;
  fm_sincosf(player->rotY, &s, &c);
  float attackPosition[] = {
    player->playerPos.v[0] + s * ATTACK_OFFSET,
    player->playerPos.v[2] + c * ATTACK_OFFSET,
  };

  for (size_t i = 0; i < TOTAL_BLOCKS; i++)
  {
    DirtBlock *block = &dirtBlocks[i];
    if (block->isDestroyed) continue;

    float positionDifference[] = {
      block->dirtBlockPos.v[0] - attackPosition[0],
      block->dirtBlockPos.v[2] - attackPosition[1],
    };

    float distance = sqrtf(positionDifference[0]*positionDifference[0] + positionDifference[1]*positionDifference[1]);

    if (distance < (ATTACK_RADIUS + HITBOX_RADIUS)) {
      block->damage = block->damage + 13 + player->comboBonus;
    }

    if (block->damage > 100) {
      block->destroyingPlayer = player->plynum;
	  block->isDestroyed = true;
	} 
  }
}

bool player_has_control(SnakePlayer *player)
{
  return countDownTimer < 0.0f;
}

void player_fixedloop(SnakePlayer *player, float deltaTime, joypad_port_t port, bool is_human)
{
  float speed = 0.0f;
  T3DVec3 newDir = {0};

  if (player_has_control(player)) {
    if (is_human) {
      joypad_inputs_t joypad = joypad_get_inputs(port);

      newDir.v[0] = (float)joypad.stick_x * 0.05f;
      newDir.v[2] = -(float)joypad.stick_y * 0.05f;
      speed = sqrtf(t3d_vec3_len2(&newDir));
    } else {
      DirtBlock* target = &dirtBlocks[player->ai_target];
      if (!target->isDestroyed) { // Check for a valid target
        // Move towards the direction of the target
        float dist, norm;
        newDir.v[0] = (target->dirtBlockPos.v[0] - player->playerPos.v[0]);
        newDir.v[2] = (target->dirtBlockPos.v[2] - player->playerPos.v[2]);
        dist = sqrtf(newDir.v[0]*newDir.v[0] + newDir.v[2]*newDir.v[2]);
        norm = 1/dist;
        newDir.v[0] *= norm;
        newDir.v[2] *= norm;
        speed = 20;
    
        // Attack if close, and the reaction time has elapsed
        if (dist < 25 && !player->isAttack) {
          if (player->ai_reactionspeed <= 0) {
            t3d_anim_set_playing(&player->animAttack, true);
            t3d_anim_set_time(&player->animAttack, 0.0f);
            player->isAttack = true;
            player->attackTimer = 0;
            player->ai_reactionspeed = (2-core_get_aidifficulty())*5 + rand()%((3-core_get_aidifficulty())*3);
          } else {
            player->ai_reactionspeed--;
          }
        }
      } else {
        player->ai_target = rand() % 9; // (Attempt) to aquire a new target this frame
      }
    }
  }

  // Player movement
  if(speed > 0.15f && !player->isAttack) {
    newDir.v[0] /= speed;
    newDir.v[2] /= speed;
    player->moveDir = newDir;

    float newAngle = atan2f(player->moveDir.v[0], player->moveDir.v[2]);
    player->rotY = t3d_lerp_angle(player->rotY, newAngle, 0.5f);
    player->currSpeed = t3d_lerp(player->currSpeed, speed * 0.3f, 0.15f);
  } else {
    player->currSpeed *= 0.64f;
  }

  // use blend based on speed for smooth transitions
  player->animBlend = player->currSpeed / 0.51f;
  if(player->animBlend > 1.0f)player->animBlend = 1.0f;

  // move player...
  player->playerPos.v[0] += player->moveDir.v[0] * player->currSpeed;
  player->playerPos.v[2] += player->moveDir.v[2] * player->currSpeed;
  // ...and limit position inside the box
  const float BOX_SIZE = 140.0f;
  if(player->playerPos.v[0] < -BOX_SIZE)player->playerPos.v[0] = -BOX_SIZE;
  if(player->playerPos.v[0] >  BOX_SIZE)player->playerPos.v[0] =  BOX_SIZE;
  if(player->playerPos.v[2] < -BOX_SIZE)player->playerPos.v[2] = -BOX_SIZE;
  if(player->playerPos.v[2] >  BOX_SIZE)player->playerPos.v[2] =  BOX_SIZE;

  if (player->isAttack) {
    player->attackTimer += deltaTime;
    if (player->attackTimer > ATTACK_TIME_START && player->attackTimer < ATTACK_TIME_END) {
      player_do_damage(player);
    }
  }
}

void player_loop(SnakePlayer *player, float deltaTime, joypad_port_t port, bool is_human)
{
  if (is_human && player_has_control(player))
  {
    joypad_buttons_t btn = joypad_get_buttons_pressed(port);

    if (btn.start) minigame_end();

    // Player Dig
    if ((((btn.z) && player->previousButtonPressed == btn.a) || ((btn.a) && player->previousButtonPressed == btn.z))  && !player->animAttack.isPlaying) {
      t3d_anim_set_playing(&player->animAttack, true);
      t3d_anim_set_time(&player->animAttack, 0.0f);
      player->isAttack = true;
      player->attackTimer = 0;
      if (btn.a) {
        player->previousButtonPressed = btn.a;
      }
      if (btn.z) {
        player->previousButtonPressed = btn.z;
      }
      player->comboBonus = 15;
    }
    if ((btn.z || btn.a) && !player->animAttack.isPlaying) {
      t3d_anim_set_playing(&player->animAttack, true);
      t3d_anim_set_time(&player->animAttack, 0.0f);
      player->isAttack = true;
      player->attackTimer = 0;
      player->previousButtonPressed = btn.z;
      player->comboBonus = 0;
    }

    if((btn.b) && !player->animJump.isPlaying) {
      t3d_anim_set_playing(&player->animJump, true);
      t3d_anim_set_time(&player->animJump, 0.0f);
      player->isJump = true;
      player->jumpTimer = 0;
    }

  }
  
  // Update the animation and modify the skeleton, this will however NOT recalculate the matrices
  t3d_anim_update(&player->animIdle, deltaTime);
  t3d_anim_set_speed(&player->animWalk, player->animBlend + 0.15f);
  t3d_anim_update(&player->animWalk, deltaTime);

  if(player->isAttack) {
    t3d_anim_update(&player->animAttack, deltaTime); // attack animation now overrides the idle one
    if(!player->animAttack.isPlaying)player->isAttack = false;
  }

  if(player->isJump) {
    t3d_anim_update(&player->animJump, deltaTime); // attack animation now overrides the idle one
    if(!player->animJump.isPlaying)player->isJump = false;
  }

  // We now blend the walk animation with the idle/attack one
  t3d_skeleton_blend(&player->skel, &player->skel, &player->skelBlend, player->animBlend);

  if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

  // Now recalc. the matrices, this will cause any model referencing them to use the new pose
  t3d_skeleton_update(&player->skel);

  // Update player matrix
  t3d_mat4fp_from_srt_euler(player->modelMatFP,
    (float[3]){0.125f, 0.125f, 0.125f},
    (float[3]){0.0f, -player->rotY, 0},
    player->playerPos.v
  );
}

void player_draw(SnakePlayer *player)
{
  rspq_block_run(player->dplSnake);
}

void dirtBlockDraw(DirtBlock *dirtBlock)
{
  if (!dirtBlock->isDestroyed) {
  	rspq_block_run(dirtBlock->dplDirtBlock);
  }

  if (dirtBlock->isDestroyed && dirtBlock->isContainingChest) {
    rspq_block_run(chests[0].dplChestBlock);
  }
}

void player_draw_billboard(SnakePlayer *player, PlyNum playerNum)
{
  T3DVec3 billboardPos = (T3DVec3){{
    player->playerPos.v[0],
    player->playerPos.v[1] + BILLBOARD_YOFFSET,
    player->playerPos.v[2]
  }};

  T3DVec3 billboardScreenPos;
  t3d_viewport_calc_viewspace_pos(&viewport, &billboardScreenPos, &billboardPos);

  int x = floorf(billboardScreenPos.v[0]);
  int y = floorf(billboardScreenPos.v[1]);

  rdpq_sync_pipe(); // Hardware crashes otherwise
  rdpq_sync_tile(); // Hardware crashes otherwise

  rdpq_text_printf(&(rdpq_textparms_t){ .style_id = playerNum }, FONT_BILLBOARD, x-5, y-16, "P%d", playerNum+1);
}

void minigame_fixedloop(float deltaTime)
{
  bool controlbefore = player_has_control(&players[0]);
  uint32_t playercount = core_get_playercount();
  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    player_fixedloop(&players[i], deltaTime, core_get_playercontroller(i), i < playercount);
  }

  if (countDownTimer > -GO_DELAY)
  {
    float prevCountDown = countDownTimer;
    countDownTimer -= deltaTime;
    if ((int)prevCountDown != (int)countDownTimer && countDownTimer >= 0)
      wav64_play(&sfx_countdown, 31);
  }
  if (!controlbefore && player_has_control(&players[0]))
    wav64_play(&sfx_start, 31);

  if (!isEnding) {
    // Determine if a player has won
    PlyNum lastPlayer = -1;
    for (size_t i = 0; i < TOTAL_BLOCKS; i++)
    {
      if (dirtBlocks[i].isDestroyed && dirtBlocks[i].isContainingChest)
      {
        lastPlayer = dirtBlocks[i].destroyingPlayer;
      }
    }
    
    if (lastPlayer != -1) {
      isEnding = true;
      winner = lastPlayer;
      wav64_play(&sfx_stop, 31);
    }
  } else {
    float prevEndTime = endTimer;
    endTimer += deltaTime;
    if ((int)prevEndTime != (int)endTimer && (int)endTimer == WIN_SHOW_DELAY)
        wav64_play(&sfx_winner, 31);
    if (endTimer > WIN_DELAY) {
      core_set_winner(winner);
      minigame_end();
    }
  }
}

void minigame_loop(float deltaTime)
{
  uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
  uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

  t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(90.0f), 20.0f, 160.0f);
  t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

  uint32_t playercount = core_get_playercount();
  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    player_loop(&players[i], deltaTime, core_get_playercontroller(i), i < playercount);
  }

  // ======== Draw (3D) ======== //
  rdpq_attach(display_get(), depthBuffer);
  t3d_frame_start();
  t3d_viewport_attach(&viewport);

  t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
  t3d_screen_clear_depth();

  t3d_light_set_ambient(colorAmbient);
  t3d_light_set_directional(0, colorDir, &lightDirVec);
  t3d_light_set_count(1);

  rspq_block_run(dplMap);
  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    player_draw(&players[i]);
  }

  for (size_t i = 0; i < TOTAL_BLOCKS; i++)
  {
    dirtBlockDraw(&dirtBlocks[i]);
  }

  syncPoint = rspq_syncpoint_new();

  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    player_draw_billboard(&players[i], i);
  }

  rdpq_sync_tile();
  rdpq_sync_pipe(); // Hardware crashes otherwise

  if (countDownTimer > 0.0f) {
    rdpq_text_printf(NULL, FONT_TEXT, 155, 100, "%d", (int)ceilf(countDownTimer));
  } else if (countDownTimer > -GO_DELAY) {
    rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, };
    rdpq_text_print(&textparms, FONT_TEXT, 0, 100, "GO!");
  } else if (isEnding && endTimer >= WIN_SHOW_DELAY) {
    rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = 320, };
    rdpq_text_printf(&textparms, FONT_TEXT, 0, 100, "Player %d wins!", winner+1);
  }

  rdpq_text_printf(NULL, FONT_TEXT, 20, 240-20, "FPS: %.2f", display_get_fps());

  rdpq_detach_show();
}

void minigame_cleanup(void)
{
  for (size_t i = 0; i < MAXPLAYERS; i++)
  {
    cleanupSnakePlayer(&players[i]);
  }

  for (size_t i = 0; i < TOTAL_BLOCKS; i++)
  {
    cleanupDirtBlock(&dirtBlocks[i]);
  }
  
  for (size_t i = 0; i < 1; i++)
  {
    cleanupChest(&chests[i]);
  }

  wav64_close(&sfx_start);
  wav64_close(&sfx_countdown);
  wav64_close(&sfx_stop);
  wav64_close(&sfx_winner);
  xm64player_stop(&music);
  xm64player_close(&music);
  rspq_block_free(dplMap);

  t3d_model_free(snakeModel);
  t3d_model_free(modelMap);
  t3d_model_free(shadowModel);
  t3d_model_free(chestModel);
  t3d_model_free(dirtBlockModel);

  free_uncached(mapMatFP);

  rdpq_text_unregister_font(FONT_BILLBOARD);
  rdpq_font_free(fontBillboard);
  rdpq_text_unregister_font(FONT_TEXT);
  rdpq_font_free(font);
  t3d_destroy();

  display_close();
}