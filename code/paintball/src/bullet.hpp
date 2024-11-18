#ifndef __DAMAGE_H
#define __DAMAGE_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <array>
#include <algorithm>

#include "../../../core.h"
#include "../../../minigame.h"

#include "./wrappers.hpp"
#include "./constants.hpp"
#include "./player.hpp"
#include "./map.hpp"

constexpr float BulletVelocity = 300;
constexpr float BulletHeight = 35.f;
constexpr int BulletLimit = 100;
constexpr float PlayerRadius = 12;
constexpr float Gravity = -200;

class Bullet
{
    public:
        Bullet();
        T3DVec3 pos;
        T3DVec3 prevPos;
        T3DVec3 velocity;
        PlyNum team;
        PlyNum owner;
        U::T3DMat4FP matFP;
};

class BulletController
{
    private:
        std::size_t newBulletCount;
        U::T3DModel model;
        U::RSPQBlock block;

        // Assume all players firing in all possible directions
        std::array<Bullet, PlayerCount * 4> newBullets;

        std::array<Bullet, BulletLimit> bullets;

        std::shared_ptr<MapRenderer> map;

        void simulatePhysics(float deltaTime, Bullet &bullet);
        void killBullet(Bullet &bullet);
        bool applyDamage(Player::GameplayData &gameplayData, PlyNum team);

    public:
        BulletController(std::shared_ptr<MapRenderer> map);
        void render(float deltaTime);
        std::array<bool, PlayerCount> fixedUpdate(float deltaTime, std::vector<Player::GameplayData> &gameplayData);
        void fireBullet(const T3DVec3 &pos, const T3DVec3 &velocity, PlyNum owner, PlyNum team);
};

#endif // __DAMAGE_H