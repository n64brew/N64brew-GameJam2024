/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "player.h"
#include "../render/colorHelper.h"
#include "../debug/debugDraw.h"
#include "shadows.h"
#include "scene.h"
#include "../utils/mesh.h"
#include <t3d/t3dmodel.h>

namespace {
  constexpr int SPRITES_X = 4;
  constexpr int SPRITE_SIZE_X = 64;
  constexpr int SPRITE_SIZE_Y = 64;

  constexpr float RESPAWN_TIMEOUT = 1.2f;

  constexpr float SLASH_TIMEOUT   = 0.5f;
  constexpr float SLASH_TIMER_MAX = 0.4f;
  constexpr float COIN_TIMER_MAX  = 2.0f;
  constexpr float HURT_TIMEOUT    = 1.25f;

  constexpr float MOVE_RUN_SPEED  =  3.0f;
  constexpr float MOVE_SPIN_SPEED =  9.0f;
  constexpr float JUMP_HEIGHT     =  7.0f;
  constexpr float JUMP_BOOST_TIME =  0.75f;
  constexpr float GRAVITY         =  31.0f;

  constexpr float OOB_HEIGHT = -15.0f;
  constexpr float OOB_LR_SCREEN = 90.0f;

  constexpr float ALERT_FADE_TIME = 0.35f;

  // Shared resources
  uint32_t refCount = 0;
  T3DModel *model;
  sprite_t *sprites[4]{};

  T3DObject *objBody;
  T3DObject *objFX;
  sprite_t *texNumbers;
  uint32_t globalTicks = 0;

  rdpq_blitparms_t getSpriteParams(int index) {
    index %= SPRITES_X;
    return {
      .s0 = index * SPRITE_SIZE_X,
      .t0 = 0,
      .width = SPRITE_SIZE_X,
      .height = SPRITE_SIZE_Y,
    };
  }
}

Player::Player(uint8_t index, Scene &scene)
  : collider{{0, 0, 0}, 0.275f}, scene{scene}, index{index}
{
  if(refCount++ == 0) {
    model = t3d_model_load(FS_BASE_PATH "player.t3dm");

    sprites[0] = sprite_load(FS_BASE_PATH "knight0.ci4.sprite");
    sprites[1] = sprite_load(FS_BASE_PATH "knight1.ci4.sprite");
    sprites[2] = sprite_load(FS_BASE_PATH "knight2.ci4.sprite");
    sprites[3] = sprite_load(FS_BASE_PATH "knight3.ci4.sprite");

    texNumbers = sprite_load(FS_BASE_PATH "ui/numbers.ia8.sprite");

    objBody = Mesh::recordObject(model, "Body");
    objFX = Mesh::recordObject(model, "Sword");
  }

  availAttacks = 1;

  collider.mask = 0xFF;
  collider.type = Coll::CollType::PLAYER;
  collider.callback = [this](Coll::Sphere &sphere) {onCollision(sphere);};
  collider.interactType = Coll::InteractType::TRI_MESH | Coll::InteractType::SPHERES;
  scene.getCollScene().registerSphere(&collider);

  collSword = {{0, 0, 0}, 0.5f};
  collSword.type = Coll::CollType::SWORD;
  collSword.interactType = 0;
  scene.getCollScene().registerSphere(&collSword);
}

Player::~Player() {
  // @TODO: destructor order in scene (currently no mem-leaks though)
  //scene.getCollScene().unregisterSphere(&collider);
  if(--refCount == 0) {
    t3d_model_free(model);
    for(auto &sprite : sprites) {
      sprite_free(sprite);
    }
    sprite_free(texNumbers);
  }
}

void Player::hurt() {
  if(hurtTimer > 0.0f)return;
  hurtTimer = HURT_TIMEOUT;
  scene.requestSpawnActor("Part"_u32, collider.center, 2);

  if(coinCount > 3) {
    coinCount -= 3;
    showCoinCount();
    for(int i=0; i<2; ++i) {
      scene.requestSpawnActor("Coin"_u32, collider.center, 1);
    }
  }
}

void Player::collectCoin(int amount) {
  coinCount += amount;
  if(coinCount % 10 == 0) {
    showCoinCount();
  }
}

