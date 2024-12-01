#ifndef __BULLET_H
#define __BULLET_H

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

class BulletController;
class AI;
class Player;

class Bullet
{
    friend class ::BulletController;
    friend class ::AI;
    friend class ::Player;

    public:
        Bullet();
        Bullet(T3DVec3 pos, T3DVec3 velocity, PlyNum owner, PlyNum team);
        Bullet(Bullet&& other);
        Bullet& operator=(Bullet&& rhs) = delete;
        Bullet& operator=(Bullet& rhs);

    private:
        T3DVec3 pos;
        T3DVec3 prevPos;
        T3DVec3 velocity;
        PlyNum team;
        PlyNum owner;

        // This is non-movable, it can only be created with default ctor
        const U::T3DMat4FP matFP;
};

#endif // __BULLET_H