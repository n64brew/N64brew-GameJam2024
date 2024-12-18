/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "playerAI.h"
#include "scene.h"
#include "../debug/debugDraw.h"

using CT = Coll::CollType;

namespace {
}

InputState PlayerAI::update(float deltaTime) {
  InputState res{};

  uint32_t rng = rand();
  bool againstWall = player.getColl().hitTriTypes & Coll::TriType::WALL;
  bool onFloor = player.getColl().hitTriTypes & Coll::TriType::FLOOR;

  time += deltaTime;
  if(time > randTimeEnd && onFloor) {
    bool isAttack = (rng & 0xFF) > 150;
    changeAction(isAttack ? Action::ATTACK : Action::COLLECT);
  }

  if(lastRespawnCounter != player.respawnCounter) {
    lastRespawnCounter = player.respawnCounter;
    lastNavPoint = {};
    noProgressTime = 0.0f;
    lastPos = player.getPos();
  }

  // check if we could safely move to our target
  res.move = {};
  if(action == Action::PROGRESS) {
    res.move.x = 1.0f;
  }

  // check if we moved at all in the last time, if not force some action
  if(fabsf(lastPos.x - player.getPos().x) < 0.1f) {
    noProgressTime += deltaTime;
  } else {
    noProgressTime = 0.0f;
    lastPos = player.getPos();
  }

  // raycast ahead
  auto nextPos = player.getPos() + res.move * 0.5f;

  auto nextFloor = scene.getCollScene().raycastFloor(nextPos + T3DVec3{{0, 5.0f, 0}});
  bool nextNoFloor = nextFloor.collCount == 0;
  float heightDiff = nextFloor.hitPos.y - player.getPos().y;

  if(nextFloor.collCount == 0) {
    heightDiff = 1.0f;
  }

  // Idle prevention, if we are not progressing for a while, force a jump
  // if that doesn't help switch to attack mode
  if(action == Action::ATTACK && noProgressTime > 1.0f)lastFocusPlayer = 0xFF;
  if(action == Action::ATTACK && noProgressTime > 1.75f)changeAction(Action::COLLECT);

  if(noProgressTime > 2.0f)heightDiff = 1.0f;
  if(noProgressTime > 3.0f)changeAction(Action::COLLECT);

  // check if we have a guide-point ahead of us
  if(!againstWall) {

    if(!lastNavPoint.point) {
      auto navPoint = scene.getNavPoints().getClosest(player.getPos(), 1.0f);
      if(navPoint.point && navPoint.dist2 < 100.0f) {
        lastNavPoint = navPoint;
      }
    }

    if(lastNavPoint.point) {
      //Debug::drawLine(player.getPos()*COLL_WORLD_SCALE, *lastNavPoint.point * COLL_WORLD_SCALE, color_t{0x22, 0x22, 0xFF, 0xFF});
      res.move.x = lastNavPoint.point->x - player.getPos().x;
      res.move.z = lastNavPoint.point->z - player.getPos().z;
      float dist2 = res.move.x*res.move.x + res.move.z*res.move.z;

      if(dist2 < 20.0f && onFloor) {
        lastNavPoint = {};
      }
      nextNoFloor = false;
    }
  }

  // if we ran against a wall we can either try to jump or walk around it
  // first check if slightly up-right or down-right is free via another raycast
  if(againstWall || nextNoFloor) {
    int zSteps = 10;
    auto posFwd = nextPos + T3DVec3{{0, 5.0f, 1.0f}};
    auto posBwd = nextPos + T3DVec3{{0, 5.0f, -1.0f}};
    if(nextNoFloor) {
      posFwd += res.move;
      posBwd += res.move;
    }

    int i=0;
    for(; i<zSteps; ++i)
    {
      auto avoidCastA = scene.getCollScene().raycastFloor(posFwd);
      auto avoidCastB = scene.getCollScene().raycastFloor(posBwd);

      //if(avoidCastA.collCount)Debug::drawLine(player.getPos()*COLL_WORLD_SCALE, avoidCastA.hitPos*COLL_WORLD_SCALE, color_t{0xFF, 0xFF, 0x22, 0xFF});
      //if(avoidCastB.collCount)Debug::drawLine(player.getPos()*COLL_WORLD_SCALE, avoidCastB.hitPos*COLL_WORLD_SCALE, color_t{0xFF, 0xFF, 0x22, 0xFF});

      // now check if any floor has roughly the same height as the current one
      if(avoidCastA.collCount && fabsf(avoidCastA.hitPos.y - player.getPos().y) < 0.2f) {
        res.move.z = 2.0f;
        break;
      } else if(avoidCastB.collCount && fabsf(avoidCastB.hitPos.y - player.getPos().y) < 0.2f) {
        res.move.z = -2.0f;
        break;
      }
      posFwd.z += 1.0f;
      posBwd.z -= 1.0f;
    }

    // if we can't move around force a jump
    if(i == zSteps)heightDiff = 999;
  }

  // if something interrupted our jump, reset the jump hold time
  if(jumpHoldTime > 0.0f && !player.isMidJump()) {
    jumpHoldTime = 0.0f;
  }

  // if the next spot we land is too high, jump...
  if(heightDiff > 0.2f)
  {
    if(jumpHoldTime <= 0.0f) {
      jumpHoldTime = 1.0f;
      res.jump = true;
    }
    //...and hold for a while to jump further
    res.jumpHold = jumpHoldTime > 0.0f;
    jumpHoldTime -= deltaTime;
  }

  // if we are falling, either from dropping or after the peak of a jump
  // check if we are above ground. if not attack to get more distance
  if(player.isMidJump() && player.getColl().velocity.y < 1.0f) {
    if(nextFloor.collCount == 0) {
      res.attack = true;
    }
  }

  //##################// COLLECT LOGIC //##################//
  auto preAttackMove = res.move;

  if(action == Action::COLLECT)
  {
    float closestCoinDist = 1000000.0f;
    float closestDestroyDist = 1000000.0f;
    T3DVec3 *closestCoin = nullptr;
    T3DVec3 *closestDestroy = nullptr;
    float camX = scene.getCamera().getPos().x / COLL_WORLD_SCALE;

    auto &coll = scene.getCollScene().getSpheres();
    for(const auto c : coll) {
      if(c->center.x - 4.0f > camX)continue;

      if(c->type == CT::COIN || c->type == CT::COIN_MULTI) {
        auto coinDist = t3d_vec3_len2(c->center - player.getPos());
        if(coinDist < closestCoinDist) {
          closestCoinDist = coinDist;
          closestCoin = &c->center;
        }
      } else if(c->type == CT::DESTRUCTABLE || c->type == CT::BOSS_HEAD) {
        auto destroyDist = t3d_vec3_len2(c->center - player.getPos());
        if(destroyDist < closestDestroyDist) {
          closestDestroyDist = destroyDist;
          closestDestroy = &c->center;
        }
      }
    }

    if(closestCoin && closestCoinDist < closestDestroyDist) {
      //Debug::drawLine(player.getPos()*COLL_WORLD_SCALE, *closestCoin*COLL_WORLD_SCALE, color_t{0x22, 0x22, 0xFF, 0xFF});
      res.move.x = closestCoin->x - player.getPos().x;
      res.move.z = closestCoin->z - player.getPos().z;
    } else if(closestDestroy) {
      //Debug::drawLine(player.getPos()*COLL_WORLD_SCALE, *closestDestroy*COLL_WORLD_SCALE, color_t{0xFF, 0x22, 0x22, 0xFF});
      res.move.x = closestDestroy->x - player.getPos().x;
      res.move.z = closestDestroy->z - player.getPos().z;
      if(closestDestroyDist < 1.0f) {
        res.attack = true;
      }
    } else {
      changeAction((rng&1) ? Action::IDLE : Action::ATTACK);
    }

    if(rng % 30 == 0) {
      res.attack = true;
    }
  }

  // If the player we are targeting is dead or too far away, reset the target
  if(action == Action::ATTACK)
  {
    if(lastFocusPlayer != 0xFF) {
      auto &pl = scene.getPlayer(lastFocusPlayer);
      float heightDiff = pl.getPos().y - player.getPos().y;
      if(heightDiff < -2.0f) {
        lastFocusPlayer = 0xFF;
      }
    }

    if(lastFocusPlayer == 0xFF) {
      // find the closest player, and stick to it for a bit
      const Player& closetPlayer = getClosetPlayer();
      if(closetPlayer.getIndex() != player.getIndex()) {
        lastFocusPlayer = closetPlayer.getIndex();
      }
      if(lastFocusPlayer == 0xFF) {
        changeAction(player.getScreenPos().x < 100.0f ? Action::PROGRESS : Action::IDLE);
      }
    }

    // Attack logic, if a player is nearby try to attack it
    if(!nextNoFloor && lastFocusPlayer != 0xFF) {
      auto targetColl = scene.getPlayer(lastFocusPlayer).getColl();

      float minDist2 = targetColl.radius * targetColl.radius;
      auto dirBoss = targetColl.center - player.getPos();

      //Debug::drawLine(player.getPos()*COLL_WORLD_SCALE, targetColl.center*COLL_WORLD_SCALE, color_t{0xFF, 0x22, 0x22, 0xFF});

      float dist2 = t3d_vec3_len2(dirBoss);
      if(dist2 > 0.0001f) {
        dirBoss /= sqrtf(dist2);
      } else {
        dirBoss = {-1.0f, 0.0f, 0.0f};
      }

      moveOut = !player.canAttack();

      res.move.v[0] = moveOut ? -dirBoss.v[0] : dirBoss.v[0];
      res.move.v[2] = moveOut ? -dirBoss.v[2] : dirBoss.v[2];
      float moveSpeed = fminf(dist2 / minDist2, 1.0f);

      res.move *= moveSpeed;

      // random offset to make it less predictable
      res.move.v[0] += (rand() % 100) < 50 ? 0.1f : -0.1f;
      res.move.v[2] += (rand() % 100) < 50 ? 0.1f : -0.1f;

      if(player.isHurt())moveOut = true;
      if(moveOut) {
        if(dist2 > 5.0f) {
          moveOut = false;
        }
      }

      if(dist2 < minDist2*10.0f) {
        res.attack = (rand() % 100) < 90;
      }
    }
  }

  if(action == Action::ATTACK || action == Action::COLLECT) {
    // check once more if floor is ahead
    nextPos = player.getPos() + res.move * 0.5f;
    nextFloor = scene.getCollScene().raycastFloor(nextPos + T3DVec3{{0, 5.0f, 0}});
    if(nextFloor.collCount == 0) {
      res.move = preAttackMove;
    } else {
      heightDiff = nextFloor.hitPos.y - player.getPos().y;
      if(heightDiff > 0.1f) {
        res.jump = true;
      }
    }
  }

  if(onFloor && action == Action::IDLE) {
    res = {};
  }

  // check if we need to move or can focus on attacking
  // for that check see how far to the left we are in screen space
  float offScreenMax = action == Action::COLLECT ? 10.0f : 60.0f;
  if(player.getScreenPos().x < offScreenMax) {
    changeAction(Action::PROGRESS);
  }

  if(player.getScreenPos().x > 200.0f) {
    if(action == Action::PROGRESS && onFloor) {
      changeAction(Action::COLLECT);
    }
  }

  lastPos = player.getPos();

  t3d_vec3_norm(res.move);
  return res;
}