void Player::onCollision(Coll::Sphere &sphere) {
  if(sphere.type== Coll::CollType::COIN) {
    return collectCoin(1);
  }
  if(sphere.type== Coll::CollType::COIN_MULTI) {
    return collectCoin(5);
  }

  if(hurtTimer > 0.0f)return;
  if(&sphere == &collSword)return;

  bool canHurt = sphere.type == Coll::CollType::SWORD ||
                  sphere.type == Coll::CollType::BOSS_BODY;

  if(canHurt)
  {
    auto diff = collider.center - sphere.center;
    t3d_vec3_norm(diff);
    if(sphere.type == Coll::CollType::BOSS_BODY) {
      hurtVel = diff * 20.0f;
    } else {
      hurtVel = diff * 9.0f;
    }

    if(sphere.type == Coll::CollType::BOSS_BODY) {
      hurt();
    }
  }
}

void Player::respawn() {
  auto ticks = get_ticks();

  // spawn close to camera
  auto basePos = collider.center;
  basePos.x = (scene.getCamera().pos.x / COLL_WORLD_SCALE) - 2.0f;

  collider.mask = 0xFF;
  collider.center = scene.getClosesRespawn(basePos) + T3DVec3{{0, 0.025f, 0}};
  collider.velocity = {0,0,0};
  hurt();
  scene.getAudio().playSFX("Notice"_u64, collider.center, {.volume = 0.6f, .variation = 64});
  ticks = get_ticks() - ticks;
  ++respawnCounter;
}

