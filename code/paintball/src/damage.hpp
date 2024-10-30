#ifndef __DAMAGE_H
#define __DAMAGE_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>

#include <memory>
#include <vector>

#include "../../../core.h"
#include "../../../minigame.h"

constexpr int BulletLimit = 100;

struct Bullet
{
    T3DVec3 pos;
    T3DVec3 direction;
    color_t color;
    std::unique_ptr<rspq_block_t, decltype(&rspq_block_free)> block;
    std::unique_ptr<T3DMat4FP, decltype(&free_uncached)> matFP;
};

class DamageController
{
    private:
        int bulletCount;
        std::vector<Bullet> bullets;
        std::unique_ptr<T3DModel, decltype(&t3d_model_free)> model;

    public:
        DamageController();
        void update(float deltaTime);
        void fixed_update(float deltaTime);

        void fireBullet(const T3DVec3 &pos, T3DVec3 &direction, color_t color);
};

#endif // __DAMAGE_H