const Player& PlayerAI::getClosetPlayer() {
  float closestDist2 = 1000000.0f;
  int closestIndex = player.getIndex();
  for(int i=0; i<4; ++i) {
    if(i == player.getIndex())continue;
    const auto &playerColl = scene.getPlayer(i).getColl();
    float dist2 = t3d_vec3_len2(playerColl.center - player.getPos());
    if(dist2 < closestDist2) {
      closestDist2 = dist2;
      closestIndex = i;
    }
  }
  return scene.getPlayer(closestIndex);
}

void PlayerAI::changeAction(PlayerAI::Action newAction) {
  action = newAction;
  time = 0.0f;
  randTimeEnd = (float)(rand() % 4) + 2.5f;
  lastNavPoint = {};
  noProgressTime = 0.0f;

  if(action == Action::ATTACK) {
    lastFocusPlayer = 0xFF;
  }
}

void PlayerAI::changeAction() {
  changeAction((Action)(rand() % 2));
}

void PlayerAI::debugDraw() {
  if(player.getIndex() == 0)return;
  float px = player.getScreenPos().x;
  float py = player.getScreenPos().y;
  Debug::printf(px, py, "%d", (int)action);
  Debug::printf(px, py + 8, "%.2f", randTimeEnd - time);
  Debug::printf(px, py + 16, "%d", lastFocusPlayer);
}