void Player::update(const InputState &input, float deltaTime)
{
  if(respawnTimer > 0.0f) {
    respawnTimer -= deltaTime;
    collider.mask = 0;
    collSword.mask = 0;
    if(respawnTimer < 0) {
      respawn();
    }
    return;
  }

  bool touchedFloor = collider.hitTriTypes & Coll::TriType::FLOOR;
  if(index == 0)++globalTicks;

  slashTimer = fmaxf(0.0f, slashTimer - deltaTime);
  hurtTimer = fmaxf(0.0f, hurtTimer - deltaTime);
  slashTimeout = fmaxf(0.0f, slashTimeout - deltaTime);
  coinTimer = fmaxf(0.0f, coinTimer - deltaTime);
  dustTimer = fmaxf(0.0f, dustTimer - deltaTime);
  alertTimer.update(deltaTime);

  if(touchedFloor && timeInAir > 0.1f) {
    scene.getAudio().playSFX("PlImpact"_u64, collider.center, {.volume = 0.1f, .variation = 100});

  }
  timeInAir = touchedFloor ? 0.0f : timeInAir + deltaTime;

  if(!touchedFloor && globalTicks % 2 == 0 && slashTimer > 0.0f) {
    float relTime = (slashTimer / SLASH_TIMER_MAX);
    if(relTime > 0.1f && collider.velocity.y > -0.1f) {
      scene.getPTSwirl().add(collider.center*COLL_WORLD_SCALE, rand(), relTime * 0.6f);
    }
  }

  collSword.center = collider.center;
  collSword.mask = isAttacking() ? 0xFF : 0x00;

  if(canAttack() && input.attack && !isHurt()) {
    slashTimer = SLASH_TIMER_MAX;
    slashTimeout = SLASH_TIMEOUT;
    --availAttacks;

    if(!touchedFloor) {
      auto seed = (uint32_t)rand();
      spawnParticles(seed % 3 + 8, seed, 12.0f, 0.55f);
    }

    // little range-boost if attacking-mid air by resetting gravity
    if(collider.velocity.y < 0.1f) {
      collider.velocity.y = 0.1f;
    }

    scene.getAudio().playSFX("PlSpin"_u64, collider.center, {.volume = 0.65f});
  }

  float moveSpeed = isAttacking()
    ? (slashTimer / SLASH_TIMER_MAX * MOVE_SPIN_SPEED)
    : MOVE_RUN_SPEED;

  float moveLen = t3d_vec3_len2(input.move);
  isMoving = moveLen > 0.1f;
  collider.velocity.x = hurtVel.x;
  collider.velocity.z = hurtVel.z;
  hurtVel *= 0.8f;

  if(touchedFloor) {
    availAttacks = 1;

    if(!isMoving)dustTimer = 0.3f;
    if(dustTimer == 0.0f) {

      //scene.getAudio().playSFX("PlImpact"_u64, collider.center, {.volume = 0.5f});

      auto seed = (uint32_t)rand();
      spawnParticles(seed % 3 + 1, seed, 4.0f, 0.4f);
      dustTimer = 0.1f + Math::rand01() * 0.3f;
    }
  }

  if(isJumping)
  {
    if(touchedFloor)isJumping = false;
    if(!input.jumpHold || collider.velocity.v[1] < 0.0f)isJumpHeld = false;
  } else if(input.jump)
  {
    collider.velocity.y += JUMP_HEIGHT;
    isJumping = true;
    isJumpHeld = true;
    jumpBoostTimer = JUMP_BOOST_TIME;
  }

  if(isJumping) {
    float boostRel = jumpBoostTimer / JUMP_BOOST_TIME;
    moveSpeed *= (1.0f + (boostRel * 0.7f));
    jumpBoostTimer = fmaxf(0.0f, jumpBoostTimer - deltaTime);
  }

  if(isMoving || walkTimer == 0.0f) {
    auto newVel = input.move * moveSpeed;
    collider.velocity.x += newVel.x;
    collider.velocity.z += newVel.z;
  }

  float localGrav = GRAVITY;
  if(isJumpHeld) {
    localGrav *= 0.55f;
  }

  collider.velocity.y -= localGrav * deltaTime;

  //debugf("Player pos: %f %f %f\n", sphere.center.v[0], sphere.center.v[1], sphere.center.v[2]);
  //auto collRes = collScene.vsSphere(collider, velocity, deltaTime);
  /*if(collRes.collCount) {
    float penLen2 = t3d_vec3_len2(&collRes.penetration);
    velocity.v[1] = 0.0f;
  }*/

  if(isMoving) {
    faceDir = -atan2f(input.move.x, input.move.z);
  }

  faceDirDisp = faceDir > 0.0f ? T3D_PI : 0.0f;
  faceDirDispTarget = t3d_lerp(faceDirDispTarget, faceDirDisp, 0.4f);

  float scale = 0.12f;
  auto posWorld = collider.center * COLL_WORLD_SCALE;

  float rotY = faceDirDispTarget;
  if(isAttacking()) {
    float slashIdx = 1.0f - (slashTimer / SLASH_TIMER_MAX);
    slashIdx = Math::easeOutCubic(slashIdx);
    rotY += slashIdx * T3D_PI*4;

    float swordScale = 1.0f - (slashTimer / SLASH_TIMER_MAX);
    swordScale = Math::easeOutCubic(swordScale);
    swordScale = scale * (0.5f + swordScale);

    t3d_mat4fp_from_srt_euler((T3DMat4FP*)UncachedAddr(&matSwordFP),
      T3DVec3{swordScale, swordScale, swordScale},
      T3DVec3{0,rotY * 1.25f ,0},
      posWorld + T3DVec3{0,collider.radius*14.0f,0}
    );
    data_cache_hit_writeback(&matSwordFP, sizeof(T3DMat4FP));
  }

  t3d_mat4fp_from_srt_euler((T3DMat4FP*)UncachedAddr(&matFP),
    T3DVec3{scale, scale, scale},
    T3DVec3{0,rotY,0},
    posWorld - T3DVec3{0,4.0f,0}
  );
  data_cache_hit_writeback(&matFP, sizeof(T3DMat4FP));

  auto floorPos = scene.getCollScene().raycastFloor(collider.center);
  floorPos.hitPos *= 16.0f;

  auto headPos = posWorld + T3DVec3{0,10.0f,0};
  t3d_viewport_calc_viewspace_pos(NULL, &pos2D, &headPos);
  if(floorPos.collCount) {
    Shadows::addShadow(floorPos.hitPos - T3DVec3 {{0,-0.1f,0}}, floorPos.normal, 1.0f, 1.0f);
  }

  // OOB / fall-off check
  if(collider.center.y < OOB_HEIGHT ||
    pos2D.x < -OOB_LR_SCREEN ||
    pos2D.x > SCREEN_WIDTH + OOB_LR_SCREEN
  ) {
    collider.velocity.x = 0;
    collider.velocity.z = 0;
    slashTimer = 0;
    respawnTimer = RESPAWN_TIMEOUT;
  }
}

