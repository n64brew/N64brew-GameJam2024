#ifndef __MAP_H
#define __MAP_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

#include <memory>
#include <vector>

#include "./constants.hpp"
#include "./wrappers.hpp"
#include "./list.hpp"

#include "../../../core.h"

constexpr int MapWidth = 512;
constexpr int TileSize = 32;
constexpr int SegmentSize = 75;

struct Splash {
    float x;
    float y;
    PlyNum team;
};

class MapRenderer
{
    private:
        RDPQSurface surface;
        U::RSPQBlock renderModeBlock;
        U::RSPQBlock paintBlock;
        // U::RSPQBlock drawBlock;
        U::Sprite sprite;

        U::TLUT tlut;

        // Assume all players firing in all possible directions
        // in reality, they can pop in subticks but should be fine
        List<Splash, PlayerCount * 4> newSplashes;

        T3DVertPacked* vertices;

        void __splash(int x, int y, PlyNum player);
    public:
        MapRenderer();
        ~MapRenderer();
        void render(const T3DViewport &viewport);
        void splash(float x, float y, PlyNum team);
};

#endif // __MAP_H

