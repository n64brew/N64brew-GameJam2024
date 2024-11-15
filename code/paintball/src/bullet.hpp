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

constexpr float BulletVelocity = 500;
constexpr float BulletHeight = 25.f;
constexpr int BulletLimit = 100;
constexpr float PlayerRadius = 12;
constexpr int MaxHealth = 100;
constexpr int Damage = 10;
constexpr float Gravity = -300;

class Bullet
{
    public:
        Bullet();
        T3DVec3 pos;
        T3DVec3 prevPos;
        T3DVec3 velocity;
        PlyNum team;
        U::T3DMat4FP matFP;
};

class BulletController
{
    private:
        int newBulletCount;
        U::T3DModel model;
        U::RSPQBlock block;

        // TODO: MAXPLAYERS here is not correct for more than 4 players
        std::array<Bullet, MAXPLAYERS> newBullets;

        std::array<Bullet, BulletLimit> bullets;

        std::shared_ptr<MapRenderer> map;

        void simulatePhysics(float deltaTime, Bullet &bullet);
        void killBullet(Bullet &bullet);
        bool applyDamage(PlayerGameplayData &gameplayData, PlyNum team);

    public:
        BulletController(std::shared_ptr<MapRenderer> map);
        void render(float deltaTime);
        std::array<bool, PlayerCount> fixedUpdate(float deltaTime, std::vector<PlayerGameplayData> &gameplayData);
        void fireBullet(const T3DVec3 &pos, const T3DVec3 &velocity, PlyNum player);
};

#endif // __DAMAGE_H