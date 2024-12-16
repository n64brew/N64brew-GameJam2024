#ifndef __BULLET_CONTROLLER_H
#define __BULLET_CONTROLLER_H

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
#include "./ui.hpp"
#include "./bullet.hpp"

constexpr float BulletHeight = 35.f;
constexpr int BulletLimit = 100;
constexpr float Gravity = -200;

class BulletController
{
    private:
        std::size_t newBulletCount;
        U::T3DModel model;
        U::RSPQBlock block;

        List<Bullet, BulletLimit> bullets;

        std::shared_ptr<MapRenderer> map;
        std::shared_ptr<UIRenderer> ui;

        Wav64 sfxFire;
        Wav64 sfxHit;

        bool simulatePhysics(float deltaTime, Bullet &bullet);
        void killBullet(Bullet &bullet);

    public:
        BulletController(std::shared_ptr<MapRenderer> map, std::shared_ptr<UIRenderer> ui);
        void render(float deltaTime);
        void fixedUpdate(float deltaTime, std::vector<Player> &);
        void fireBullet(const T3DVec3 &pos, const T3DVec3 &velocity, PlyNum owner, PlyNum team);
};

#endif // __BULLET_CONTROLLER_H