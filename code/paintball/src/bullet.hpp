#ifndef __DAMAGE_H
#define __DAMAGE_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <array>

#include "../../../core.h"
#include "../../../minigame.h"

#include "./wrappers.hpp"
#include "./constants.hpp"
#include "./player.hpp"

class Bullet
{
    public:
        Bullet();
        T3DVec3 pos;
        T3DVec3 velocity;
        color_t color;
        U::T3DMat4FP matFP;
};

class BulletController
{
    private:
        int newBulletCount;
        U::T3DModel model;
        U::RSPQBlock block;

        std::array<Bullet, MAXPLAYERS> newBullets;

        std::array<Bullet, BulletLimit> bullets;

    public:
        BulletController();
        void update(float deltaTime);
        void fixed_update(float deltaTime, const std::vector<PlayerGameplayData> &gameplayData);

        void fireBullet(const T3DVec3 &pos, const T3DVec3 &velocity, color_t color);
};

#endif // __DAMAGE_H