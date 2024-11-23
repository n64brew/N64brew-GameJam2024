#ifndef __UI_HPP
#define __UI_HPP

#include <libdragon.h>
#include "./wrappers.hpp"
#include "./constants.hpp"
#include "./gamestate.hpp"

struct HitMark {
    T3DVec3 pos;
    PlyNum team;
    float lifetime;
};

class UIRenderer
{
    private:
        RDPQFont mediumFont;
        RDPQFont bigFont;

        std::size_t newHitCount;
        std::array<HitMark, PlayerCount> newHits;

        std::array<HitMark, PlayerCount * 4> activeHits;

        U::Sprite hitSprite;

        void renderHitMarks(T3DViewport &viewport, float deltaTime);

    public:
        UIRenderer();
        void render(const GameState &state, T3DViewport &viewport, float deltaTime);

        void registerHit(const HitMark &hit);
};

#endif // __UI_HPP