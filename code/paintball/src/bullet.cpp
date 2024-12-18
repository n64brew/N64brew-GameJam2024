#include "bullet.hpp"

Bullet::Bullet() :
    pos {0},
    prevPos {0},
    velocity {0},
    team {PLAYER_1},
    owner {PLAYER_1},
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)),free_uncached}) { }

Bullet::Bullet(T3DVec3 pos, T3DVec3 velocity, PlyNum owner, PlyNum team) :
    pos {pos},
    prevPos {pos},
    velocity {velocity},
    team {team},
    owner {owner},
    matFP({nullptr, free_uncached}) { }

Bullet::Bullet(Bullet&& other) :
    pos {other.pos},
    prevPos {other.pos},
    velocity {other.velocity},
    team {other.team},
    owner {other.owner},
    matFP({nullptr, free_uncached}) { }

Bullet& Bullet::operator=(Bullet& rhs) {
    if (this == &rhs) return *this;
    pos = rhs.pos;
    prevPos = rhs.prevPos;
    velocity = rhs.velocity;
    team = rhs.team;
    owner = rhs.owner;
    return *this;
};
