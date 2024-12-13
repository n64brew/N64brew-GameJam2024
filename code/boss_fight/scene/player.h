/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include "../main.h"
#include "../collision/scene.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

class Scene;

class Player
{
  private:
    Coll::Sphere collider{};
    Coll::Sphere collSword{};
    T3DVec3 pos2D{};
    T3DVec3 hurtVel{};

    float walkTimer{0.0f};
    float faceDir{0.0f};

    float faceDirDisp{0.0f};
    float faceDirDispTarget{0.0f};

    bool isMoving{false};
    bool isJumping{false};
    bool isJumpHeld{false};
    int availAttacks{0};

    float slashTimer{0.0f};
    float hurtTimer{0.0f};
    float jumpBoostTimer{0.0f};
    float slashTimeout{0.0f};

    float coinTimer{0.0f};
    float dustTimer{0.0f};
    float timeInAir{0.0f};
    float respawnTimer{0.0f};

    T3DMat4FP matFP{};
    T3DMat4FP matSwordFP{};

    Math::Timer alertTimer{};

    Scene &scene;
    uint8_t index{0};
    uint32_t coinCount{0};

    void onCollision(Coll::Sphere &sphere);
    void respawn();
    void spawnParticles(uint32_t count, uint32_t seed, float dist, float size);

  public:
    uint32_t respawnCounter{0};

    Player(uint8_t index, Scene &scene);
    ~Player();

    void update(const InputState &input, float deltaTime);
    void draw(float deltaTime);
    void drawTransp(float deltaTime);
    void draw2D();

    void setPos(const T3DVec3 &newPos) {
      collider.center = newPos;
    }

    void hurt();
    void collectCoin(int amount);

    void setAlertIcon(bool show);
    void showCoinCount();

    [[nodiscard]] const T3DVec3 &getPos() const { return collider.center; }
    [[nodiscard]] const T3DVec3 &getScreenPos() const { return pos2D; }
    [[nodiscard]] uint8_t getIndex() const { return index; }
    [[nodiscard]] bool canAttack() const { return availAttacks && slashTimeout == 0.0f; }
    [[nodiscard]] uint32_t getCoinCount() const { return coinCount; }

    [[nodiscard]] const Coll::Sphere &getColl() const { return collider; }

    [[nodiscard]] bool isAttacking(float threshold = 0.0f) const { return slashTimer > threshold; }
    [[nodiscard]] bool isHurt() const { return hurtTimer > 0.0f; }
    [[nodiscard]] bool isDead() const { return respawnTimer > 0.0f; }

    [[nodiscard]] bool isMidJump() const { return isJumping; }
};