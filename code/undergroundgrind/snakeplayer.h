#include <t3d/t3d.h>
#include <libdragon.h>
#include "../../core.h"
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

typedef struct
{
  PlyNum plynum;
  T3DMat4FP* modelMatFP;
  rspq_block_t *dplSnake;
  T3DAnim animAttack;
  T3DAnim animWalk;
  T3DAnim animIdle;
  T3DAnim animJump;
  T3DSkeleton skelBlend;
  T3DSkeleton skel;
  T3DVec3 moveDir;
  T3DVec3 playerPos;
  float rotY;
  float currSpeed;
  float animBlend;
  bool isAttack;
  bool isJump;
  char previousButtonPressed;
  float attackTimer;
  float jumpTimer;
  int comboBonus;
  float comboTimer;
  PlyNum ai_target;
  int ai_reactionspeed;
} SnakePlayer;

void initSnakePlayer(SnakePlayer *player, color_t color, T3DVec3 position, float rotation, T3DModel *shadowModel, T3DModel *snakeModel)
{
  player->modelMatFP = malloc_uncached(sizeof(T3DMat4FP));

  player->moveDir = (T3DVec3){{0,0,0}};
  player->playerPos = position;

  // First instantiate skeletons, they will be used to draw models in a specific pose
  // And serve as the target for animations to modify
  player->skel = t3d_skeleton_create(snakeModel);
  player->skelBlend = t3d_skeleton_clone(&player->skel, false); // optimized for blending, has no matrices

  // Now create animation instances (by name), the data in 'model' is fixed,
  // whereas 'anim' contains all the runtime data.
  // Note that tiny3d internally keeps no track of animations, it's up to the user to manage and play them.
  player->animIdle = t3d_anim_create(snakeModel, "Snake_Idle");
  t3d_anim_attach(&player->animIdle, &player->skel); // tells the animation which skeleton to modify
  
  player->animJump = t3d_anim_create(snakeModel, "Snake_Jump");
  t3d_anim_set_looping(&player->animJump, false); // don't loop this animation
  t3d_anim_set_playing(&player->animJump, false); // start in a paused state
  t3d_anim_attach(&player->animJump, &player->skel); // tells the animation which skeleton to modify

  player->animWalk = t3d_anim_create(snakeModel, "Snake_Walk");
  t3d_anim_attach(&player->animWalk, &player->skelBlend);

  // multiple animations can attach to the same skeleton, this will NOT perform any blending
  // rather the last animation that updates "wins", this can be useful if multiple animations touch different bones
  player->animAttack = t3d_anim_create(snakeModel, "Snake_Attack");
  t3d_anim_set_looping(&player->animAttack, false); // don't loop this animation
  t3d_anim_set_playing(&player->animAttack, false); // start in a paused state
  t3d_anim_attach(&player->animAttack, &player->skel);

  rspq_block_begin();
    t3d_matrix_push(player->modelMatFP);
    rdpq_set_prim_color(color);
    t3d_model_draw_skinned(snakeModel, &player->skel); // as in the last example, draw skinned with the main skeleton

    rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
    t3d_model_draw(shadowModel);
    t3d_matrix_pop(1);
  player->dplSnake = rspq_block_end();

  player->rotY = rotation;
  player->currSpeed = 0.0f;
  player->animBlend = 0.0f;
  player->isAttack = false;
  player->isJump = false;
  player->ai_target = rand() % 9;
  player->ai_reactionspeed = (2-core_get_aidifficulty())*5 + rand()%((3-core_get_aidifficulty())*3);
}

void cleanupSnakePlayer(SnakePlayer *player)
{
  rspq_block_free(player->dplSnake);

  t3d_skeleton_destroy(&player->skel);
  t3d_skeleton_destroy(&player->skelBlend);

  t3d_anim_destroy(&player->animIdle);
  t3d_anim_destroy(&player->animWalk);
  t3d_anim_destroy(&player->animAttack);
  t3d_anim_destroy(&player->animJump);

  free_uncached(player->modelMatFP);
}