void Player::draw(float deltaTime)
{
  if(index == 0) {
    t3d_state_set_drawflags(
      static_cast<T3DDrawFlags>(T3DDrawFlags::T3D_FLAG_DEPTH | T3DDrawFlags::T3D_FLAG_SHADED |
                                T3DDrawFlags::T3D_FLAG_TEXTURED)
    );

    rdpq_sync_pipe();
    rdpq_mode_begin();
      rdpq_mode_alphacompare(100);
      //rdpq_mode_filter(FILTER_POINT);
      rdpq_mode_filter(FILTER_BILINEAR);
      rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
      rdpq_mode_zbuf(true, true);
    rdpq_mode_end();
  }

  if(hurtTimer > 0.0f) {
    auto col = ColorHelper::primHurtEffect(hurtTimer, 1.0f / HURT_TIMEOUT);
    rdpq_set_prim_color(col);
  } else {
    //auto col = PLAYER_COLORS[index];
    //col.a = 120;
    //rdpq_set_prim_color(col);
    rdpq_set_prim_color({0xFF, 0xFF, 0xFF, 0xFF});
  }

  float len2 = t3d_vec3_len(collider.velocity) / 3.0f;
  len2 = fmin(len2, 1.0f);

  walkTimer += deltaTime * len2;

  int moveIdx = (int)(walkTimer*10.0f) % 3;
  if(!isMoving)moveIdx = 0;

  auto params = getSpriteParams(moveIdx);

  auto surf = sprite_get_pixels(sprites[index]);
  auto surfSub = surface_make_sub(&surf, params.s0, params.t0, params.width, params.height);
  rdpq_tex_upload(TILE0, &surfSub, nullptr);

  rdpq_tex_upload_tlut(sprite_get_palette(sprites[index]), 0, 16);
  rdpq_mode_tlut(TLUT_RGBA16);

  t3d_matrix_set(&matFP, true);
  rspq_block_run(objBody->userBlock);
}

void Player::drawTransp(float deltaTime) {
  if(index == 0) {
    t3d_model_draw_material(objFX->material, nullptr);
    rdpq_mode_alphacompare(0);
  }

  if(isAttacking()) {
    float slashFact = (slashTimer / SLASH_TIMER_MAX);

    rdpq_set_prim_color({0xFF, 0xFF, 0xFF, (uint8_t)(slashFact * 0xFF)});
    t3d_matrix_set(&matSwordFP, true);
    rspq_block_run(objFX->userBlock);
  }
}

void Player::draw2D()
{
  if(index == 0) {
    rdpq_mode_combiner(RDPQ_COMBINER1((TEX0,0,PRIM,0), (TEX0,0,PRIM,0)));
    rdpq_mode_alphacompare(1);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_upload(TILE0, texNumbers, nullptr);
  }

  auto numPos = pos2D;
  float scale, sX, sY;

  if(alertTimer.value != 0.0f) {
    scale = Math::easeOutCubic(alertTimer.value * (1.0f / ALERT_FADE_TIME)) * 0.9f;
    sX = 14*scale;
    sY = 16*scale;
    float offX = 7*scale;
    float offY = 8*scale + 4;

    auto &col = PLAYER_COLORS[index];
    rdpq_set_prim_color({col.r, col.g, col.b, (uint8_t)(scale*0xFF)});
    rdpq_texture_rectangle_scaled(TILE0,
      numPos.x-offX, numPos.y-offY,
      numPos.x+sX-offX, numPos.y+sY-offY,
      140, 0, 140+14, 16
    );
  }

  if(coinTimer <= 0.0f)return;

  scale = Math::easeOutCubic(coinTimer) * 0.5f;

  uint8_t alpha = (uint8_t)(0xFF * (coinTimer/COIN_TIMER_MAX));
  rdpq_set_prim_color({0xFF, 0xFF, 0xFF, alpha});

  sX = 14*scale;
  sY = 16*scale;
  float spacing = 10*scale;

  numPos = pos2D;
  numPos.x -= spacing;
  numPos.y -= (16*scale);
  for(int x = coinCount/10; x > 0; x /= 10) {
    numPos.x += spacing*0.5f;
  }

  for(int x = coinCount; x > 0; x /= 10) {
    int s = (x % 10) * 14;
    rdpq_texture_rectangle_scaled(TILE0, numPos.x, numPos.y, numPos.x+sX, numPos.y+sY, s, 0, s+14, 16);
    numPos.x -= spacing;
  }
  //Debug::printf(pos2D.x, pos2D.y, "%d", coinCount);
}

void Player::spawnParticles(uint32_t count, uint32_t seed, float dist, float size) {
  for(uint32_t i=0; i<count; ++i) {
    auto pt = collider.center * COLL_WORLD_SCALE;
    pt.x += (Math::rand01()-0.5f) * dist;
    pt.y -= slashTimer == SLASH_TIMER_MAX ? 1.0f : 5.0f;
    pt.z += (Math::rand01()-0.5f) * dist + 0.2f;
    scene.getPTSwirl().add(pt, seed+i, Math::rand01() * 0.2f + size);
  }
}

void Player::setAlertIcon(bool show) {
  alertTimer.target = show ? ALERT_FADE_TIME : 0.0f;
}

void Player::showCoinCount() {
  coinTimer = COIN_TIMER_MAX;
}

